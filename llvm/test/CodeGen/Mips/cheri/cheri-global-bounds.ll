; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: %cheri_purecap_llc %s -o - -mxcaptable | FileCheck %s

; ModuleID = 'global.c'

@x = addrspace(200) global i64 0, align 8

; Function Attrs: nounwind
define void @foo(i64 %y) addrspace(200) #0 {
; CHECK-LABEL: foo:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; CHECK-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; CHECK-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    lui $1, %captab_hi(x)
; CHECK-NEXT:    daddiu $1, $1, %captab_lo(x)
; CHECK-NEXT:    clc $c1, $1, 0($c1)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csd $4, $zero, 0($c1)
entry:
  store i64 %y, i64 addrspace(200)* @x, align 4
  ret void
}

attributes #0 = { nounwind }
