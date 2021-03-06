// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %p/Inputs/shared.s -o %t2.o
// RUN: ld.lld %t2.o -shared -o %t2.so
// RUN: not ld.lld %t.o %t2.so -o %t 2>&1 | FileCheck %s

        .global _start
_start:
        .data
        .long bar - .

// CHECK: R_X86_64_PC32 cannot be used against shared object; recompile with -fPIC.
