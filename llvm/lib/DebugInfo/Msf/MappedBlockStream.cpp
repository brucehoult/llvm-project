//===- MappedBlockStream.cpp - Reads stream data from an MSF file ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Msf/MappedBlockStream.h"

#include "llvm/DebugInfo/Msf/IMsfFile.h"
#include "llvm/DebugInfo/Msf/MsfCommon.h"
#include "llvm/DebugInfo/Msf/MsfError.h"
#include "llvm/DebugInfo/Msf/MsfStreamLayout.h"

using namespace llvm;
using namespace llvm::msf;

namespace {
template <typename Base> class MappedBlockStreamImpl : public Base {
public:
  template <typename... Args>
  MappedBlockStreamImpl(Args &&... Params)
      : Base(std::forward<Args>(Params)...) {}
};
}

typedef std::pair<uint32_t, uint32_t> Interval;
static Interval intersect(const Interval &I1, const Interval &I2) {
  return std::make_pair(std::max(I1.first, I2.first),
                        std::min(I1.second, I2.second));
}

MappedBlockStream::MappedBlockStream(uint32_t BlockSize, uint32_t NumBlocks,
                                     const MsfStreamLayout &Layout,
                                     const ReadableStream &MsfData)
    : BlockSize(BlockSize), NumBlocks(NumBlocks), StreamLayout(Layout),
      MsfData(MsfData) {}

std::unique_ptr<MappedBlockStream>
MappedBlockStream::createStream(uint32_t BlockSize, uint32_t NumBlocks,
                                const MsfStreamLayout &Layout,
                                const ReadableStream &MsfData) {
  return llvm::make_unique<MappedBlockStreamImpl<MappedBlockStream>>(
      BlockSize, NumBlocks, Layout, MsfData);
}

std::unique_ptr<MappedBlockStream>
MappedBlockStream::createIndexedStream(const MsfLayout &Layout,
                                       const ReadableStream &MsfData,
                                       uint32_t StreamIndex) {
  MsfStreamLayout SL;
  SL.Blocks = Layout.StreamMap[StreamIndex];
  SL.Length = Layout.StreamSizes[StreamIndex];
  return llvm::make_unique<MappedBlockStreamImpl<MappedBlockStream>>(
      Layout.SB->BlockSize, Layout.SB->NumBlocks, SL, MsfData);
}

std::unique_ptr<MappedBlockStream>
MappedBlockStream::createDirectoryStream(const MsfLayout &Layout,
                                         const ReadableStream &MsfData) {
  MsfStreamLayout SL;
  SL.Blocks = Layout.DirectoryBlocks;
  SL.Length = Layout.SB->NumDirectoryBytes;
  return createStream(Layout.SB->BlockSize, Layout.SB->NumBlocks, SL, MsfData);
}

