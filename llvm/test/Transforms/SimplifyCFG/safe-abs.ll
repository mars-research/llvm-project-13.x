; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -simplifycfg -simplifycfg-require-and-preserve-domtree=1 -S | FileCheck %s

; Reduced from arm_abs_q31() from CMSIS DSP suite.
; https://reviews.llvm.org/D65148#1629010

define i32 @abs_with_clamp(i32 %arg) {
; CHECK-LABEL: @abs_with_clamp(
; CHECK-NEXT:  begin:
; CHECK-NEXT:    [[IS_POSITIVE:%.*]] = icmp sgt i32 [[ARG:%.*]], 0
; CHECK-NEXT:    [[IS_INT_MIN:%.*]] = icmp eq i32 [[ARG]], -2147483648
; CHECK-NEXT:    [[NEGATED:%.*]] = sub nsw i32 0, [[ARG]]
; CHECK-NEXT:    [[ABS:%.*]] = select i1 [[IS_INT_MIN]], i32 2147483647, i32 [[NEGATED]]
; CHECK-NEXT:    [[TMP6:%.*]] = select i1 [[IS_POSITIVE]], i32 [[ARG]], i32 [[ABS]]
; CHECK-NEXT:    ret i32 [[TMP6]]
;
begin:
  %is_positive = icmp sgt i32 %arg, 0
  br i1 %is_positive, label %end, label %negative

negative: ; preds = %begin
  %is_int_min = icmp eq i32 %arg, -2147483648
  %negated = sub nsw i32 0, %arg
  %abs = select i1 %is_int_min, i32 2147483647, i32 %negated
  br label %end

end:      ; preds = %negative, %begin
  %tmp6 = phi i32 [ %arg, %begin ], [ %abs, %negative ]
  ret i32 %tmp6
}
