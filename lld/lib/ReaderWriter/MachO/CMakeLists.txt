add_lld_library(lldMachO
  ArchHandler.cpp
  ArchHandler_arm.cpp
  ArchHandler_arm64.cpp
  ArchHandler_x86.cpp
  ArchHandler_x86_64.cpp
  CompactUnwindPass.cpp
  GOTPass.cpp
  LayoutPass.cpp
  MachOLinkingContext.cpp
  MachONormalizedFileBinaryReader.cpp
  MachONormalizedFileBinaryWriter.cpp
  MachONormalizedFileFromAtoms.cpp
  MachONormalizedFileToAtoms.cpp
  MachONormalizedFileYAML.cpp
  ObjCPass.cpp
  ShimPass.cpp
  StubsPass.cpp
  TLVPass.cpp
  WriterMachO.cpp
  LINK_LIBS
    lldCore
    lldYAML
    LLVMDebugInfoDWARF
    LLVMObject
    LLVMSupport
    ${PTHREAD_LIB}
  )

include_directories(.)
