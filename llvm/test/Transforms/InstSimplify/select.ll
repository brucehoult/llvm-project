; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -instsimplify -S | FileCheck %s

define i32 @test1(i32 %x) {
; CHECK-LABEL: @test1(
; CHECK-NEXT:    ret i32 %x
;
  %and = and i32 %x, 1
  %cmp = icmp eq i32 %and, 0
  %and1 = and i32 %x, -2
  %and1.x = select i1 %cmp, i32 %and1, i32 %x
  ret i32 %and1.x
}

define i32 @test2(i32 %x) {
; CHECK-LABEL: @test2(
; CHECK-NEXT:    ret i32 %x
;
  %and = and i32 %x, 1
  %cmp = icmp ne i32 %and, 0
  %and1 = and i32 %x, -2
  %and1.x = select i1 %cmp, i32 %x, i32 %and1
  ret i32 %and1.x
}

define i32 @test3(i32 %x) {
; CHECK-LABEL: @test3(
; CHECK-NEXT:    [[AND1:%.*]] = and i32 %x, -2
; CHECK-NEXT:    ret i32 [[AND1]]
;
  %and = and i32 %x, 1
  %cmp = icmp ne i32 %and, 0
  %and1 = and i32 %x, -2
  %and1.x = select i1 %cmp, i32 %and1, i32 %x
  ret i32 %and1.x
}

define i32 @test4(i32 %X) {
; CHECK-LABEL: @test4(
; CHECK-NEXT:    [[OR:%.*]] = or i32 %X, -2147483648
; CHECK-NEXT:    ret i32 [[OR]]
;
  %cmp = icmp slt i32 %X, 0
  %or = or i32 %X, -2147483648
  %cond = select i1 %cmp, i32 %X, i32 %or
  ret i32 %cond
}

define i32 @test5(i32 %X) {
; CHECK-LABEL: @test5(
; CHECK-NEXT:    ret i32 %X
;
  %cmp = icmp slt i32 %X, 0
  %or = or i32 %X, -2147483648
  %cond = select i1 %cmp, i32 %or, i32 %X
  ret i32 %cond
}

define i32 @test6(i32 %X) {
; CHECK-LABEL: @test6(
; CHECK-NEXT:    [[AND:%.*]] = and i32 %X, 2147483647
; CHECK-NEXT:    ret i32 [[AND]]
;
  %cmp = icmp slt i32 %X, 0
  %and = and i32 %X, 2147483647
  %cond = select i1 %cmp, i32 %and, i32 %X
  ret i32 %cond
}

define i32 @test7(i32 %X) {
; CHECK-LABEL: @test7(
; CHECK-NEXT:    ret i32 %X
;
  %cmp = icmp slt i32 %X, 0
  %and = and i32 %X, 2147483647
  %cond = select i1 %cmp, i32 %X, i32 %and
  ret i32 %cond
}

define i32 @test8(i32 %X) {
; CHECK-LABEL: @test8(
; CHECK-NEXT:    ret i32 %X
;
  %cmp = icmp sgt i32 %X, -1
  %or = or i32 %X, -2147483648
  %cond = select i1 %cmp, i32 %X, i32 %or
  ret i32 %cond
}

define i32 @test9(i32 %X) {
; CHECK-LABEL: @test9(
; CHECK-NEXT:    [[OR:%.*]] = or i32 %X, -2147483648
; CHECK-NEXT:    ret i32 [[OR]]
;
  %cmp = icmp sgt i32 %X, -1
  %or = or i32 %X, -2147483648
  %cond = select i1 %cmp, i32 %or, i32 %X
  ret i32 %cond
}

define i32 @test10(i32 %X) {
; CHECK-LABEL: @test10(
; CHECK-NEXT:    ret i32 %X
;
  %cmp = icmp sgt i32 %X, -1
  %and = and i32 %X, 2147483647
  %cond = select i1 %cmp, i32 %and, i32 %X
  ret i32 %cond
}

define i32 @test11(i32 %X) {
; CHECK-LABEL: @test11(
; CHECK-NEXT:    [[AND:%.*]] = and i32 %X, 2147483647
; CHECK-NEXT:    ret i32 [[AND]]
;
  %cmp = icmp sgt i32 %X, -1
  %and = and i32 %X, 2147483647
  %cond = select i1 %cmp, i32 %X, i32 %and
  ret i32 %cond
}

define i32 @select_icmp_and_8_eq_0_or_8(i32 %x) {
; CHECK-LABEL: @select_icmp_and_8_eq_0_or_8(
; CHECK-NEXT:    [[OR:%.*]] = or i32 %x, 8
; CHECK-NEXT:    ret i32 [[OR]]
;
  %and = and i32 %x, 8
  %cmp = icmp eq i32 %and, 0
  %or = or i32 %x, 8
  %or.x = select i1 %cmp, i32 %or, i32 %x
  ret i32 %or.x
}

; PR28466: https://llvm.org/bugs/show_bug.cgi?id=28466
; InstSimplify needs to recognize variations of this pattern.

define i32 @select_icmp_and_8_ne_0_or_128(i32 %x) {
; CHECK-LABEL: @select_icmp_and_8_ne_0_or_128(
; CHECK-NEXT:    ret i32 %x
;
  %and = and i32 %x, 128
  %cmp = icmp eq i32 %and, 0
  %or = or i32 %x, 128
  %or.x = select i1 %cmp, i32 %x, i32 %or
  ret i32 %or.x
}

define i32 @select_icmp_and_8_ne_0_and_not_8(i32 %x) {
; CHECK-LABEL: @select_icmp_and_8_ne_0_and_not_8(
; CHECK-NEXT:    [[AND1:%.*]] = and i32 %x, -9
; CHECK-NEXT:    ret i32 [[AND1]]
;
  %and = and i32 %x, 8
  %cmp = icmp eq i32 %and, 0
  %and1 = and i32 %x, -9
  %x.and1 = select i1 %cmp, i32 %x, i32 %and1
  ret i32 %x.and1
}

define i32 @select_icmp_and_8_eq_0_and_not_8(i32 %x) {
; CHECK-LABEL: @select_icmp_and_8_eq_0_and_not_8(
; CHECK-NEXT:    ret i32 %x
;
  %and = and i32 %x, 8
  %cmp = icmp eq i32 %and, 0
  %and1 = and i32 %x, -9
  %and1.x = select i1 %cmp, i32 %and1, i32 %x
  ret i32 %and1.x
}

define i64 @select_icmp_x_and_8_eq_0_y_and_not_8(i32 %x, i64 %y) {
; CHECK-LABEL: @select_icmp_x_and_8_eq_0_y_and_not_8(
; CHECK-NEXT:    [[AND:%.*]] = and i32 %x, 8
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i32 [[AND]], 0
; CHECK-NEXT:    [[AND1:%.*]] = and i64 %y, -9
; CHECK-NEXT:    [[Y_AND1:%.*]] = select i1 [[CMP]], i64 %y, i64 [[AND1]]
; CHECK-NEXT:    ret i64 [[Y_AND1]]
;
  %and = and i32 %x, 8
  %cmp = icmp eq i32 %and, 0
  %and1 = and i64 %y, -9
  %y.and1 = select i1 %cmp, i64 %y, i64 %and1
  ret i64 %y.and1
}

define i64 @select_icmp_x_and_8_ne_0_y_and_not_8(i32 %x, i64 %y) {
; CHECK-LABEL: @select_icmp_x_and_8_ne_0_y_and_not_8(
; CHECK-NEXT:    [[AND:%.*]] = and i32 %x, 8
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i32 [[AND]], 0
; CHECK-NEXT:    [[AND1:%.*]] = and i64 %y, -9
; CHECK-NEXT:    [[AND1_Y:%.*]] = select i1 [[CMP]], i64 [[AND1]], i64 %y
; CHECK-NEXT:    ret i64 [[AND1_Y]]
;
  %and = and i32 %x, 8
  %cmp = icmp eq i32 %and, 0
  %and1 = and i64 %y, -9
  %and1.y = select i1 %cmp, i64 %and1, i64 %y
  ret i64 %and1.y
}

