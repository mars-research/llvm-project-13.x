; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=arm-eabi -mattr=+neon %s -o - | FileCheck %s

define <8 x i8> @vuzpi8(<8 x i8>* %A, <8 x i8>* %B) nounwind {
; CHECK-LABEL: vuzpi8:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vldr d16, [r1]
; CHECK-NEXT:    vldr d17, [r0]
; CHECK-NEXT:    vuzp.8 d17, d16
; CHECK-NEXT:    vmul.i8 d16, d17, d16
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i8>, <8 x i8>* %A
	%tmp2 = load <8 x i8>, <8 x i8>* %B
	%tmp3 = shufflevector <8 x i8> %tmp1, <8 x i8> %tmp2, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
	%tmp4 = shufflevector <8 x i8> %tmp1, <8 x i8> %tmp2, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
        %tmp5 = mul <8 x i8> %tmp3, %tmp4
	ret <8 x i8> %tmp5
}

define <16 x i8> @vuzpi8_Qres(<8 x i8>* %A, <8 x i8>* %B) nounwind {
; CHECK-LABEL: vuzpi8_Qres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vldr d17, [r1]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vuzp.8 d16, d17
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i8>, <8 x i8>* %A
	%tmp2 = load <8 x i8>, <8 x i8>* %B
	%tmp3 = shufflevector <8 x i8> %tmp1, <8 x i8> %tmp2, <16 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14, i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
	ret <16 x i8> %tmp3
}

define <4 x i16> @vuzpi16(<4 x i16>* %A, <4 x i16>* %B) nounwind {
; CHECK-LABEL: vuzpi16:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vldr d16, [r1]
; CHECK-NEXT:    vldr d17, [r0]
; CHECK-NEXT:    vuzp.16 d17, d16
; CHECK-NEXT:    vmul.i16 d16, d17, d16
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <4 x i16>, <4 x i16>* %A
	%tmp2 = load <4 x i16>, <4 x i16>* %B
	%tmp3 = shufflevector <4 x i16> %tmp1, <4 x i16> %tmp2, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
	%tmp4 = shufflevector <4 x i16> %tmp1, <4 x i16> %tmp2, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
        %tmp5 = mul <4 x i16> %tmp3, %tmp4
	ret <4 x i16> %tmp5
}

define <8 x i16> @vuzpi16_Qres(<4 x i16>* %A, <4 x i16>* %B) nounwind {
; CHECK-LABEL: vuzpi16_Qres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vldr d17, [r1]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vuzp.16 d16, d17
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <4 x i16>, <4 x i16>* %A
	%tmp2 = load <4 x i16>, <4 x i16>* %B
	%tmp3 = shufflevector <4 x i16> %tmp1, <4 x i16> %tmp2, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
	ret <8 x i16> %tmp3
}

; VUZP.32 is equivalent to VTRN.32 for 64-bit vectors.

define <16 x i8> @vuzpQi8(<16 x i8>* %A, <16 x i8>* %B) nounwind {
; CHECK-LABEL: vuzpQi8:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r1]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r0]
; CHECK-NEXT:    vuzp.8 q9, q8
; CHECK-NEXT:    vadd.i8 q8, q9, q8
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <16 x i8>, <16 x i8>* %A
	%tmp2 = load <16 x i8>, <16 x i8>* %B
	%tmp3 = shufflevector <16 x i8> %tmp1, <16 x i8> %tmp2, <16 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14, i32 16, i32 18, i32 20, i32 22, i32 24, i32 26, i32 28, i32 30>
	%tmp4 = shufflevector <16 x i8> %tmp1, <16 x i8> %tmp2, <16 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15, i32 17, i32 19, i32 21, i32 23, i32 25, i32 27, i32 29, i32 31>
        %tmp5 = add <16 x i8> %tmp3, %tmp4
	ret <16 x i8> %tmp5
}

