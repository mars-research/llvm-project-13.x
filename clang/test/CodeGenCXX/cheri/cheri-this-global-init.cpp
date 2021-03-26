// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --function-signature
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple aarch64-none-elf -target-feature +c64 -target-abi purecap -mllvm -cheri-cap-table-abi=pcrel -emit-llvm -o - %s | FileCheck %s

class time_point {
  public:
// CHECK-LABEL: define {{[^@]+}}@_ZN10time_pointC1Ev
// CHECK-SAME: (%class.time_point addrspace(200)* dereferenceable(1) [[THIS:%.*]]) unnamed_addr addrspace(200) [[ATTR1:#.*]] comdat align 2 {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[THIS_ADDR:%.*]] = alloca [[CLASS_TIME_POINT:%.*]] addrspace(200)*, align 16, addrspace(200)
// CHECK-NEXT:    store [[CLASS_TIME_POINT]] addrspace(200)* [[THIS]], [[CLASS_TIME_POINT]] addrspace(200)* addrspace(200)* [[THIS_ADDR]], align 16
// CHECK-NEXT:    [[THIS1:%.*]] = load [[CLASS_TIME_POINT]] addrspace(200)*, [[CLASS_TIME_POINT]] addrspace(200)* addrspace(200)* [[THIS_ADDR]], align 16
// CHECK-NEXT:    call void @_ZN10time_pointC2Ev(%class.time_point addrspace(200)* dereferenceable(1) [[THIS1]])
// CHECK-NEXT:    ret void
//
    time_point() { }
};

time_point t1;
