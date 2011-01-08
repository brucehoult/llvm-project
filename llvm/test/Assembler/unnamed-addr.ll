; RUN: llvm-as < %s | llvm-dis | FileCheck %s

%struct.foobar = type { i32 }

@bar.d = internal unnamed_addr constant %struct.foobar zeroinitializer, align 4
@foo.d = internal constant %struct.foobar zeroinitializer, align 4

define unnamed_addr i32 @main() nounwind ssp {
entry:
  %call2 = tail call i32 @zed(%struct.foobar* @foo.d, %struct.foobar* @bar.d) nounwind
  ret i32 0
}

declare i32 @zed(%struct.foobar*, %struct.foobar*)

; CHECK: @bar.d = internal unnamed_addr constant %struct.foobar zeroinitializer, align 4
; CHECK: @foo.d = internal constant %struct.foobar zeroinitializer, align 4
; CHECK: define unnamed_addr i32 @main() nounwind ssp {
