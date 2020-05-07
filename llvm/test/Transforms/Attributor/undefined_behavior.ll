; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --function-signature --scrub-attributes
; RUN: opt -attributor -attributor-manifest-internal  -attributor-max-iterations-verify -attributor-annotate-decl-cs -attributor-max-iterations=5 -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_CGSCC_NPM,NOT_CGSCC_OPM,NOT_TUNIT_NPM,IS__TUNIT____,IS________OPM,IS__TUNIT_OPM
; RUN: opt -aa-pipeline=basic-aa -passes=attributor -attributor-manifest-internal  -attributor-max-iterations-verify -attributor-annotate-decl-cs -attributor-max-iterations=5 -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_CGSCC_OPM,NOT_CGSCC_NPM,NOT_TUNIT_OPM,IS__TUNIT____,IS________NPM,IS__TUNIT_NPM
; RUN: opt -attributor-cgscc -attributor-manifest-internal  -attributor-annotate-decl-cs -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_TUNIT_NPM,NOT_TUNIT_OPM,NOT_CGSCC_NPM,IS__CGSCC____,IS________OPM,IS__CGSCC_OPM
; RUN: opt -aa-pipeline=basic-aa -passes=attributor-cgscc -attributor-manifest-internal  -attributor-annotate-decl-cs -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_TUNIT_NPM,NOT_TUNIT_OPM,NOT_CGSCC_OPM,IS__CGSCC____,IS________NPM,IS__CGSCC_NPM

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test cases specifically designed for the "undefined behavior" abstract function attribute.
; We want to verify that whenever undefined behavior is assumed, the code becomes unreachable.
; We use FIXME's to indicate problems and missing attributes.

; -- Load tests --

define void @load_wholly_unreachable() {
; CHECK-LABEL: define {{[^@]+}}@load_wholly_unreachable()
; CHECK-NEXT:    unreachable
;
  %a = load i32, i32* null
  ret void
}

define void @loads_wholly_unreachable() {
; CHECK-LABEL: define {{[^@]+}}@loads_wholly_unreachable()
; CHECK-NEXT:    unreachable
;
  %a = load i32, i32* null
  %b = load i32, i32* null
  ret void
}


define void @load_single_bb_unreachable(i1 %cond) {
; CHECK-LABEL: define {{[^@]+}}@load_single_bb_unreachable
; CHECK-SAME: (i1 [[COND:%.*]])
; CHECK-NEXT:    br i1 [[COND]], label [[T:%.*]], label [[E:%.*]]
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    ret void
;
  br i1 %cond, label %t, label %e
t:
  %b = load i32, i32* null
  br label %e
e:
  ret void
}

