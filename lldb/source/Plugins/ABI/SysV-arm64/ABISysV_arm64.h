//===-- ABISysV_arm64.h ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ABISysV_arm64_h_
#define liblldb_ABISysV_arm64_h_

#include "lldb/Target/ABI.h"
#include "lldb/lldb-private.h"

class ABISysV_arm64 : public lldb_private::ABI {
public:
  ~ABISysV_arm64() override = default;

  size_t GetRedZoneSize() const override;

  bool PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                          lldb::addr_t functionAddress,
                          lldb::addr_t returnAddress,
                          llvm::ArrayRef<lldb::addr_t> args) const override;

  bool GetArgumentValues(lldb_private::Thread &thread,
                         lldb_private::ValueList &values) const override;

  lldb_private::Status
  SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                       lldb::ValueObjectSP &new_value) override;

  bool
  CreateFunctionEntryUnwindPlan(lldb_private::UnwindPlan &unwind_plan) override;

  bool CreateDefaultUnwindPlan(lldb_private::UnwindPlan &unwind_plan) override;

  bool RegisterIsVolatile(lldb_private::RegisterContext &reg_ctx,
                          const lldb_private::RegisterInfo *reg_info,
                          FrameState state,
                          const lldb_private::UnwindPlan *unwind_plan) override;

  bool GetFallbackRegisterLocation(
      lldb_private::RegisterContext &reg_ctx,
      const lldb_private::RegisterInfo *reg_info, FrameState frame_state,
      const lldb_private::UnwindPlan *unwind_plan,
      lldb::RegisterKind &unwind_registerkind,
      lldb_private::UnwindPlan::Row::RegisterLocation &unwind_regloc) override;

  uint32_t GetExtendedRegisterForUnwind(lldb_private::RegisterContext &reg_ctx,
                                        uint32_t lldb_regnum) const override;

  uint32_t
  GetPrimordialRegisterForUnwind(lldb_private::RegisterContext &reg_ctx,
                                 uint32_t lldb_regnum,
                                 uint32_t byte_size) const override;

  uint32_t GetReturnRegisterForUnwind(lldb_private::RegisterContext &reg_ctx,
                                      uint32_t pc_lldb_regnum,
                                      uint32_t ra_lldb_regnum) const override;

  bool GetFrameState(lldb_private::RegisterContext &reg_ctx,
                     FrameState &state) const override;

  bool GetCalleeRegisterToSearch(lldb_private::RegisterContext &reg_ctx,
                                 uint32_t lldb_regnum,
                                 FrameState caller_frame_state,
                                 uint32_t &search_lldb_regnum) const override;

  // The arm64 ABI requires that stack frames be 16 byte aligned.
  // When there is a trap handler on the stack, e.g. _sigtramp in userland
  // code, we've seen that the stack pointer is often not aligned properly
  // before the handler is invoked.  This means that lldb will stop the unwind
  // early -- before the function which caused the trap.
  //
  // To work around this, we relax that alignment to be just word-size
  // (8-bytes).
  // Whitelisting the trap handlers for user space would be easy (_sigtramp) but
  // in other environments there can be a large number of different functions
  // involved in async traps.
  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    // Make sure the stack call frame addresses are are 8 byte aligned
    if (cfa & (8ull - 1ull))
      return false; // Not 8 byte aligned
    if (cfa == 0)
      return false; // Zero is not a valid stack address
    return true;
  }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    // Bit zero distinguishes A64 (0) and C64 (1). Enforce that the address is
    // 4-byte aligned without taking this bit into account.
    if ((pc & 2) != 0)
      return false; // Not 4 byte aligned

    // Anything else if fair game..
    return true;
  }

  lldb::addr_t FixCodeAddress(lldb::addr_t pc) override {
    // Clear bit zero in the address as it is used to signify use of the C64
    // instruction set.
    return pc & ~(lldb::addr_t)1;
  }

  const lldb_private::RegisterInfo *
  GetRegisterInfoArray(uint32_t &count) override;

  bool GetPointerReturnRegister(const char *&name) override;

  // Static Functions

  static void Initialize();

  static void Terminate();

  static lldb::ABISP CreateInstance(lldb::ProcessSP process_sp, const lldb_private::ArchSpec &arch);

  static lldb_private::ConstString GetPluginNameStatic();

  // PluginInterface protocol

  lldb_private::ConstString GetPluginName() override;

  uint32_t GetPluginVersion() override;

protected:
  lldb::ValueObjectSP
  GetReturnValueObjectImpl(lldb_private::Thread &thread,
                           lldb_private::CompilerType &ast_type) const override;

  lldb_private::CompilerType
  GetSigInfoCompilerType(const lldb_private::Target &target,
                         lldb_private::TypeSystemClang &ast_ctx,
                         llvm::StringRef type_name) const override;

private:
  ABISysV_arm64(lldb::ProcessSP process_sp,
                std::unique_ptr<llvm::MCRegisterInfo> info_up)
      : lldb_private::ABI(std::move(process_sp), std::move(info_up)) {
    // Call CreateInstance instead.
  }
};

#endif // liblldb_ABISysV_arm64_h_
