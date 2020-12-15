; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=thumbv8.1m.main-none-none-eabi -mattr=+mve -verify-machineinstrs %s -o - | FileCheck %s

define arm_aapcs_vfpcc <4 x i32> @vcmp_eqz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_eqz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i32 eq, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_nez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_nez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i32 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ne <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_sgtz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_sgtz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 gt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sgt <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_sgez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_sgez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 ge, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sge <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_sltz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_sltz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 lt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp slt <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_slez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_slez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 le, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sle <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_ugtz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_ugtz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i32 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ugt <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_ugez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_ugez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q1
; CHECK-NEXT:    bx lr
entry:
  %c = icmp uge <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_ultz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_ultz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ult <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_ulez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_ulez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.u32 cs, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ule <4 x i32> %src, zeroinitializer
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}


define arm_aapcs_vfpcc <8 x i16> @vcmp_eqz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_eqz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i16 eq, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_nez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_nez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i16 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ne <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_sgtz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_sgtz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 gt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sgt <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_sgez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_sgez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 ge, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sge <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_sltz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_sltz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 lt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp slt <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_slez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_slez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 le, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sle <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_ugtz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_ugtz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i16 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ugt <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_ugez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_ugez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q1
; CHECK-NEXT:    bx lr
entry:
  %c = icmp uge <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_ultz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_ultz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ult <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_ulez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_ulez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.u16 cs, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ule <8 x i16> %src, zeroinitializer
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}


define arm_aapcs_vfpcc <16 x i8> @vcmp_eqz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_eqz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i8 eq, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_nez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_nez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i8 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ne <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_sgtz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_sgtz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 gt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sgt <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_sgez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_sgez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 ge, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sge <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_sltz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_sltz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 lt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp slt <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_slez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_slez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 le, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sle <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_ugtz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_ugtz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i8 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ugt <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_ugez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_ugez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q1
; CHECK-NEXT:    bx lr
entry:
  %c = icmp uge <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_ultz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_ultz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ult <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_ulez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_ulez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.u8 cs, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ule <16 x i8> %src, zeroinitializer
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}


define arm_aapcs_vfpcc <2 x i64> @vcmp_eqz_v2i64(<2 x i64> %src, <2 x i64> %a, <2 x i64> %b) {
; CHECK-LABEL: vcmp_eqz_v2i64:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov r0, s1
; CHECK-NEXT:    vmov r1, s0
; CHECK-NEXT:    vmov r2, s2
; CHECK-NEXT:    orrs r0, r1
; CHECK-NEXT:    vmov r1, s3
; CHECK-NEXT:    cset r0, eq
; CHECK-NEXT:    tst.w r0, #1
; CHECK-NEXT:    csetm r0, ne
; CHECK-NEXT:    orrs r1, r2
; CHECK-NEXT:    cset r1, eq
; CHECK-NEXT:    tst.w r1, #1
; CHECK-NEXT:    csetm r1, ne
; CHECK-NEXT:    vmov q0[2], q0[0], r1, r0
; CHECK-NEXT:    vmov q0[3], q0[1], r1, r0
; CHECK-NEXT:    vbic q2, q2, q0
; CHECK-NEXT:    vand q0, q1, q0
; CHECK-NEXT:    vorr q0, q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <2 x i64> %src, zeroinitializer
  %s = select <2 x i1> %c, <2 x i64> %a, <2 x i64> %b
  ret <2 x i64> %s
}

define arm_aapcs_vfpcc <2 x i32> @vcmp_eqz_v2i32(<2 x i64> %src, <2 x i32> %a, <2 x i32> %b) {
; CHECK-LABEL: vcmp_eqz_v2i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov r0, s1
; CHECK-NEXT:    vmov r1, s0
; CHECK-NEXT:    vmov r2, s2
; CHECK-NEXT:    orrs r0, r1
; CHECK-NEXT:    vmov r1, s3
; CHECK-NEXT:    cset r0, eq
; CHECK-NEXT:    tst.w r0, #1
; CHECK-NEXT:    csetm r0, ne
; CHECK-NEXT:    orrs r1, r2
; CHECK-NEXT:    cset r1, eq
; CHECK-NEXT:    tst.w r1, #1
; CHECK-NEXT:    csetm r1, ne
; CHECK-NEXT:    vmov q0[2], q0[0], r1, r0
; CHECK-NEXT:    vmov q0[3], q0[1], r1, r0
; CHECK-NEXT:    vbic q2, q2, q0
; CHECK-NEXT:    vand q0, q1, q0
; CHECK-NEXT:    vorr q0, q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <2 x i64> %src, zeroinitializer
  %s = select <2 x i1> %c, <2 x i32> %a, <2 x i32> %b
  ret <2 x i32> %s
}


; Reversed

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_eqz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_eqz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i32 eq, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_nez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_nez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i32 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ne <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_sgtz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_sgtz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 lt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sgt <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_sgez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_sgez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 le, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sge <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_sltz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_sltz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 gt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp slt <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_slez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_slez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s32 ge, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sle <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_ugtz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_ugtz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ugt <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_ugez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_ugez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.u32 cs, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp uge <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_ultz_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_ultz_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i32 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ult <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}