define <32 x i8> @vuzpQi8_QQres(<16 x i8>* %A, <16 x i8>* %B) nounwind {
; CHECK-LABEL: vuzpQi8_QQres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r2]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r1]
; CHECK-NEXT:    vuzp.8 q9, q8
; CHECK-NEXT:    vst1.8 {d18, d19}, [r0:128]!
; CHECK-NEXT:    vst1.64 {d16, d17}, [r0:128]
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <16 x i8>, <16 x i8>* %A
	%tmp2 = load <16 x i8>, <16 x i8>* %B
	%tmp3 = shufflevector <16 x i8> %tmp1, <16 x i8> %tmp2, <32 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14, i32 16, i32 18, i32 20, i32 22, i32 24, i32 26, i32 28, i32 30, i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15, i32 17, i32 19, i32 21, i32 23, i32 25, i32 27, i32 29, i32 31>
	ret <32 x i8> %tmp3
}

define <8 x i16> @vuzpQi16(<8 x i16>* %A, <8 x i16>* %B) nounwind {
; CHECK-LABEL: vuzpQi16:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r1]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r0]
; CHECK-NEXT:    vuzp.16 q9, q8
; CHECK-NEXT:    vadd.i16 q8, q9, q8
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i16>, <8 x i16>* %A
	%tmp2 = load <8 x i16>, <8 x i16>* %B
	%tmp3 = shufflevector <8 x i16> %tmp1, <8 x i16> %tmp2, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
	%tmp4 = shufflevector <8 x i16> %tmp1, <8 x i16> %tmp2, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
        %tmp5 = add <8 x i16> %tmp3, %tmp4
	ret <8 x i16> %tmp5
}

define <16 x i16> @vuzpQi16_QQres(<8 x i16>* %A, <8 x i16>* %B) nounwind {
; CHECK-LABEL: vuzpQi16_QQres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r2]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r1]
; CHECK-NEXT:    vuzp.16 q9, q8
; CHECK-NEXT:    vst1.16 {d18, d19}, [r0:128]!
; CHECK-NEXT:    vst1.64 {d16, d17}, [r0:128]
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i16>, <8 x i16>* %A
	%tmp2 = load <8 x i16>, <8 x i16>* %B
	%tmp3 = shufflevector <8 x i16> %tmp1, <8 x i16> %tmp2, <16 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14, i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
	ret <16 x i16> %tmp3
}

define <4 x i32> @vuzpQi32(<4 x i32>* %A, <4 x i32>* %B) nounwind {
; CHECK-LABEL: vuzpQi32:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r1]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r0]
; CHECK-NEXT:    vuzp.32 q9, q8
; CHECK-NEXT:    vadd.i32 q8, q9, q8
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <4 x i32>, <4 x i32>* %A
	%tmp2 = load <4 x i32>, <4 x i32>* %B
	%tmp3 = shufflevector <4 x i32> %tmp1, <4 x i32> %tmp2, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
	%tmp4 = shufflevector <4 x i32> %tmp1, <4 x i32> %tmp2, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
        %tmp5 = add <4 x i32> %tmp3, %tmp4
	ret <4 x i32> %tmp5
}

define <8 x i32> @vuzpQi32_QQres(<4 x i32>* %A, <4 x i32>* %B) nounwind {
; CHECK-LABEL: vuzpQi32_QQres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r2]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r1]
; CHECK-NEXT:    vuzp.32 q9, q8
; CHECK-NEXT:    vst1.32 {d18, d19}, [r0:128]!
; CHECK-NEXT:    vst1.64 {d16, d17}, [r0:128]
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <4 x i32>, <4 x i32>* %A
	%tmp2 = load <4 x i32>, <4 x i32>* %B
	%tmp3 = shufflevector <4 x i32> %tmp1, <4 x i32> %tmp2, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
	ret <8 x i32> %tmp3
}

define <4 x float> @vuzpQf(<4 x float>* %A, <4 x float>* %B) nounwind {
; CHECK-LABEL: vuzpQf:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r1]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r0]
; CHECK-NEXT:    vuzp.32 q9, q8
; CHECK-NEXT:    vadd.f32 q8, q9, q8
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <4 x float>, <4 x float>* %A
	%tmp2 = load <4 x float>, <4 x float>* %B
	%tmp3 = shufflevector <4 x float> %tmp1, <4 x float> %tmp2, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
	%tmp4 = shufflevector <4 x float> %tmp1, <4 x float> %tmp2, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
        %tmp5 = fadd <4 x float> %tmp3, %tmp4
	ret <4 x float> %tmp5
}

