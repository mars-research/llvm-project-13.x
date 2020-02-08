; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mcpu=bdver2 -mattr=-fma -mtriple=i686-apple-darwin < %s | FileCheck %s
; RUN: llc -mcpu=bdver2 -mattr=-fma,-fma4 -mtriple=i686-apple-darwin < %s | FileCheck %s --check-prefix=CHECK-NOFMA

define <3 x float> @fmafunc(<3 x float> %a, <3 x float> %b, <3 x float> %c) {
; CHECK-LABEL: fmafunc:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vfmaddps %xmm2, %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-NOFMA-LABEL: fmafunc:
; CHECK-NOFMA:       ## %bb.0:
; CHECK-NOFMA-NEXT:    vmulps %xmm1, %xmm0, %xmm0
; CHECK-NOFMA-NEXT:    vaddps %xmm2, %xmm0, %xmm0
; CHECK-NOFMA-NEXT:    retl
  %ret = tail call <3 x float> @llvm.fmuladd.v3f32(<3 x float> %a, <3 x float> %b, <3 x float> %c)
  ret <3 x float> %ret
}

declare <3 x float> @llvm.fmuladd.v3f32(<3 x float>, <3 x float>, <3 x float>) nounwind readnone