define arm_aapcs_vfpcc <4 x i32> @vcmp_r_ulez_v4i32(<4 x i32> %src, <4 x i32> %a, <4 x i32> %b) {
; CHECK-LABEL: vcmp_r_ulez_v4i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q1
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ule <4 x i32> zeroinitializer, %src
  %s = select <4 x i1> %c, <4 x i32> %a, <4 x i32> %b
  ret <4 x i32> %s
}


define arm_aapcs_vfpcc <8 x i16> @vcmp_r_eqz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_eqz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i16 eq, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_nez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_nez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i16 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ne <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_sgtz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_sgtz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 lt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sgt <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_sgez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_sgez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 le, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sge <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_sltz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_sltz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 gt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp slt <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_slez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_slez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s16 ge, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sle <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_ugtz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_ugtz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ugt <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_ugez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_ugez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.u16 cs, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp uge <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_ultz_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_ultz_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i16 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ult <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}

define arm_aapcs_vfpcc <8 x i16> @vcmp_r_ulez_v8i16(<8 x i16> %src, <8 x i16> %a, <8 x i16> %b) {
; CHECK-LABEL: vcmp_r_ulez_v8i16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q1
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ule <8 x i16> zeroinitializer, %src
  %s = select <8 x i1> %c, <8 x i16> %a, <8 x i16> %b
  ret <8 x i16> %s
}


define arm_aapcs_vfpcc <16 x i8> @vcmp_r_eqz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_eqz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i8 eq, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_nez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_nez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i8 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ne <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_sgtz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_sgtz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 lt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sgt <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_sgez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_sgez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 le, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sge <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_sltz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_sltz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 gt, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp slt <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_slez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_slez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.s8 ge, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp sle <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_ugtz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_ugtz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ugt <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_ugez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_ugez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.u8 cs, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp uge <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_ultz_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_ultz_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vcmp.i8 ne, q0, zr
; CHECK-NEXT:    vpsel q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ult <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}

define arm_aapcs_vfpcc <16 x i8> @vcmp_r_ulez_v16i8(<16 x i8> %src, <16 x i8> %a, <16 x i8> %b) {
; CHECK-LABEL: vcmp_r_ulez_v16i8:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov q0, q1
; CHECK-NEXT:    bx lr
entry:
  %c = icmp ule <16 x i8> zeroinitializer, %src
  %s = select <16 x i1> %c, <16 x i8> %a, <16 x i8> %b
  ret <16 x i8> %s
}


define arm_aapcs_vfpcc <2 x i64> @vcmp_r_eqz_v2i64(<2 x i64> %src, <2 x i64> %a, <2 x i64> %b) {
; CHECK-LABEL: vcmp_r_eqz_v2i64:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov r0, s1
; CHECK-NEXT:    vmov r1, s0
; CHECK-NEXT:    vmov r2, s2
; CHECK-NEXT:    orrs r0, r1
; CHECK-NEXT:    vmov r1, s3
; CHECK-NEXT:    cset r0, eq
; CHECK-NEXT:    tst.w r0, #1
; CHECK-NEXT:    csetm r0, ne
; CHECK-NEXT:    orrs r1, r2
; CHECK-NEXT:    cset r1, eq
; CHECK-NEXT:    tst.w r1, #1
; CHECK-NEXT:    csetm r1, ne
; CHECK-NEXT:    vmov q0[2], q0[0], r1, r0
; CHECK-NEXT:    vmov q0[3], q0[1], r1, r0
; CHECK-NEXT:    vbic q2, q2, q0
; CHECK-NEXT:    vand q0, q1, q0
; CHECK-NEXT:    vorr q0, q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <2 x i64> zeroinitializer, %src
  %s = select <2 x i1> %c, <2 x i64> %a, <2 x i64> %b
  ret <2 x i64> %s
}

define arm_aapcs_vfpcc <2 x i32> @vcmp_r_eqz_v2i32(<2 x i64> %src, <2 x i32> %a, <2 x i32> %b) {
; CHECK-LABEL: vcmp_r_eqz_v2i32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vmov r0, s1
; CHECK-NEXT:    vmov r1, s0
; CHECK-NEXT:    vmov r2, s2
; CHECK-NEXT:    orrs r0, r1
; CHECK-NEXT:    vmov r1, s3
; CHECK-NEXT:    cset r0, eq
; CHECK-NEXT:    tst.w r0, #1
; CHECK-NEXT:    csetm r0, ne
; CHECK-NEXT:    orrs r1, r2
; CHECK-NEXT:    cset r1, eq
; CHECK-NEXT:    tst.w r1, #1
; CHECK-NEXT:    csetm r1, ne
; CHECK-NEXT:    vmov q0[2], q0[0], r1, r0
; CHECK-NEXT:    vmov q0[3], q0[1], r1, r0
; CHECK-NEXT:    vbic q2, q2, q0
; CHECK-NEXT:    vand q0, q1, q0
; CHECK-NEXT:    vorr q0, q0, q2
; CHECK-NEXT:    bx lr
entry:
  %c = icmp eq <2 x i64> %src, zeroinitializer
  %s = select <2 x i1> %c, <2 x i32> %a, <2 x i32> %b
  ret <2 x i32> %s
}
