// RUN: llvm-mc -filetype=obj -triple x86_64-pc-linux-gnu %s -o - | elf-dump  | FileCheck %s

// Test that these names are accepted.

.section	.note.GNU-stack,"",@progbits
.section	.note.GNU-,"",@progbits
.section	-.note.GNU,"",@progbits

// CHECK: ('sh_name', 0x00000012) # '.note.GNU-stack'
// CHECK: ('sh_name', 0x00000022) # '.note.GNU-'
// CHECK: ('sh_name', 0x0000002d) # '-.note.GNU'

// Test that the dafults are used

.section	.init
.section	.fini
.section	.rodata

// CHECK:      (('sh_name', 0x00000038) # '.init'
// CHECK-NEXT:  ('sh_type', 0x00000001)
// CHECK-NEXT:  ('sh_flags', 0x00000006)
// CHECK-NEXT:  ('sh_addr', 0x00000000)
// CHECK-NEXT:  ('sh_offset', 0x00000050)
// CHECK-NEXT:  ('sh_size', 0x00000000)
// CHECK-NEXT:  ('sh_link', 0x00000000)
// CHECK-NEXT:  ('sh_info', 0x00000000)
// CHECK-NEXT:  ('sh_addralign', 0x00000001)
// CHECK-NEXT:  ('sh_entsize', 0x00000000)
// CHECK-NEXT: ),
// CHECK-NEXT: # Section 0x0000000a
// CHECK-NEXT: (('sh_name', 0x0000003e) # '.fini'
// CHECK-NEXT:  ('sh_type', 0x00000001)
// CHECK-NEXT:  ('sh_flags', 0x00000006)
// CHECK-NEXT:  ('sh_addr', 0x00000000)
// CHECK-NEXT:  ('sh_offset', 0x00000050)
// CHECK-NEXT:  ('sh_size', 0x00000000)
// CHECK-NEXT:  ('sh_link', 0x00000000)
// CHECK-NEXT:  ('sh_info', 0x00000000)
// CHECK-NEXT:  ('sh_addralign', 0x00000001)
// CHECK-NEXT:  ('sh_entsize', 0x00000000)
// CHECK-NEXT: ),
// CHECK-NEXT: # Section 0x0000000b
// CHECK-NEXT: (('sh_name', 0x00000044) # '.rodata'
// CHECK-NEXT:  ('sh_type', 0x00000001)
// CHECK-NEXT:  ('sh_flags', 0x00000002)
// CHECK-NEXT:  ('sh_addr', 0x00000000)
// CHECK-NEXT:  ('sh_offset', 0x00000050)
// CHECK-NEXT:  ('sh_size', 0x00000000)
// CHECK-NEXT:  ('sh_link', 0x00000000)
// CHECK-NEXT:  ('sh_info', 0x00000000)
// CHECK-NEXT:  ('sh_addralign', 0x00000001)
// CHECK-NEXT:  ('sh_entsize', 0x00000000)
// CHECK-NEXT: ),

// Test that we can parse these
foo:
bar:
.section        .text.foo,"axG",@progbits,foo,comdat
.section        .text.bar,"axMG",@progbits,42,bar,comdat
