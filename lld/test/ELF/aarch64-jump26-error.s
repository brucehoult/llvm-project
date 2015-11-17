// RUN: llvm-mc -filetype=obj -triple=aarch64-pc-freebsd %S/Inputs/abs.s -o %tabs
// RUN: llvm-mc -filetype=obj -triple=aarch64-pc-freebsd %s -o %t
// RUN: not ld.lld2 %t %tabs -o %t2 2>&1 | FileCheck %s
// REQUIRES: aarch64

.text
.globl _start
_start:
    b big

// CHECK: R_AARCH64_JUMP26 out of range