define <8 x float> @vuzpQf_QQres(<4 x float>* %A, <4 x float>* %B) nounwind {
; CHECK-LABEL: vuzpQf_QQres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r2]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r1]
; CHECK-NEXT:    vuzp.32 q9, q8
; CHECK-NEXT:    vst1.32 {d18, d19}, [r0:128]!
; CHECK-NEXT:    vst1.64 {d16, d17}, [r0:128]
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <4 x float>, <4 x float>* %A
	%tmp2 = load <4 x float>, <4 x float>* %B
	%tmp3 = shufflevector <4 x float> %tmp1, <4 x float> %tmp2, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
	ret <8 x float> %tmp3
}

; Undef shuffle indices should not prevent matching to VUZP:

define <8 x i8> @vuzpi8_undef(<8 x i8>* %A, <8 x i8>* %B) nounwind {
; CHECK-LABEL: vuzpi8_undef:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vldr d16, [r1]
; CHECK-NEXT:    vldr d17, [r0]
; CHECK-NEXT:    vuzp.8 d17, d16
; CHECK-NEXT:    vmul.i8 d16, d17, d16
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i8>, <8 x i8>* %A
	%tmp2 = load <8 x i8>, <8 x i8>* %B
	%tmp3 = shufflevector <8 x i8> %tmp1, <8 x i8> %tmp2, <8 x i32> <i32 0, i32 2, i32 undef, i32 undef, i32 8, i32 10, i32 12, i32 14>
	%tmp4 = shufflevector <8 x i8> %tmp1, <8 x i8> %tmp2, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 undef, i32 undef, i32 13, i32 15>
        %tmp5 = mul <8 x i8> %tmp3, %tmp4
	ret <8 x i8> %tmp5
}

define <16 x i8> @vuzpi8_undef_Qres(<8 x i8>* %A, <8 x i8>* %B) nounwind {
; CHECK-LABEL: vuzpi8_undef_Qres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vldr d17, [r1]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vuzp.8 d16, d17
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i8>, <8 x i8>* %A
	%tmp2 = load <8 x i8>, <8 x i8>* %B
	%tmp3 = shufflevector <8 x i8> %tmp1, <8 x i8> %tmp2, <16 x i32> <i32 0, i32 2, i32 undef, i32 undef, i32 8, i32 10, i32 12, i32 14, i32 1, i32 3, i32 5, i32 7, i32 undef, i32 undef, i32 13, i32 15>
	ret <16 x i8> %tmp3
}

define <8 x i16> @vuzpQi16_undef(<8 x i16>* %A, <8 x i16>* %B) nounwind {
; CHECK-LABEL: vuzpQi16_undef:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r1]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r0]
; CHECK-NEXT:    vuzp.16 q9, q8
; CHECK-NEXT:    vadd.i16 q8, q9, q8
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i16>, <8 x i16>* %A
	%tmp2 = load <8 x i16>, <8 x i16>* %B
	%tmp3 = shufflevector <8 x i16> %tmp1, <8 x i16> %tmp2, <8 x i32> <i32 0, i32 undef, i32 4, i32 undef, i32 8, i32 10, i32 12, i32 14>
	%tmp4 = shufflevector <8 x i16> %tmp1, <8 x i16> %tmp2, <8 x i32> <i32 1, i32 3, i32 5, i32 undef, i32 undef, i32 11, i32 13, i32 15>
        %tmp5 = add <8 x i16> %tmp3, %tmp4
	ret <8 x i16> %tmp5
}

define <16 x i16> @vuzpQi16_undef_QQres(<8 x i16>* %A, <8 x i16>* %B) nounwind {
; CHECK-LABEL: vuzpQi16_undef_QQres:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vld1.64 {d16, d17}, [r2]
; CHECK-NEXT:    vld1.64 {d18, d19}, [r1]
; CHECK-NEXT:    vuzp.16 q9, q8
; CHECK-NEXT:    vst1.16 {d18, d19}, [r0:128]!
; CHECK-NEXT:    vst1.64 {d16, d17}, [r0:128]
; CHECK-NEXT:    mov pc, lr
	%tmp1 = load <8 x i16>, <8 x i16>* %A
	%tmp2 = load <8 x i16>, <8 x i16>* %B
	%tmp3 = shufflevector <8 x i16> %tmp1, <8 x i16> %tmp2, <16 x i32> <i32 0, i32 undef, i32 4, i32 undef, i32 8, i32 10, i32 12, i32 14, i32 1, i32 3, i32 5, i32 undef, i32 undef, i32 11, i32 13, i32 15>
	ret <16 x i16> %tmp3
}

