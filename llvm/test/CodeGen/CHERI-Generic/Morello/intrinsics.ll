; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --function-signature --scrub-attributes --force-update
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/intrinsics.ll
; RUN: llc -mtriple=aarch64 --relocation-model=pic -target-abi purecap -mattr=+morello,+c64 %s -o - < %s | FileCheck %s --check-prefix=PURECAP
; RUN: llc -mtriple=aarch64 --relocation-model=pic -target-abi aapcs -mattr=+morello,-c64 -o - < %s | FileCheck %s --check-prefix=HYBRID
; Check that the target-independent CHERI intrinsics are support for all architectures
; The grouping/ordering in this test is based on the RISC-V instruction listing
; in the CHERI ISA specification (Appendix C.1 in ISAv7).

; Capability-Inspection Instructions

declare i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)*)
declare i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)*)
declare i64 @llvm.cheri.cap.base.get.i64(i8 addrspace(200)*)
declare i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)*)
declare i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)*)
declare i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*)
declare i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*)
declare i64 @llvm.cheri.cap.flags.get.i64(i8 addrspace(200)*)
declare i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*)

define i64 @perms_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: perms_get:
; PURECAP:       .Lfunc_begin0:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gcperm x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: perms_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gcperm x0, c0
; HYBRID-NEXT:    ret
  %perms = call i64 @llvm.cheri.cap.perms.get.i64(i8 addrspace(200)* %cap)
  ret i64 %perms
}

define i64 @type_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: type_get:
; PURECAP:       .Lfunc_begin1:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gctype x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: type_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gctype x0, c0
; HYBRID-NEXT:    ret
  %type = call i64 @llvm.cheri.cap.type.get.i64(i8 addrspace(200)* %cap)
  ret i64 %type
}

define i64 @base_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: base_get:
; PURECAP:       .Lfunc_begin2:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gcbase x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: base_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gcbase x0, c0
; HYBRID-NEXT:    ret
  %base = call i64 @llvm.cheri.cap.base.get.i64(i8 addrspace(200)* %cap)
  ret i64 %base
}

define i64 @length_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: length_get:
; PURECAP:       .Lfunc_begin3:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gclen x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: length_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gclen x0, c0
; HYBRID-NEXT:    ret
  %length = call i64 @llvm.cheri.cap.length.get.i64(i8 addrspace(200)* %cap)
  ret i64 %length
}

define i64 @tag_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: tag_get:
; PURECAP:       .Lfunc_begin4:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gctag x8, c0
; PURECAP-NEXT:    cmp x8, #0 // =0
; PURECAP-NEXT:    cset w0, ne
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: tag_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gctag x8, c0
; HYBRID-NEXT:    cmp x8, #0 // =0
; HYBRID-NEXT:    cset w0, ne
; HYBRID-NEXT:    ret
  %tag = call i1 @llvm.cheri.cap.tag.get(i8 addrspace(200)* %cap)
  %tag.zext = zext i1 %tag to i64
  ret i64 %tag.zext
}

define i64 @sealed_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: sealed_get:
; PURECAP:       .Lfunc_begin5:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gcseal x8, c0
; PURECAP-NEXT:    cmp x8, #0 // =0
; PURECAP-NEXT:    cset w0, ne
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: sealed_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gcseal x8, c0
; HYBRID-NEXT:    cmp x8, #0 // =0
; HYBRID-NEXT:    cset w0, ne
; HYBRID-NEXT:    ret
  %sealed = call i1 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* %cap)
  %sealed.zext = zext i1 %sealed to i64
  ret i64 %sealed.zext
}

define i64 @offset_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: offset_get:
; PURECAP:       .Lfunc_begin6:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gcoff x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: offset_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gcoff x0, c0
; HYBRID-NEXT:    ret
  %offset = call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %cap)
  ret i64 %offset
}

define i64 @flags_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: flags_get:
; PURECAP:       .Lfunc_begin7:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    mov x0, xzr
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: flags_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    mov x0, xzr
; HYBRID-NEXT:    ret
  %flags = call i64 @llvm.cheri.cap.flags.get.i64(i8 addrspace(200)* %cap)
  ret i64 %flags
}

define i64 @address_get(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: address_get:
; PURECAP:       .Lfunc_begin8:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gcvalue x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: address_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gcvalue x0, c0
; HYBRID-NEXT:    ret
  %address = call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %cap)
  ret i64 %address
}

; Capability-Modification Instructions