Error MappedBlockStream::readBytes(uint32_t Offset, uint32_t Size,
                                   ArrayRef<uint8_t> &Buffer) const {
  // Make sure we aren't trying to read beyond the end of the stream.
  if (Size > StreamLayout.Length)
    return make_error<MsfError>(msf_error_code::insufficient_buffer);
  if (Offset > StreamLayout.Length - Size)
    return make_error<MsfError>(msf_error_code::insufficient_buffer);

  if (tryReadContiguously(Offset, Size, Buffer))
    return Error::success();

  auto CacheIter = CacheMap.find(Offset);
  if (CacheIter != CacheMap.end()) {
    // Try to find an alloc that was large enough for this request.
    for (auto &Entry : CacheIter->second) {
      if (Entry.size() >= Size) {
        Buffer = Entry.slice(0, Size);
        return Error::success();
      }
    }
  }

  // We couldn't find a buffer that started at the correct offset (the most
  // common scenario).  Try to see if there is a buffer that starts at some
  // other offset but overlaps the desired range.
  for (auto &CacheItem : CacheMap) {
    Interval RequestExtent = std::make_pair(Offset, Offset + Size);

    // We already checked this one on the fast path above.
    if (CacheItem.first == Offset)
      continue;
    // If the initial extent of the cached item is beyond the ending extent
    // of the request, there is no overlap.
    if (CacheItem.first >= Offset + Size)
      continue;

    // We really only have to check the last item in the list, since we append
    // in order of increasing length.
    if (CacheItem.second.empty())
      continue;

    auto CachedAlloc = CacheItem.second.back();
    // If the initial extent of the request is beyond the ending extent of
    // the cached item, there is no overlap.
    Interval CachedExtent =
        std::make_pair(CacheItem.first, CacheItem.first + CachedAlloc.size());
    if (RequestExtent.first >= CachedExtent.first + CachedExtent.second)
      continue;

    Interval Intersection = intersect(CachedExtent, RequestExtent);
    // Only use this if the entire request extent is contained in the cached
    // extent.
    if (Intersection != RequestExtent)
      continue;

    uint32_t CacheRangeOffset =
        AbsoluteDifference(CachedExtent.first, Intersection.first);
    Buffer = CachedAlloc.slice(CacheRangeOffset, Size);
    return Error::success();
  }

  // Otherwise allocate a large enough buffer in the pool, memcpy the data
  // into it, and return an ArrayRef to that.  Do not touch existing pool
  // allocations, as existing clients may be holding a pointer which must
  // not be invalidated.
  uint8_t *WriteBuffer = static_cast<uint8_t *>(Pool.Allocate(Size, 8));
  if (auto EC = readBytes(Offset, MutableArrayRef<uint8_t>(WriteBuffer, Size)))
    return EC;

  if (CacheIter != CacheMap.end()) {
    CacheIter->second.emplace_back(WriteBuffer, Size);
  } else {
    std::vector<CacheEntry> List;
    List.emplace_back(WriteBuffer, Size);
    CacheMap.insert(std::make_pair(Offset, List));
  }
  Buffer = ArrayRef<uint8_t>(WriteBuffer, Size);
  return Error::success();
}

Error MappedBlockStream::readLongestContiguousChunk(
    uint32_t Offset, ArrayRef<uint8_t> &Buffer) const {
  // Make sure we aren't trying to read beyond the end of the stream.
  if (Offset >= StreamLayout.Length)
    return make_error<MsfError>(msf_error_code::insufficient_buffer);
  uint32_t First = Offset / BlockSize;
  uint32_t Last = First;

  while (Last < NumBlocks - 1) {
    if (StreamLayout.Blocks[Last] != StreamLayout.Blocks[Last + 1] - 1)
      break;
    ++Last;
  }

  uint32_t OffsetInFirstBlock = Offset % BlockSize;
  uint32_t BytesFromFirstBlock = BlockSize - OffsetInFirstBlock;
  uint32_t BlockSpan = Last - First + 1;
  uint32_t ByteSpan = BytesFromFirstBlock + (BlockSpan - 1) * BlockSize;

  ArrayRef<uint8_t> BlockData;
  uint32_t MsfOffset = blockToOffset(StreamLayout.Blocks[First], BlockSize);
  if (auto EC = MsfData.readBytes(MsfOffset, BlockSize, BlockData))
    return EC;

  BlockData = BlockData.drop_front(OffsetInFirstBlock);
  Buffer = ArrayRef<uint8_t>(BlockData.data(), ByteSpan);
  return Error::success();
}

uint32_t MappedBlockStream::getLength() const { return StreamLayout.Length; }