define <8 x i16> @vuzp_lower_shufflemask_undef(<4 x i16>* %A, <4 x i16>* %B) {
; CHECK-LABEL: vuzp_lower_shufflemask_undef:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vldr d17, [r1]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vorr q9, q8, q8
; CHECK-NEXT:    vuzp.16 q8, q9
; CHECK-NEXT:    vmov r0, r1, d18
; CHECK-NEXT:    vmov r2, r3, d19
; CHECK-NEXT:    mov pc, lr
entry:
	%tmp1 = load <4 x i16>, <4 x i16>* %A
	%tmp2 = load <4 x i16>, <4 x i16>* %B
  %0 = shufflevector <4 x i16> %tmp1, <4 x i16> %tmp2, <8 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 1, i32 3, i32 5, i32 7>
  ret <8 x i16> %0
}

define <4 x i32> @vuzp_lower_shufflemask_zeroed(<2 x i32>* %A, <2 x i32>* %B) {
; CHECK-LABEL: vuzp_lower_shufflemask_zeroed:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vldr d17, [r1]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vdup.32 q9, d16[0]
; CHECK-NEXT:    vuzp.32 q8, q9
; CHECK-NEXT:    vext.32 q8, q9, q9, #2
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    mov pc, lr
entry:
  %tmp1 = load <2 x i32>, <2 x i32>* %A
	%tmp2 = load <2 x i32>, <2 x i32>* %B
  %0 = shufflevector <2 x i32> %tmp1, <2 x i32> %tmp2, <4 x i32> <i32 0, i32 0, i32 1, i32 3>
  ret <4 x i32> %0
}

define void @vuzp_rev_shufflemask_vtrn(<2 x i32>* %A, <2 x i32>* %B, <4 x i32>* %C) {
; CHECK-LABEL: vuzp_rev_shufflemask_vtrn:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vldr d17, [r1]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vrev64.32 q9, q8
; CHECK-NEXT:    vuzp.32 q8, q9
; CHECK-NEXT:    vst1.64 {d18, d19}, [r2]
; CHECK-NEXT:    mov pc, lr
entry:
  %tmp1 = load <2 x i32>, <2 x i32>* %A
  %tmp2 = load <2 x i32>, <2 x i32>* %B
  %0 = shufflevector <2 x i32> %tmp1, <2 x i32> %tmp2, <4 x i32> <i32 1, i32 3, i32 0, i32 2>
  store <4 x i32> %0, <4 x i32>* %C
  ret void
}

define <8 x i8> @cmpsel_trunc(<8 x i8> %in0, <8 x i8> %in1, <8 x i32> %cmp0, <8 x i32> %cmp1) {
; In order to create the select we need to truncate the vcgt result from a vector of i32 to a vector of i8.
; This results in a build_vector with mismatched types. We will generate two vmovn.i32 instructions to
; truncate from i32 to i16 and one vmovn.i16 to perform the final truncation for i8.
; CHECK-LABEL: cmpsel_trunc:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    add r12, sp, #16
; CHECK-NEXT:    vld1.64 {d16, d17}, [r12]
; CHECK-NEXT:    mov r12, sp
; CHECK-NEXT:    vld1.64 {d18, d19}, [r12]
; CHECK-NEXT:    add r12, sp, #48
; CHECK-NEXT:    vld1.64 {d20, d21}, [r12]
; CHECK-NEXT:    add r12, sp, #32
; CHECK-NEXT:    vcgt.u32 q8, q10, q8
; CHECK-NEXT:    vld1.64 {d20, d21}, [r12]
; CHECK-NEXT:    vcgt.u32 q9, q10, q9
; CHECK-NEXT:    vmov d20, r2, r3
; CHECK-NEXT:    vmovn.i32 d17, q8
; CHECK-NEXT:    vmovn.i32 d16, q9
; CHECK-NEXT:    vmov d18, r0, r1
; CHECK-NEXT:    vmovn.i16 d16, q8
; CHECK-NEXT:    vbsl d16, d18, d20
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    mov pc, lr
  %c = icmp ult <8 x i32> %cmp0, %cmp1
  %res = select <8 x i1> %c, <8 x i8> %in0, <8 x i8> %in1
  ret <8 x i8> %res
}

