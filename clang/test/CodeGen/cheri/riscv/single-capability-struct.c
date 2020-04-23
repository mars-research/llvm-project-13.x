// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py

// RUN: %riscv32_cheri_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=CHECK-ILP32
// RUN: %riscv64_cheri_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=CHECK-LP64
// RUN: %riscv32_cheri_purecap_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=CHECK-IL32PC64
// RUN: %riscv64_cheri_purecap_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=CHECK-L64PC128

struct boxed {
	char * __capability x;
};

// CHECK-ILP32-LABEL: @callee(
// CHECK-ILP32-NEXT:  entry:
// CHECK-ILP32-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 8
// CHECK-ILP32-NEXT:    [[X:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], i32 0, i32 0
// CHECK-ILP32-NEXT:    store i8 addrspace(200)* null, i8 addrspace(200)** [[X]], align 8
// CHECK-ILP32-NEXT:    [[TMP0:%.*]] = load [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], align 8
// CHECK-ILP32-NEXT:    ret [[STRUCT_BOXED]] %0
//
// CHECK-LP64-LABEL: @callee(
// CHECK-LP64-NEXT:  entry:
// CHECK-LP64-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 16
// CHECK-LP64-NEXT:    [[X:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], i32 0, i32 0
// CHECK-LP64-NEXT:    store i8 addrspace(200)* null, i8 addrspace(200)** [[X]], align 16
// CHECK-LP64-NEXT:    [[TMP0:%.*]] = load [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], align 16
// CHECK-LP64-NEXT:    ret [[STRUCT_BOXED]] %0
//
// CHECK-IL32PC64-LABEL: @callee(
// CHECK-IL32PC64-NEXT:  entry:
// CHECK-IL32PC64-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 8, addrspace(200)
// CHECK-IL32PC64-NEXT:    [[X:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], i32 0, i32 0
// CHECK-IL32PC64-NEXT:    store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* [[X]], align 8
// CHECK-IL32PC64-NEXT:    [[TMP0:%.*]] = load [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], align 8
// CHECK-IL32PC64-NEXT:    ret [[STRUCT_BOXED]] %0
//
// CHECK-L64PC128-LABEL: @callee(
// CHECK-L64PC128-NEXT:  entry:
// CHECK-L64PC128-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 16, addrspace(200)
// CHECK-L64PC128-NEXT:    [[X:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], i32 0, i32 0
// CHECK-L64PC128-NEXT:    store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* [[X]], align 16
// CHECK-L64PC128-NEXT:    [[TMP0:%.*]] = load [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], align 16
// CHECK-L64PC128-NEXT:    ret [[STRUCT_BOXED]] %0
//
struct boxed callee(void) {
	return (struct boxed){ 0 };
}

// CHECK-ILP32-LABEL: @caller(
// CHECK-ILP32-NEXT:  entry:
// CHECK-ILP32-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 8
// CHECK-ILP32-NEXT:    [[CALL:%.*]] = call [[STRUCT_BOXED]] @callee()
// CHECK-ILP32-NEXT:    [[TMP0:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], i32 0, i32 0
// CHECK-ILP32-NEXT:    [[TMP1:%.*]] = extractvalue [[STRUCT_BOXED]] %call, 0
// CHECK-ILP32-NEXT:    store i8 addrspace(200)* [[TMP1]], i8 addrspace(200)** [[TMP0]], align 8
// CHECK-ILP32-NEXT:    [[TMP2:%.*]] = load [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], align 8
// CHECK-ILP32-NEXT:    ret [[STRUCT_BOXED]] %2
//
// CHECK-LP64-LABEL: @caller(
// CHECK-LP64-NEXT:  entry:
// CHECK-LP64-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 16
// CHECK-LP64-NEXT:    [[CALL:%.*]] = call [[STRUCT_BOXED]] @callee()
// CHECK-LP64-NEXT:    [[TMP0:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], i32 0, i32 0
// CHECK-LP64-NEXT:    [[TMP1:%.*]] = extractvalue [[STRUCT_BOXED]] %call, 0
// CHECK-LP64-NEXT:    store i8 addrspace(200)* [[TMP1]], i8 addrspace(200)** [[TMP0]], align 16
// CHECK-LP64-NEXT:    [[TMP2:%.*]] = load [[STRUCT_BOXED]], %struct.boxed* [[RETVAL]], align 16
// CHECK-LP64-NEXT:    ret [[STRUCT_BOXED]] %2
//
// CHECK-IL32PC64-LABEL: @caller(
// CHECK-IL32PC64-NEXT:  entry:
// CHECK-IL32PC64-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 8, addrspace(200)
// CHECK-IL32PC64-NEXT:    [[CALL:%.*]] = call [[STRUCT_BOXED]] @callee()
// CHECK-IL32PC64-NEXT:    [[TMP0:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], i32 0, i32 0
// CHECK-IL32PC64-NEXT:    [[TMP1:%.*]] = extractvalue [[STRUCT_BOXED]] %call, 0
// CHECK-IL32PC64-NEXT:    store i8 addrspace(200)* [[TMP1]], i8 addrspace(200)* addrspace(200)* [[TMP0]], align 8
// CHECK-IL32PC64-NEXT:    [[TMP2:%.*]] = load [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], align 8
// CHECK-IL32PC64-NEXT:    ret [[STRUCT_BOXED]] %2
//
// CHECK-L64PC128-LABEL: @caller(
// CHECK-L64PC128-NEXT:  entry:
// CHECK-L64PC128-NEXT:    [[RETVAL:%.*]] = alloca [[STRUCT_BOXED:%.*]], align 16, addrspace(200)
// CHECK-L64PC128-NEXT:    [[CALL:%.*]] = call [[STRUCT_BOXED]] @callee()
// CHECK-L64PC128-NEXT:    [[TMP0:%.*]] = getelementptr inbounds [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], i32 0, i32 0
// CHECK-L64PC128-NEXT:    [[TMP1:%.*]] = extractvalue [[STRUCT_BOXED]] %call, 0
// CHECK-L64PC128-NEXT:    store i8 addrspace(200)* [[TMP1]], i8 addrspace(200)* addrspace(200)* [[TMP0]], align 16
// CHECK-L64PC128-NEXT:    [[TMP2:%.*]] = load [[STRUCT_BOXED]], [[STRUCT_BOXED]] addrspace(200)* [[RETVAL]], align 16
// CHECK-L64PC128-NEXT:    ret [[STRUCT_BOXED]] %2
//
struct boxed caller(void) {
	return callee();
}