declare i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.flags.set.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.tag.clear(i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.build(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.type.copy(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.conditional.seal(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.seal.entry(i8 addrspace(200)*)

define i8 addrspace(200)* @seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: seal:
; PURECAP:       .Lfunc_begin9:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    seal c0, c0, c1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: seal:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    seal c0, c0, c1
; HYBRID-NEXT:    ret
  %sealed = call i8 addrspace(200)* @llvm.cheri.cap.seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %sealed
}

define i8 addrspace(200)* @unseal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: unseal:
; PURECAP:       .Lfunc_begin10:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    unseal c0, c0, c1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: unseal:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    unseal c0, c0, c1
; HYBRID-NEXT:    ret
  %unsealed = call i8 addrspace(200)* @llvm.cheri.cap.unseal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %unsealed
}

define i8 addrspace(200)* @perms_and(i8 addrspace(200)* %cap, i64 %perms) nounwind {
; PURECAP-LABEL: perms_and:
; PURECAP:       .Lfunc_begin11:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    mvn x8, x1
; PURECAP-NEXT:    clrperm c0, c0, x8
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: perms_and:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    mvn x8, x1
; HYBRID-NEXT:    clrperm c0, c0, x8
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64(i8 addrspace(200)* %cap, i64 %perms)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @flags_set(i8 addrspace(200)* %cap, i64 %flags) nounwind {
; PURECAP-LABEL: flags_set:
; PURECAP:       .Lfunc_begin12:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: flags_set:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.flags.set.i64(i8 addrspace(200)* %cap, i64 %flags)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @offset_set(i8 addrspace(200)* %cap, i64 %offset) nounwind {
; PURECAP-LABEL: offset_set:
; PURECAP:       .Lfunc_begin13:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    scoff c0, c0, x1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: offset_set:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    scoff c0, c0, x1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %cap, i64 %offset)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @address_set(i8 addrspace(200)* %cap, i64 %address) nounwind {
; PURECAP-LABEL: address_set:
; PURECAP:       .Lfunc_begin14:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    scvalue c0, c0, x1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: address_set:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    scvalue c0, c0, x1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %cap, i64 %address)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @bounds_set(i8 addrspace(200)* %cap, i64 %bounds) nounwind {
; PURECAP-LABEL: bounds_set:
; PURECAP:       .Lfunc_begin15:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    scbnds c0, c0, x1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: bounds_set:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    scbnds c0, c0, x1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %cap, i64 %bounds)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @bounds_set_exact(i8 addrspace(200)* %cap, i64 %bounds) nounwind {
; PURECAP-LABEL: bounds_set_exact:
; PURECAP:       .Lfunc_begin16:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    scbndse c0, c0, x1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: bounds_set_exact:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    scbndse c0, c0, x1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.i64(i8 addrspace(200)* %cap, i64 %bounds)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @bounds_set_immediate(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: bounds_set_immediate:
; PURECAP:       .Lfunc_begin17:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    scbnds c0, c0, #42 // =42
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: bounds_set_immediate:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    scbnds c0, c0, #42 // =42
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* %cap, i64 42)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @tag_clear(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: tag_clear:
; PURECAP:       .Lfunc_begin18:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    clrtag c0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: tag_clear:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    clrtag c0, c0
; HYBRID-NEXT:    ret
  %untagged = call i8 addrspace(200)* @llvm.cheri.cap.tag.clear(i8 addrspace(200)* %cap)
  ret i8 addrspace(200)* %untagged
}

define i8 addrspace(200)* @build(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: build:
; PURECAP:       .Lfunc_begin19:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    build c0, c1, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: build:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    build c0, c1, c0
; HYBRID-NEXT:    ret
  %built = call i8 addrspace(200)* @llvm.cheri.cap.build(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %built
}

define i8 addrspace(200)* @type_copy(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: type_copy:
; PURECAP:       .Lfunc_begin20:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    cpytype c0, c0, c1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: type_copy:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    cpytype c0, c0, c1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.type.copy(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @conditional_seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: conditional_seal:
; PURECAP:       .Lfunc_begin21:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    cseal c0, c0, c1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: conditional_seal:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    cseal c0, c0, c1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.conditional.seal(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @seal_entry(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: seal_entry:
; PURECAP:       .Lfunc_begin22:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    seal c0, c0, rb
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: seal_entry:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    seal c0, c0, rb
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.seal.entry(i8 addrspace(200)* %cap)
  ret i8 addrspace(200)* %newcap
}

; Pointer-Arithmetic Instructions

declare i64 @llvm.cheri.cap.to.pointer(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)*, i64)
declare i64 @llvm.cheri.cap.diff(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.ddc.get()
declare i8 addrspace(200)* @llvm.cheri.pcc.get()

define i64 @to_pointer(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: to_pointer:
; PURECAP:       .Lfunc_begin23:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    cvt x0, c1, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: to_pointer:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    cvt x0, c1, c0
; HYBRID-NEXT:    ret
  %ptr = call i64 @llvm.cheri.cap.to.pointer(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i64 %ptr
}

define i64 @to_pointer_ddc_relative(i8 addrspace(200)* %cap) nounwind {
; PURECAP-LABEL: to_pointer_ddc_relative:
; PURECAP:       .Lfunc_begin24:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    cvtd x0, c0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: to_pointer_ddc_relative:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    cvtd x0, c0
; HYBRID-NEXT:    ret
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %ptr = call i64 @llvm.cheri.cap.to.pointer(i8 addrspace(200)* %ddc, i8 addrspace(200)* %cap)
  ret i64 %ptr
}

define i8 addrspace(200)* @from_pointer(i8 addrspace(200)* %cap, i64 %ptr) nounwind {
; PURECAP-LABEL: from_pointer:
; PURECAP:       .Lfunc_begin25:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    cvtz c0, c0, x1
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: from_pointer:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    cvtz c0, c0, x1
; HYBRID-NEXT:    ret
  %newcap = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* %cap, i64 %ptr)
  ret i8 addrspace(200)* %newcap
}

define i8 addrspace(200)* @from_ddc(i64 %ptr) nounwind {
; PURECAP-LABEL: from_ddc:
; PURECAP:       .Lfunc_begin26:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    cvtdz c0, x0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: from_ddc:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    cvtdz c0, x0
; HYBRID-NEXT:    ret
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %cap = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* %ddc, i64 %ptr)
  ret i8 addrspace(200)* %cap
}

define i64 @diff(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: diff:
; PURECAP:       .Lfunc_begin27:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    gcvalue x8, c1
; PURECAP-NEXT:    gcvalue x9, c0
; PURECAP-NEXT:    sub x0, x9, x8
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: diff:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    gcvalue x8, c1
; HYBRID-NEXT:    gcvalue x9, c0
; HYBRID-NEXT:    sub x0, x9, x8
; HYBRID-NEXT:    ret
  %diff = call i64 @llvm.cheri.cap.diff(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  ret i64 %diff
}

define i8 addrspace(200)* @ddc_get() nounwind {
; PURECAP-LABEL: ddc_get:
; PURECAP:       .Lfunc_begin28:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    mrs c0, DDC
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: ddc_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    mrs c0, DDC
; HYBRID-NEXT:    ret
  %cap = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  ret i8 addrspace(200)* %cap
}

define i8 addrspace(200)* @pcc_get() nounwind {
; PURECAP-LABEL: pcc_get:
; PURECAP:       .Lfunc_begin29:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    adr c0, #0
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: pcc_get:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    adr x8, #0
; HYBRID-NEXT:    cvtp c0, x8
; HYBRID-NEXT:    ret
  %cap = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  ret i8 addrspace(200)* %cap
}

; Assertion Instructions

declare i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)

define i64 @subset_test(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: subset_test:
; PURECAP:       .Lfunc_begin30:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    chkss c0, c1
; PURECAP-NEXT:    cset w0, mi
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: subset_test:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    chkss c0, c1
; HYBRID-NEXT:    cset w0, mi
; HYBRID-NEXT:    ret
  %subset = call i1 @llvm.cheri.cap.subset.test(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  %subset.zext = zext i1 %subset to i64
  ret i64 %subset.zext
}

declare i1 @llvm.cheri.cap.equal.exact(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)

define i64 @equal_exact(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2) nounwind {
; PURECAP-LABEL: equal_exact:
; PURECAP:       .Lfunc_begin31:
; PURECAP-NEXT:  // %bb.0:
; PURECAP-NEXT:    chkeq c0, c1
; PURECAP-NEXT:    cset w0, eq
; PURECAP-NEXT:    ret c30
;
; HYBRID-LABEL: equal_exact:
; HYBRID:       // %bb.0:
; HYBRID-NEXT:    chkeq c0, c1
; HYBRID-NEXT:    cset w0, eq
; HYBRID-NEXT:    ret
  %eqex = call i1 @llvm.cheri.cap.equal.exact(i8 addrspace(200)* %cap1, i8 addrspace(200)* %cap2)
  %eqex.zext = zext i1 %eqex to i64
  ret i64 %eqex.zext
}