bool MappedBlockStream::tryReadContiguously(uint32_t Offset, uint32_t Size,
                                            ArrayRef<uint8_t> &Buffer) const {
  // Attempt to fulfill the request with a reference directly into the stream.
  // This can work even if the request crosses a block boundary, provided that
  // all subsequent blocks are contiguous.  For example, a 10k read with a 4k
  // block size can be filled with a reference if, from the starting offset,
  // 3 blocks in a row are contiguous.
  uint32_t BlockNum = Offset / BlockSize;
  uint32_t OffsetInBlock = Offset % BlockSize;
  uint32_t BytesFromFirstBlock = std::min(Size, BlockSize - OffsetInBlock);
  uint32_t NumAdditionalBlocks =
      llvm::alignTo(Size - BytesFromFirstBlock, BlockSize) / BlockSize;

  uint32_t RequiredContiguousBlocks = NumAdditionalBlocks + 1;
  uint32_t E = StreamLayout.Blocks[BlockNum];
  for (uint32_t I = 0; I < RequiredContiguousBlocks; ++I, ++E) {
    if (StreamLayout.Blocks[I + BlockNum] != E)
      return false;
  }

  // Read out the entire block where the requested offset starts.  Then drop
  // bytes from the beginning so that the actual starting byte lines up with
  // the requested starting byte.  Then, since we know this is a contiguous
  // cross-block span, explicitly resize the ArrayRef to cover the entire
  // request length.
  ArrayRef<uint8_t> BlockData;
  uint32_t FirstBlockAddr = StreamLayout.Blocks[BlockNum];
  uint32_t MsfOffset = blockToOffset(FirstBlockAddr, BlockSize);
  if (auto EC = MsfData.readBytes(MsfOffset, BlockSize, BlockData)) {
    consumeError(std::move(EC));
    return false;
  }
  BlockData = BlockData.drop_front(OffsetInBlock);
  Buffer = ArrayRef<uint8_t>(BlockData.data(), Size);
  return true;
}

Error MappedBlockStream::readBytes(uint32_t Offset,
                                   MutableArrayRef<uint8_t> Buffer) const {
  uint32_t BlockNum = Offset / BlockSize;
  uint32_t OffsetInBlock = Offset % BlockSize;

  // Make sure we aren't trying to read beyond the end of the stream.
  if (Buffer.size() > StreamLayout.Length)
    return make_error<MsfError>(msf_error_code::insufficient_buffer);
  if (Offset > StreamLayout.Length - Buffer.size())
    return make_error<MsfError>(msf_error_code::insufficient_buffer);

  uint32_t BytesLeft = Buffer.size();
  uint32_t BytesWritten = 0;
  uint8_t *WriteBuffer = Buffer.data();
  while (BytesLeft > 0) {
    uint32_t StreamBlockAddr = StreamLayout.Blocks[BlockNum];

    ArrayRef<uint8_t> BlockData;
    uint32_t Offset = blockToOffset(StreamBlockAddr, BlockSize);
    if (auto EC = MsfData.readBytes(Offset, BlockSize, BlockData))
      return EC;

    const uint8_t *ChunkStart = BlockData.data() + OffsetInBlock;
    uint32_t BytesInChunk = std::min(BytesLeft, BlockSize - OffsetInBlock);
    ::memcpy(WriteBuffer + BytesWritten, ChunkStart, BytesInChunk);

    BytesWritten += BytesInChunk;
    BytesLeft -= BytesInChunk;
    ++BlockNum;
    OffsetInBlock = 0;
  }

  return Error::success();
}

uint32_t MappedBlockStream::getNumBytesCopied() const {
  return static_cast<uint32_t>(Pool.getBytesAllocated());
}

void MappedBlockStream::invalidateCache() { CacheMap.shrink_and_clear(); }

void MappedBlockStream::fixCacheAfterWrite(uint32_t Offset,
                                           ArrayRef<uint8_t> Data) const {
  // If this write overlapped a read which previously came from the pool,
  // someone may still be holding a pointer to that alloc which is now invalid.
  // Compute the overlapping range and update the cache entry, so any
  // outstanding buffers are automatically updated.
  for (const auto &MapEntry : CacheMap) {
    // If the end of the written extent precedes the beginning of the cached
    // extent, ignore this map entry.
    if (Offset + Data.size() < MapEntry.first)
      continue;
    for (const auto &Alloc : MapEntry.second) {
      // If the end of the cached extent precedes the beginning of the written
      // extent, ignore this alloc.
      if (MapEntry.first + Alloc.size() < Offset)
        continue;

      // If we get here, they are guaranteed to overlap.
      Interval WriteInterval = std::make_pair(Offset, Offset + Data.size());
      Interval CachedInterval =
          std::make_pair(MapEntry.first, MapEntry.first + Alloc.size());
      // If they overlap, we need to write the new data into the overlapping
      // range.
      auto Intersection = intersect(WriteInterval, CachedInterval);
      assert(Intersection.first <= Intersection.second);

      uint32_t Length = Intersection.second - Intersection.first;
      uint32_t SrcOffset =
          AbsoluteDifference(WriteInterval.first, Intersection.first);
      uint32_t DestOffset =
          AbsoluteDifference(CachedInterval.first, Intersection.first);
      ::memcpy(Alloc.data() + DestOffset, Data.data() + SrcOffset, Length);
    }
  }
}