; Shuffle the result from the compare with a <4 x i8>.
; We need to extend the loaded <4 x i8> to <4 x i16>. Otherwise we wouldn't be able
; to perform the vuzp and get the vbsl mask.
define <8 x i8> @vuzp_trunc_and_shuffle(<8 x i8> %tr0, <8 x i8> %tr1,
; CHECK-LABEL: vuzp_trunc_and_shuffle:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    .save {r11, lr}
; CHECK-NEXT:    push {r11, lr}
; CHECK-NEXT:    add r12, sp, #8
; CHECK-NEXT:    add lr, sp, #24
; CHECK-NEXT:    vld1.64 {d16, d17}, [r12]
; CHECK-NEXT:    ldr r12, [sp, #40]
; CHECK-NEXT:    vld1.64 {d18, d19}, [lr]
; CHECK-NEXT:    vcgt.u32 q8, q9, q8
; CHECK-NEXT:    vld1.32 {d18[0]}, [r12:32]
; CHECK-NEXT:    vmov.i8 d19, #0x7
; CHECK-NEXT:    vmovl.u8 q10, d18
; CHECK-NEXT:    vmovn.i32 d16, q8
; CHECK-NEXT:    vneg.s8 d17, d19
; CHECK-NEXT:    vmov d18, r2, r3
; CHECK-NEXT:    vuzp.8 d16, d20
; CHECK-NEXT:    vshl.i8 d16, d16, #7
; CHECK-NEXT:    vshl.s8 d16, d16, d17
; CHECK-NEXT:    vmov d17, r0, r1
; CHECK-NEXT:    vbsl d16, d17, d18
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    pop {r11, lr}
; CHECK-NEXT:    mov pc, lr
                         <4 x i32> %cmp0, <4 x i32> %cmp1, <4 x i8> *%cmp2_ptr) {
  %cmp2_load = load <4 x i8>, <4 x i8> * %cmp2_ptr, align 4
  %cmp2 = trunc <4 x i8> %cmp2_load to <4 x i1>
  %c0 = icmp ult <4 x i32> %cmp0, %cmp1
  %c = shufflevector <4 x i1> %c0, <4 x i1> %cmp2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %rv = select <8 x i1> %c, <8 x i8> %tr0, <8 x i8> %tr1
  ret <8 x i8> %rv
}

; Use an undef value for the <4 x i8> that is being shuffled with the compare result.
; This produces a build_vector with some of the operands undefs.
define <8 x i8> @vuzp_trunc_and_shuffle_undef_right(<8 x i8> %tr0, <8 x i8> %tr1,
; CHECK-LABEL: vuzp_trunc_and_shuffle_undef_right:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    mov r12, sp
; CHECK-NEXT:    vld1.64 {d16, d17}, [r12]
; CHECK-NEXT:    add r12, sp, #16
; CHECK-NEXT:    vld1.64 {d18, d19}, [r12]
; CHECK-NEXT:    vcgt.u32 q8, q9, q8
; CHECK-NEXT:    vmov.i8 d18, #0x7
; CHECK-NEXT:    vmovn.i32 d16, q8
; CHECK-NEXT:    vuzp.8 d16, d17
; CHECK-NEXT:    vneg.s8 d17, d18
; CHECK-NEXT:    vshl.i8 d16, d16, #7
; CHECK-NEXT:    vmov d18, r2, r3
; CHECK-NEXT:    vshl.s8 d16, d16, d17
; CHECK-NEXT:    vmov d17, r0, r1
; CHECK-NEXT:    vbsl d16, d17, d18
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    mov pc, lr
                         <4 x i32> %cmp0, <4 x i32> %cmp1, <4 x i8> *%cmp2_ptr) {
  %cmp2_load = load <4 x i8>, <4 x i8> * %cmp2_ptr, align 4
  %cmp2 = trunc <4 x i8> %cmp2_load to <4 x i1>
  %c0 = icmp ult <4 x i32> %cmp0, %cmp1
  %c = shufflevector <4 x i1> %c0, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %rv = select <8 x i1> %c, <8 x i8> %tr0, <8 x i8> %tr1
  ret <8 x i8> %rv
}

define <8 x i8> @vuzp_trunc_and_shuffle_undef_left(<8 x i8> %tr0, <8 x i8> %tr1,
; CHECK-LABEL: vuzp_trunc_and_shuffle_undef_left:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    mov r12, sp
; CHECK-NEXT:    vld1.64 {d16, d17}, [r12]
; CHECK-NEXT:    add r12, sp, #16
; CHECK-NEXT:    vld1.64 {d18, d19}, [r12]
; CHECK-NEXT:    vcgt.u32 q8, q9, q8
; CHECK-NEXT:    vldr d18, .LCPI22_0
; CHECK-NEXT:    vmov.i8 d19, #0x7
; CHECK-NEXT:    vmovn.i32 d16, q8
; CHECK-NEXT:    vtbl.8 d16, {d16}, d18
; CHECK-NEXT:    vneg.s8 d17, d19
; CHECK-NEXT:    vmov d18, r2, r3
; CHECK-NEXT:    vshl.i8 d16, d16, #7
; CHECK-NEXT:    vshl.s8 d16, d16, d17
; CHECK-NEXT:    vmov d17, r0, r1
; CHECK-NEXT:    vbsl d16, d17, d18
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    mov pc, lr
; CHECK-NEXT:    .p2align 3
; CHECK-NEXT:  @ %bb.1:
; CHECK-NEXT:  .LCPI22_0:
; CHECK-NEXT:    .byte 255 @ 0xff
; CHECK-NEXT:    .byte 255 @ 0xff
; CHECK-NEXT:    .byte 255 @ 0xff
; CHECK-NEXT:    .byte 255 @ 0xff
; CHECK-NEXT:    .byte 0 @ 0x0
; CHECK-NEXT:    .byte 2 @ 0x2
; CHECK-NEXT:    .byte 4 @ 0x4
; CHECK-NEXT:    .byte 6 @ 0x6
                         <4 x i32> %cmp0, <4 x i32> %cmp1, <4 x i8> *%cmp2_ptr) {
  %cmp2_load = load <4 x i8>, <4 x i8> * %cmp2_ptr, align 4
  %cmp2 = trunc <4 x i8> %cmp2_load to <4 x i1>
  %c0 = icmp ult <4 x i32> %cmp0, %cmp1
  %c = shufflevector <4 x i1> undef, <4 x i1> %c0, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %rv = select <8 x i1> %c, <8 x i8> %tr0, <8 x i8> %tr1
  ret <8 x i8> %rv
}

; We're using large data types here, and we have to fill with undef values until we
; get some vector size that we can represent.
define <10 x i8> @vuzp_wide_type(<10 x i8> %tr0, <10 x i8> %tr1,
; CHECK-LABEL: vuzp_wide_type:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    .save {r4, lr}
; CHECK-NEXT:    push {r4, lr}
; CHECK-NEXT:    add r12, sp, #32
; CHECK-NEXT:    add lr, sp, #48
; CHECK-NEXT:    vld1.32 {d17[0]}, [r12:32]
; CHECK-NEXT:    add r12, sp, #24
; CHECK-NEXT:    vld1.32 {d16[0]}, [r12:32]
; CHECK-NEXT:    add r12, sp, #56
; CHECK-NEXT:    vld1.32 {d19[0]}, [r12:32]
; CHECK-NEXT:    vld1.32 {d18[0]}, [lr:32]
; CHECK-NEXT:    add lr, sp, #40
; CHECK-NEXT:    vld1.32 {d20[0]}, [lr:32]
; CHECK-NEXT:    ldr r12, [sp, #68]
; CHECK-NEXT:    ldr r4, [r12]
; CHECK-NEXT:    vmov.32 d23[0], r4
; CHECK-NEXT:    add r4, sp, #64
; CHECK-NEXT:    vld1.32 {d24[0]}, [r4:32]
; CHECK-NEXT:    add r4, sp, #36
; CHECK-NEXT:    vcgt.u32 q10, q12, q10
; CHECK-NEXT:    vld1.32 {d17[1]}, [r4:32]
; CHECK-NEXT:    add r4, sp, #28
; CHECK-NEXT:    vld1.32 {d16[1]}, [r4:32]
; CHECK-NEXT:    add r4, sp, #60
; CHECK-NEXT:    vld1.32 {d19[1]}, [r4:32]
; CHECK-NEXT:    add r4, sp, #52
; CHECK-NEXT:    vld1.32 {d18[1]}, [r4:32]
; CHECK-NEXT:    add r4, r12, #4
; CHECK-NEXT:    vcgt.u32 q8, q9, q8
; CHECK-NEXT:    vmovn.i32 d19, q10
; CHECK-NEXT:    vmov.u8 lr, d23[3]
; CHECK-NEXT:    vldr d20, .LCPI23_0
; CHECK-NEXT:    vmovn.i32 d18, q8
; CHECK-NEXT:    vmovn.i16 d22, q9
; CHECK-NEXT:    vmov.i8 q9, #0x7
; CHECK-NEXT:    vneg.s8 q9, q9
; CHECK-NEXT:    vmov.8 d17[0], lr
; CHECK-NEXT:    vtbl.8 d16, {d22, d23}, d20
; CHECK-NEXT:    vld1.8 {d17[1]}, [r4]
; CHECK-NEXT:    add r4, sp, #8
; CHECK-NEXT:    vshl.i8 q8, q8, #7
; CHECK-NEXT:    vld1.64 {d20, d21}, [r4]
; CHECK-NEXT:    vshl.s8 q8, q8, q9
; CHECK-NEXT:    vmov d19, r2, r3
; CHECK-NEXT:    vmov d18, r0, r1
; CHECK-NEXT:    vbsl q8, q9, q10
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d17
; CHECK-NEXT:    pop {r4, lr}
; CHECK-NEXT:    mov pc, lr
; CHECK-NEXT:    .p2align 3
; CHECK-NEXT:  @ %bb.1:
; CHECK-NEXT:  .LCPI23_0:
; CHECK-NEXT:    .byte 0 @ 0x0
; CHECK-NEXT:    .byte 1 @ 0x1
; CHECK-NEXT:    .byte 2 @ 0x2
; CHECK-NEXT:    .byte 3 @ 0x3
; CHECK-NEXT:    .byte 4 @ 0x4
; CHECK-NEXT:    .byte 8 @ 0x8
; CHECK-NEXT:    .byte 9 @ 0x9
; CHECK-NEXT:    .byte 10 @ 0xa
                            <5 x i32> %cmp0, <5 x i32> %cmp1, <5 x i8> *%cmp2_ptr) {
  %cmp2_load = load <5 x i8>, <5 x i8> * %cmp2_ptr, align 4
  %cmp2 = trunc <5 x i8> %cmp2_load to <5 x i1>
  %c0 = icmp ult <5 x i32> %cmp0, %cmp1
  %c = shufflevector <5 x i1> %c0, <5 x i1> %cmp2, <10 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9>
  %rv = select <10 x i1> %c, <10 x i8> %tr0, <10 x i8> %tr1
  ret <10 x i8> %rv
}

%struct.uint8x8x2_t = type { [2 x <8 x i8>] }
define %struct.uint8x8x2_t @vuzp_extract_subvector(<16 x i8> %t) #0 {
; CHECK-LABEL: vuzp_extract_subvector:
; CHECK:       @ %bb.0:
; CHECK-NEXT:    vmov d17, r2, r3
; CHECK-NEXT:    vmov d16, r0, r1
; CHECK-NEXT:    vorr d18, d17, d17
; CHECK-NEXT:    vuzp.8 d16, d18
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    vmov r2, r3, d18
; CHECK-NEXT:    mov pc, lr

  %vuzp.i = shufflevector <16 x i8> %t, <16 x i8> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
  %vuzp1.i = shufflevector <16 x i8> %t, <16 x i8> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
  %.fca.0.0.insert = insertvalue %struct.uint8x8x2_t undef, <8 x i8> %vuzp.i, 0, 0
  %.fca.0.1.insert = insertvalue %struct.uint8x8x2_t %.fca.0.0.insert, <8 x i8> %vuzp1.i, 0, 1
  ret %struct.uint8x8x2_t %.fca.0.1.insert
}
