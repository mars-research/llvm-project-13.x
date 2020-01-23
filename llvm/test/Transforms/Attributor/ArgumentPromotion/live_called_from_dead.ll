; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --function-signature --scrub-attributes
; RUN: opt -S -basicaa -attributor -attributor-disable=false -attributor-max-iterations-verify -attributor-max-iterations=3 < %s | FileCheck %s --check-prefixes=CHECK,OLDPM,OLDPM_MODULE
; RUN: opt -S -basicaa -attributor-cgscc -attributor-disable=false < %s | FileCheck %s --check-prefixes=CHECK,OLDPM,OLDPM_CGSCC
; RUN: opt -S -passes='attributor' -aa-pipeline='basic-aa' -attributor-disable=false -attributor-max-iterations-verify -attributor-max-iterations=3 < %s | FileCheck %s --check-prefixes=CHECK,NEWPM,NEWPM_MODULE
; RUN: opt -S -passes='attributor-cgscc' -aa-pipeline='basic-aa' -attributor-disable=false < %s | FileCheck %s --check-prefixes=CHECK,NEWPM,NEWPM_CGSCC

; OLDPM_MODULE-NOT: @dead
; NEWPM_MODULE-NOT: @dead
; OLDPM_CGSCC-NOT: @dead
; NEWPM_CGSCC-NOT: @dead

define internal void @dead() {
  call i32 @test(i32* null, i32* null)
  ret void
}

define internal i32 @test(i32* %X, i32* %Y) {
; OLDPM-LABEL: define {{[^@]+}}@test
; OLDPM-SAME: (i32* noalias nocapture nofree writeonly align 4 [[X:%.*]])
; OLDPM-NEXT:    br i1 true, label [[LIVE:%.*]], label [[DEAD:%.*]]
; OLDPM:       live:
; OLDPM-NEXT:    store i32 0, i32* [[X]], align 4
; OLDPM-NEXT:    ret i32 undef
; OLDPM:       dead:
; OLDPM-NEXT:    unreachable
;
; NEWPM_MODULE-LABEL: define {{[^@]+}}@test
; NEWPM_MODULE-SAME: (i32* noalias nocapture nofree writeonly align 4 [[X:%.*]])
; NEWPM_MODULE-NEXT:    br i1 true, label [[LIVE:%.*]], label [[DEAD:%.*]]
; NEWPM_MODULE:       live:
; NEWPM_MODULE-NEXT:    store i32 0, i32* [[X]], align 4
; NEWPM_MODULE-NEXT:    ret i32 undef
; NEWPM_MODULE:       dead:
; NEWPM_MODULE-NEXT:    unreachable
;
; NEWPM_CGSCC-LABEL: define {{[^@]+}}@test
; NEWPM_CGSCC-SAME: (i32* noalias nocapture nofree nonnull writeonly align 4 dereferenceable(4) [[X:%.*]])
; NEWPM_CGSCC-NEXT:    br i1 true, label [[LIVE:%.*]], label [[DEAD:%.*]]
; NEWPM_CGSCC:       live:
; NEWPM_CGSCC-NEXT:    store i32 0, i32* [[X]], align 4
; NEWPM_CGSCC-NEXT:    ret i32 undef
; NEWPM_CGSCC:       dead:
; NEWPM_CGSCC-NEXT:    unreachable
;
  br i1 true, label %live, label %dead
live:
  store i32 0, i32* %X
  ret i32 0
dead:
  call i32 @caller(i32* null)
  call void @dead()
  ret i32 1
}

define internal i32 @caller(i32* %B) {
; OLDPM_MODULE-LABEL: define {{[^@]+}}@caller()
; OLDPM_MODULE-NEXT:    [[A:%.*]] = alloca i32
; OLDPM_MODULE-NEXT:    store i32 1, i32* [[A]], align 4
; OLDPM_MODULE-NEXT:    [[C:%.*]] = call i32 @test(i32* noalias nocapture nofree nonnull writeonly align 4 dereferenceable(4) [[A]])
; OLDPM_MODULE-NEXT:    ret i32 undef
;
; OLDPM_CGSCC-LABEL: define {{[^@]+}}@caller()
; OLDPM_CGSCC-NEXT:    [[A:%.*]] = alloca i32
; OLDPM_CGSCC-NEXT:    store i32 1, i32* [[A]], align 4
; OLDPM_CGSCC-NEXT:    [[C:%.*]] = call i32 @test(i32* noalias nocapture nofree nonnull writeonly align 4 dereferenceable(4) [[A]])
; OLDPM_CGSCC-NEXT:    ret i32 0
;
; NEWPM_MODULE-LABEL: define {{[^@]+}}@caller()
; NEWPM_MODULE-NEXT:    [[A:%.*]] = alloca i32
; NEWPM_MODULE-NEXT:    store i32 1, i32* [[A]], align 4
; NEWPM_MODULE-NEXT:    [[C:%.*]] = call i32 @test(i32* noalias nocapture nofree nonnull writeonly align 4 dereferenceable(4) [[A]])
; NEWPM_MODULE-NEXT:    ret i32 undef
;
; NEWPM_CGSCC-LABEL: define {{[^@]+}}@caller()
; NEWPM_CGSCC-NEXT:    [[A:%.*]] = alloca i32
; NEWPM_CGSCC-NEXT:    store i32 1, i32* [[A]], align 4
; NEWPM_CGSCC-NEXT:    [[C:%.*]] = call i32 @test(i32* noalias nocapture nofree nonnull writeonly align 4 dereferenceable(4) [[A]])
; NEWPM_CGSCC-NEXT:    ret i32 0
;
  %A = alloca i32
  store i32 1, i32* %A
  %C = call i32 @test(i32* %A, i32* %B)
  ret i32 %C
}

define i32 @callercaller() {
; CHECK-LABEL: define {{[^@]+}}@callercaller()
; CHECK-NEXT:    [[B:%.*]] = alloca i32
; CHECK-NEXT:    store i32 2, i32* [[B]], align 4
; CHECK-NEXT:    [[X:%.*]] = call i32 @caller()
; CHECK-NEXT:    ret i32 0
;
  %B = alloca i32
  store i32 2, i32* %B
  %X = call i32 @caller(i32* %B)
  ret i32 %X
}