WritableMappedBlockStream::WritableMappedBlockStream(
    uint32_t BlockSize, uint32_t NumBlocks, const MsfStreamLayout &Layout,
    const WritableStream &MsfData)
    : ReadInterface(BlockSize, NumBlocks, Layout, MsfData),
      WriteInterface(MsfData) {}

std::unique_ptr<WritableMappedBlockStream>
WritableMappedBlockStream::createStream(uint32_t BlockSize, uint32_t NumBlocks,
                                        const MsfStreamLayout &Layout,
                                        const WritableStream &MsfData) {
  return llvm::make_unique<MappedBlockStreamImpl<WritableMappedBlockStream>>(
      BlockSize, NumBlocks, Layout, MsfData);
}

std::unique_ptr<WritableMappedBlockStream>
WritableMappedBlockStream::createIndexedStream(const MsfLayout &Layout,
                                               const WritableStream &MsfData,
                                               uint32_t StreamIndex) {
  MsfStreamLayout SL;
  SL.Blocks = Layout.StreamMap[StreamIndex];
  SL.Length = Layout.StreamSizes[StreamIndex];
  return createStream(Layout.SB->BlockSize, Layout.SB->NumBlocks, SL, MsfData);
}

std::unique_ptr<WritableMappedBlockStream>
WritableMappedBlockStream::createDirectoryStream(
    const MsfLayout &Layout, const WritableStream &MsfData) {
  MsfStreamLayout SL;
  SL.Blocks = Layout.DirectoryBlocks;
  SL.Length = Layout.SB->NumDirectoryBytes;
  return createStream(Layout.SB->BlockSize, Layout.SB->NumBlocks, SL, MsfData);
}

Error WritableMappedBlockStream::readBytes(uint32_t Offset, uint32_t Size,
                                           ArrayRef<uint8_t> &Buffer) const {
  return ReadInterface.readBytes(Offset, Size, Buffer);
}

Error WritableMappedBlockStream::readLongestContiguousChunk(
    uint32_t Offset, ArrayRef<uint8_t> &Buffer) const {
  return ReadInterface.readLongestContiguousChunk(Offset, Buffer);
}

uint32_t WritableMappedBlockStream::getLength() const {
  return ReadInterface.getLength();
}

Error WritableMappedBlockStream::writeBytes(uint32_t Offset,
                                            ArrayRef<uint8_t> Buffer) const {
  // Make sure we aren't trying to write beyond the end of the stream.
  if (Buffer.size() > getStreamLength())
    return make_error<MsfError>(msf_error_code::insufficient_buffer);

  if (Offset > getStreamLayout().Length - Buffer.size())
    return make_error<MsfError>(msf_error_code::insufficient_buffer);

  uint32_t BlockNum = Offset / getBlockSize();
  uint32_t OffsetInBlock = Offset % getBlockSize();

  uint32_t BytesLeft = Buffer.size();
  uint32_t BytesWritten = 0;
  while (BytesLeft > 0) {
    uint32_t StreamBlockAddr = getStreamLayout().Blocks[BlockNum];
    uint32_t BytesToWriteInChunk =
        std::min(BytesLeft, getBlockSize() - OffsetInBlock);

    const uint8_t *Chunk = Buffer.data() + BytesWritten;
    ArrayRef<uint8_t> ChunkData(Chunk, BytesToWriteInChunk);
    uint32_t MsfOffset = blockToOffset(StreamBlockAddr, getBlockSize());
    MsfOffset += OffsetInBlock;
    if (auto EC = WriteInterface.writeBytes(MsfOffset, ChunkData))
      return EC;

    BytesLeft -= BytesToWriteInChunk;
    BytesWritten += BytesToWriteInChunk;
    ++BlockNum;
    OffsetInBlock = 0;
  }

  ReadInterface.fixCacheAfterWrite(Offset, Buffer);

  return Error::success();
}

Error WritableMappedBlockStream::commit() const {
  return WriteInterface.commit();
}
