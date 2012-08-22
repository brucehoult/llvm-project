//===-- MipsSubtarget.h - Define Subtarget for the Mips ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mips specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MIPSSUBTARGET_H
#define MIPSSUBTARGET_H

#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "MipsGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class MipsSubtarget : public MipsGenSubtargetInfo {
  virtual void anchor();

public:
  // NOTE: O64 will not be supported.
  enum MipsABIEnum {
    UnknownABI, O32, N32, N64, EABI
  };

protected:

  enum MipsArchEnum {
    Mips32, Mips32r2, Mips64, Mips64r2
  };

  // Mips architecture version
  MipsArchEnum MipsArchVersion;

  // Mips supported ABIs
  MipsABIEnum MipsABI;

  // IsLittle - The target is Little Endian
  bool IsLittle;

  // IsSingleFloat - The target only supports single precision float
  // point operations. This enable the target to use all 32 32-bit
  // floating point registers instead of only using even ones.
  bool IsSingleFloat;

  // IsFP64bit - The target processor has 64-bit floating point registers.
  bool IsFP64bit;

  // IsFP64bit - General-purpose registers are 64 bits wide
  bool IsGP64bit;

  // HasVFPU - Processor has a vector floating point unit.
  bool HasVFPU;

  // isLinux - Target system is Linux. Is false we consider ELFOS for now.
  bool IsLinux;

  // UseSmallSection - Small section is used.
  bool UseSmallSection;

  /// Features related to the presence of specific instructions.

  // HasSEInReg - SEB and SEH (signext in register) instructions.
  bool HasSEInReg;

  // HasCondMov - Conditional mov (MOVZ, MOVN) instructions.
  bool HasCondMov;

  // HasMulDivAdd - Multiply add and sub (MADD, MADDu, MSUB, MSUBu)
  // instructions.
  bool HasMulDivAdd;

  // HasMinMax - MIN and MAX instructions.
  bool HasMinMax;

  // HasSwap - Byte and half swap instructions.
  bool HasSwap;

  // HasBitCount - Count leading '1' and '0' bits.
  bool HasBitCount;

  // InMips16 -- can process Mips16 instructions
  bool InMips16Mode;

  // IsAndroid -- target is android
  bool IsAndroid;

  InstrItineraryData InstrItins;

public:
  virtual bool enablePostRAScheduler(CodeGenOpt::Level OptLevel,
                                     AntiDepBreakMode& Mode,
                                     RegClassVector& CriticalPathRCs) const;

  /// Only O32 and EABI supported right now.
  bool isABI_EABI() const { return MipsABI == EABI; }
  bool isABI_N64() const { return MipsABI == N64; }
  bool isABI_N32() const { return MipsABI == N32; }
  bool isABI_O32() const { return MipsABI == O32; }
  unsigned getTargetABI() const { return MipsABI; }

  /// This constructor initializes the data members to match that
  /// of the specified triple.
  MipsSubtarget(const std::string &TT, const std::string &CPU,
                const std::string &FS, bool little, Reloc::Model RM);

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  bool hasMips32() const { return MipsArchVersion >= Mips32; }
  bool hasMips32r2() const { return MipsArchVersion == Mips32r2 ||
                                   MipsArchVersion == Mips64r2; }
  bool hasMips64() const { return MipsArchVersion >= Mips64; }
  bool hasMips64r2() const { return MipsArchVersion == Mips64r2; }

  bool hasMips32r2Or64() const { return hasMips32r2() || hasMips64(); }

  bool isLittle() const { return IsLittle; }
  bool isFP64bit() const { return IsFP64bit; }
  bool isGP64bit() const { return IsGP64bit; }
  bool isGP32bit() const { return !IsGP64bit; }
  bool isSingleFloat() const { return IsSingleFloat; }
  bool isNotSingleFloat() const { return !IsSingleFloat; }
  bool hasVFPU() const { return HasVFPU; }
  bool inMips16Mode() const { return InMips16Mode; }
  bool isAndroid() const { return IsAndroid; }
  bool isLinux() const { return IsLinux; }
  bool useSmallSection() const { return UseSmallSection; }

  bool hasStandardEncoding() const { return !inMips16Mode(); }

  /// Features related to the presence of specific instructions.
  bool hasSEInReg()   const { return HasSEInReg; }
  bool hasCondMov()   const { return HasCondMov; }
  bool hasMulDivAdd() const { return HasMulDivAdd; }
  bool hasMinMax()    const { return HasMinMax; }
  bool hasSwap()      const { return HasSwap; }
  bool hasBitCount()  const { return HasBitCount; }
};
} // End llvm namespace

#endif