; Note that while the load is removed (because it's unused), the block
; is not changed to unreachable
define void @load_null_pointer_is_defined() "null-pointer-is-valid"="true" {
; CHECK-LABEL: define {{[^@]+}}@load_null_pointer_is_defined()
; CHECK-NEXT:    ret void
;
  %a = load i32, i32* null
  ret void
}

define internal i32* @ret_null() {
; IS__CGSCC____-LABEL: define {{[^@]+}}@ret_null()
; IS__CGSCC____-NEXT:    ret i32* null
;
  ret i32* null
}

define void @load_null_propagated() {
; CHECK-LABEL: define {{[^@]+}}@load_null_propagated()
; CHECK-NEXT:    unreachable
;
  %ptr = call i32* @ret_null()
  %a = load i32, i32* %ptr
  ret void
}

; -- Store tests --

define void @store_wholly_unreachable() {
; CHECK-LABEL: define {{[^@]+}}@store_wholly_unreachable()
; CHECK-NEXT:    unreachable
;
  store i32 5, i32* null
  ret void
}

define void @store_single_bb_unreachable(i1 %cond) {
; CHECK-LABEL: define {{[^@]+}}@store_single_bb_unreachable
; CHECK-SAME: (i1 [[COND:%.*]])
; CHECK-NEXT:    br i1 [[COND]], label [[T:%.*]], label [[E:%.*]]
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    ret void
;
  br i1 %cond, label %t, label %e
t:
  store i32 5, i32* null
  br label %e
e:
  ret void
}

define void @store_null_pointer_is_defined() "null-pointer-is-valid"="true" {
; CHECK-LABEL: define {{[^@]+}}@store_null_pointer_is_defined()
; CHECK-NEXT:    store i32 5, i32* null, align 536870912
; CHECK-NEXT:    ret void
;
  store i32 5, i32* null
  ret void
}

define void @store_null_propagated() {
; ATTRIBUTOR-LABEL: @store_null_propagated(
; ATTRIBUTOR-NEXT:    unreachable
;
; CHECK-LABEL: define {{[^@]+}}@store_null_propagated()
; CHECK-NEXT:    unreachable
;
  %ptr = call i32* @ret_null()
  store i32 5, i32* %ptr
  ret void
}

; -- AtomicRMW tests --

define void @atomicrmw_wholly_unreachable() {
; CHECK-LABEL: define {{[^@]+}}@atomicrmw_wholly_unreachable()
; CHECK-NEXT:    unreachable
;
  %a = atomicrmw add i32* null, i32 1 acquire
  ret void
}

define void @atomicrmw_single_bb_unreachable(i1 %cond) {
; CHECK-LABEL: define {{[^@]+}}@atomicrmw_single_bb_unreachable
; CHECK-SAME: (i1 [[COND:%.*]])
; CHECK-NEXT:    br i1 [[COND]], label [[T:%.*]], label [[E:%.*]]
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    ret void
;
  br i1 %cond, label %t, label %e
t:
  %a = atomicrmw add i32* null, i32 1 acquire
  br label %e
e:
  ret void
}

define void @atomicrmw_null_pointer_is_defined() "null-pointer-is-valid"="true" {
; CHECK-LABEL: define {{[^@]+}}@atomicrmw_null_pointer_is_defined()
; CHECK-NEXT:    [[A:%.*]] = atomicrmw add i32* null, i32 1 acquire
; CHECK-NEXT:    ret void
;
  %a = atomicrmw add i32* null, i32 1 acquire
  ret void
}

define void @atomicrmw_null_propagated() {
; ATTRIBUTOR-LABEL: @atomicrmw_null_propagated(
; ATTRIBUTOR-NEXT:    unreachable
;
; CHECK-LABEL: define {{[^@]+}}@atomicrmw_null_propagated()
; CHECK-NEXT:    unreachable
;
  %ptr = call i32* @ret_null()
  %a = atomicrmw add i32* %ptr, i32 1 acquire
  ret void
}

; -- AtomicCmpXchg tests --

define void @atomiccmpxchg_wholly_unreachable() {
; CHECK-LABEL: define {{[^@]+}}@atomiccmpxchg_wholly_unreachable()
; CHECK-NEXT:    unreachable
;
  %a = cmpxchg i32* null, i32 2, i32 3 acq_rel monotonic
  ret void
}

define void @atomiccmpxchg_single_bb_unreachable(i1 %cond) {
; CHECK-LABEL: define {{[^@]+}}@atomiccmpxchg_single_bb_unreachable
; CHECK-SAME: (i1 [[COND:%.*]])
; CHECK-NEXT:    br i1 [[COND]], label [[T:%.*]], label [[E:%.*]]
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    ret void
;
  br i1 %cond, label %t, label %e
t:
  %a = cmpxchg i32* null, i32 2, i32 3 acq_rel monotonic
  br label %e
e:
  ret void
}

define void @atomiccmpxchg_null_pointer_is_defined() "null-pointer-is-valid"="true" {
; CHECK-LABEL: define {{[^@]+}}@atomiccmpxchg_null_pointer_is_defined()
; CHECK-NEXT:    [[A:%.*]] = cmpxchg i32* null, i32 2, i32 3 acq_rel monotonic
; CHECK-NEXT:    ret void
;
  %a = cmpxchg i32* null, i32 2, i32 3 acq_rel monotonic
  ret void
}

define void @atomiccmpxchg_null_propagated() {
; ATTRIBUTOR-LABEL: @atomiccmpxchg_null_propagated(
; ATTRIBUTOR-NEXT:    unreachable
;
; CHECK-LABEL: define {{[^@]+}}@atomiccmpxchg_null_propagated()
; CHECK-NEXT:    unreachable
;
  %ptr = call i32* @ret_null()
  %a = cmpxchg i32* %ptr, i32 2, i32 3 acq_rel monotonic
  ret void
}

; -- Conditional branching tests --

; Note: The unreachable on %t and %e is _not_ from AAUndefinedBehavior

define i32 @cond_br_on_undef() {
; CHECK-LABEL: define {{[^@]+}}@cond_br_on_undef()
; CHECK-NEXT:    unreachable
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    unreachable
;
  br i1 undef, label %t, label %e
t:
  ret i32 1
e:
  ret i32 2
}

; More complicated branching
  ; Valid branch - verify that this is not converted
  ; to unreachable.
define void @cond_br_on_undef2(i1 %cond) {
; CHECK-LABEL: define {{[^@]+}}@cond_br_on_undef2
; CHECK-SAME: (i1 [[COND:%.*]])
; CHECK-NEXT:    br i1 [[COND]], label [[T1:%.*]], label [[E1:%.*]]
; CHECK:       t1:
; CHECK-NEXT:    unreachable
; CHECK:       t2:
; CHECK-NEXT:    unreachable
; CHECK:       e2:
; CHECK-NEXT:    unreachable
; CHECK:       e1:
; CHECK-NEXT:    ret void
;
  br i1 %cond, label %t1, label %e1
t1:
  br i1 undef, label %t2, label %e2
t2:
  ret void
e2:
  ret void
e1:
  ret void
}

define i1 @ret_undef() {
; CHECK-LABEL: define {{[^@]+}}@ret_undef()
; CHECK-NEXT:    ret i1 undef
;
  ret i1 undef
}

define void @cond_br_on_undef_interproc() {
; CHECK-LABEL: define {{[^@]+}}@cond_br_on_undef_interproc()
; CHECK-NEXT:    unreachable
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    unreachable
;
  %cond = call i1 @ret_undef()
  br i1 %cond, label %t, label %e
t:
  ret void
e:
  ret void
}

define i1 @ret_undef2() {
; CHECK-LABEL: define {{[^@]+}}@ret_undef2()
; CHECK-NEXT:    br i1 true, label [[T:%.*]], label [[E:%.*]]
; CHECK:       t:
; CHECK-NEXT:    ret i1 undef
; CHECK:       e:
; CHECK-NEXT:    unreachable
;
  br i1 true, label %t, label %e
t:
  ret i1 undef
e:
  ret i1 undef
}

; More complicated interproc deduction of undef
define void @cond_br_on_undef_interproc2() {
; CHECK-LABEL: define {{[^@]+}}@cond_br_on_undef_interproc2()
; CHECK-NEXT:    unreachable
; CHECK:       t:
; CHECK-NEXT:    unreachable
; CHECK:       e:
; CHECK-NEXT:    unreachable
;
  %cond = call i1 @ret_undef2()
  br i1 %cond, label %t, label %e
t:
  ret void
e:
  ret void
}

; Branch on undef that depends on propagation of
; undef of a previous instruction.
define i32 @cond_br_on_undef3() {
; CHECK-LABEL: define {{[^@]+}}@cond_br_on_undef3()
; CHECK-NEXT:    br label [[T:%.*]]
; CHECK:       t:
; CHECK-NEXT:    ret i32 1
; CHECK:       e:
; CHECK-NEXT:    unreachable
;
  %cond = icmp ne i32 1, undef
  br i1 %cond, label %t, label %e
t:
  ret i32 1
e:
  ret i32 2
}

; Branch on undef because of uninitialized value.
; FIXME: Currently it doesn't propagate the undef.
define i32 @cond_br_on_undef_uninit() {
; CHECK-LABEL: define {{[^@]+}}@cond_br_on_undef_uninit()
; CHECK-NEXT:    [[ALLOC:%.*]] = alloca i1
; CHECK-NEXT:    [[COND:%.*]] = load i1, i1* [[ALLOC]], align 1
; CHECK-NEXT:    br i1 [[COND]], label [[T:%.*]], label [[E:%.*]]
; CHECK:       t:
; CHECK-NEXT:    ret i32 1
; CHECK:       e:
; CHECK-NEXT:    ret i32 2
;
  %alloc = alloca i1
  %cond = load i1, i1* %alloc
  br i1 %cond, label %t, label %e
t:
  ret i32 1
e:
  ret i32 2
}

; Note that the `load` has UB (so it will be changed to unreachable)
; and the branch is a terminator that can be constant-folded.
; We want to test that doing both won't cause a segfault.
; MODULE-NOT: @callee(
define internal i32 @callee(i1 %C, i32* %A) {
;
; IS__CGSCC____-LABEL: define {{[^@]+}}@callee()
; IS__CGSCC____-NEXT:  entry:
; IS__CGSCC____-NEXT:    unreachable
; IS__CGSCC____:       T:
; IS__CGSCC____-NEXT:    unreachable
; IS__CGSCC____:       F:
; IS__CGSCC____-NEXT:    ret i32 1
;
entry:
  %A.0 = load i32, i32* null
  br i1 %C, label %T, label %F

T:
  ret i32 %A.0

F:
  ret i32 1
}

define i32 @foo() {
; CHECK-LABEL: define {{[^@]+}}@foo()
; CHECK-NEXT:    ret i32 1
;
  %X = call i32 @callee(i1 false, i32* null)
  ret i32 %X
}
