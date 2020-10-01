//===- AArch64FrameLowering.cpp - AArch64 Frame Lowering -------*- C++ -*-====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the AArch64 implementation of TargetFrameLowering class.
//
// On AArch64, stack frames are structured as follows:
//
// The stack grows downward.
//
// All of the individual frame areas on the frame below are optional, i.e. it's
// possible to create a function so that the particular area isn't present
// in the frame.
//
// At function entry, the "frame" looks as follows:
//
// |                                   | Higher address
// |-----------------------------------|
// |                                   |
// | arguments passed on the stack     |
// |                                   |
// |-----------------------------------| <- sp
// |                                   | Lower address
//
//
// After the prologue has run, the frame has the following general structure.
// Note that this doesn't depict the case where a red-zone is used. Also,
// technically the last frame area (VLAs) doesn't get created until in the
// main function body, after the prologue is run. However, it's depicted here
// for completeness.
//
// |                                   | Higher address
// |-----------------------------------|
// |                                   |
// | arguments passed on the stack     |
// |                                   |
// |-----------------------------------|
// |                                   |
// | (Win64 only) varargs from reg     |
// |                                   |
// |-----------------------------------|
// |                                   |
// | callee-saved gpr registers        | <--.
// |                                   |    | On Darwin platforms these
// |- - - - - - - - - - - - - - - - - -|    | callee saves are swapped,
// |                                   |    | (frame record first)
// | prev_fp, prev_lr                  | <--'
// | (a.k.a. "frame record")           |
// |-----------------------------------| <- fp(=x29)
// |                                   |
// | callee-saved fp/simd/SVE regs     |
// |                                   |
// |-----------------------------------|
// |                                   |
// |        SVE stack objects          |
// |                                   |
// |-----------------------------------|
// |.empty.space.to.make.part.below....|
// |.aligned.in.case.it.needs.more.than| (size of this area is unknown at
// |.the.standard.16-byte.alignment....|  compile time; if present)
// |-----------------------------------|
// |                                   |
// | local variables of fixed size     |
// | including spill slots             |
// |-----------------------------------| <- bp(not defined by ABI,
// |.variable-sized.local.variables....|       LLVM chooses X19)
// |.(VLAs)............................| (size of this area is unknown at
// |...................................|  compile time)
// |-----------------------------------| <- sp
// |                                   | Lower address
//
//
// To access the data in a frame, at-compile time, a constant offset must be
// computable from one of the pointers (fp, bp, sp) to access it. The size
// of the areas with a dotted background cannot be computed at compile-time
// if they are present, making it required to have all three of fp, bp and
// sp to be set up to be able to access all contents in the frame areas,
// assuming all of the frame areas are non-empty.
//
// For most functions, some of the frame areas are empty. For those functions,
// it may not be necessary to set up fp or bp:
// * A base pointer is definitely needed when there are both VLAs and local
//   variables with more-than-default alignment requirements.
// * A frame pointer is definitely needed when there are local variables with
//   more-than-default alignment requirements.
//
// For Darwin platforms the frame-record (fp, lr) is stored at the top of the
// callee-saved area, since the unwind encoding does not allow for encoding
// this dynamically and existing tools depend on this layout. For other
// platforms, the frame-record is stored at the bottom of the (gpr) callee-saved
// area to allow SVE stack objects (allocated directly below the callee-saves,
// if available) to be accessed directly from the framepointer.
// The SVE spill/fill instructions have VL-scaled addressing modes such
// as:
//    ldr z8, [fp, #-7 mul vl]
// For SVE the size of the vector length (VL) is not known at compile-time, so
// '#-7 mul vl' is an offset that can only be evaluated at runtime. With this
// layout, we don't need to add an unscaled offset to the framepointer before
// accessing the SVE object in the frame.
//
// In some cases when a base pointer is not strictly needed, it is generated
// anyway when offsets from the frame pointer to access local variables become
// so large that the offset can't be encoded in the immediate fields of loads
// or stores.
//
// FIXME: also explain the redzone concept.
// FIXME: also explain the concept of reserved call frames.
//
//===----------------------------------------------------------------------===//

#include "AArch64FrameLowering.h"
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64RegisterInfo.h"
#include "AArch64StackOffset.h"
#include "AArch64Subtarget.h"
#include "AArch64TargetMachine.h"
#include "MCTargetDesc/AArch64AddressingModes.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/WinEHFuncInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <cassert>
#include <cstdint>
#include <iterator>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "frame-info"

static cl::opt<bool> EnableRedZone("aarch64-redzone",
                                   cl::desc("enable use of redzone on AArch64"),
                                   cl::init(false), cl::Hidden);

static cl::opt<bool>
    ReverseCSRRestoreSeq("reverse-csr-restore-seq",
                         cl::desc("reverse the CSR restore sequence"),
                         cl::init(false), cl::Hidden);

static cl::opt<bool> StackTaggingMergeSetTag(
    "stack-tagging-merge-settag",
    cl::desc("merge settag instruction in function epilog"), cl::init(true),
    cl::Hidden);

STATISTIC(NumRedZoneFunctions, "Number of functions using red zone");
STATISTIC(CapabilitySpills, "Number of capability registers spilled");
STATISTIC(XRegsSpills, "Number of GPR64 registers spilled");


/// Returns the argument pop size.
static uint64_t getArgumentPopSize(MachineFunction &MF,
                                   MachineBasicBlock &MBB) {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  bool IsTailCallReturn = false;
  if (MBB.end() != MBBI) {
    unsigned RetOpcode = MBBI->getOpcode();
    IsTailCallReturn = RetOpcode == AArch64::TCRETURNdi ||
                       RetOpcode == AArch64::CTCRETURNr ||
                       RetOpcode == AArch64::TCRETURNri ||
                       RetOpcode == AArch64::TCRETURNriBTI;
  }
  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();

  uint64_t ArgumentPopSize = 0;
  if (IsTailCallReturn) {
    MachineOperand &StackAdjust = MBBI->getOperand(1);

    // For a tail-call in a callee-pops-arguments environment, some or all of
    // the stack may actually be in use for the call's arguments, this is
    // calculated during LowerCall and consumed here...
    ArgumentPopSize = StackAdjust.getImm();
  } else {
    // ... otherwise the amount to pop is *all* of the argument space,
    // conveniently stored in the MachineFunctionInfo by
    // LowerFormalArguments. This will, of course, be zero for the C calling
    // convention.
    ArgumentPopSize = AFI->getArgumentStackToRestore();
  }

  return ArgumentPopSize;
}

/// This is the biggest offset to the stack pointer we can encode in aarch64
/// instructions (without using a separate calculation and a temp register).
/// Note that the exception here are vector stores/loads which cannot encode any
/// displacements (see estimateRSStackSizeLimit(), isAArch64FrameOffsetLegal()).
static const unsigned DefaultSafeSPDisplacement = 255;

/// Look at each instruction that references stack frames and return the stack
/// size limit beyond which some of these instructions will require a scratch
/// register during their expansion later.
static unsigned estimateRSStackSizeLimit(MachineFunction &MF) {
  // FIXME: For now, just conservatively guestimate based on unscaled indexing
  // range. We'll end up allocating an unnecessary spill slot a lot, but
  // realistically that's not a big deal at this stage of the game.
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.isDebugInstr() || MI.isPseudo() ||
          MI.getOpcode() == AArch64::CapAddImm ||
          MI.getOpcode() == AArch64::ADDXri ||
          MI.getOpcode() == AArch64::ADDSXri)
        continue;

      for (const MachineOperand &MO : MI.operands()) {
        if (!MO.isFI())
          continue;

        StackOffset Offset;
        if (isAArch64FrameOffsetLegal(MI, Offset, nullptr, nullptr, nullptr) ==
            AArch64FrameOffsetCannotUpdate)
          return 0;
      }
    }
  }
  return DefaultSafeSPDisplacement;
}

TargetStackID::Value
AArch64FrameLowering::getStackIDForScalableVectors() const {
  return TargetStackID::SVEVector;
}

/// Returns the size of the fixed object area (allocated next to sp on entry)
/// On Win64 this may include a var args area and an UnwindHelp object for EH.
static unsigned getFixedObjectSize(const MachineFunction &MF,
                                   const AArch64FunctionInfo *AFI, bool IsWin64,
                                   bool IsFunclet) {
  if (!IsWin64 || IsFunclet) {
    // Only Win64 uses fixed objects, and then only for the function (not
    // funclets)
    return 0;
  } else {
    // Var args are stored here in the primary function.
    const unsigned VarArgsArea = AFI->getVarArgsGPRSize();
    // To support EH funclets we allocate an UnwindHelp object
    const unsigned UnwindHelpObject = (MF.hasEHFunclets() ? 8 : 0);
    return alignTo(VarArgsArea + UnwindHelpObject, 16);
  }
}

/// Returns the size of the entire SVE stackframe (calleesaves + spills).
static StackOffset getSVEStackSize(const MachineFunction &MF) {
  const AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  return {(int64_t)AFI->getStackSizeSVE(), MVT::nxv1i8};
}

bool AArch64FrameLowering::canUseRedZone(const MachineFunction &MF) const {
  if (!EnableRedZone)
    return false;
  // Don't use the red zone if the function explicitly asks us not to.
  // This is typically used for kernel code.
  if (MF.getFunction().hasFnAttribute(Attribute::NoRedZone))
    return false;

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  uint64_t NumBytes = AFI->getLocalStackSize();

  return !(MFI.hasCalls() || hasFP(MF) || NumBytes > 128 ||
           getSVEStackSize(MF));
}

/// hasFP - Return true if the specified function should have a dedicated frame
/// pointer register.
bool AArch64FrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  // Win64 EH requires a frame pointer if funclets are present, as the locals
  // are accessed off the frame pointer in both the parent function and the
  // funclets.
  if (MF.hasEHFunclets())
    return true;
  // Retain behavior of always omitting the FP for leaf functions when possible.
  if (MF.getTarget().Options.DisableFramePointerElim(MF))
    return true;
  if (MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken() ||
      MFI.hasStackMap() || MFI.hasPatchPoint() ||
      RegInfo->needsStackRealignment(MF))
    return true;
  // With large callframes around we may need to use FP to access the scavenging
  // emergency spillslot.
  //
  // Unfortunately some calls to hasFP() like machine verifier ->
  // getReservedReg() -> hasFP in the middle of global isel are too early
  // to know the max call frame size. Hopefully conservatively returning "true"
  // in those cases is fine.
  // DefaultSafeSPDisplacement is fine as we only emergency spill GP regs.
  if (!MFI.isMaxCallFrameSizeComputed() ||
      MFI.getMaxCallFrameSize() > DefaultSafeSPDisplacement)
    return true;

  return false;
}

/// hasReservedCallFrame - Under normal circumstances, when a frame pointer is
/// not required, we reserve argument space for call sites in the function
/// immediately on entry to the current function.  This eliminates the need for
/// add/sub sp brackets around call sites.  Returns true if the call frame is
/// included as part of the stack frame.
bool
AArch64FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

MachineBasicBlock::iterator AArch64FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  const AArch64InstrInfo *TII =
      static_cast<const AArch64InstrInfo *>(MF.getSubtarget().getInstrInfo());
  DebugLoc DL = I->getDebugLoc();
  unsigned Opc = I->getOpcode();
  bool IsDestroy = Opc == TII->getCallFrameDestroyOpcode();
  uint64_t CalleePopAmount = IsDestroy ? I->getOperand(1).getImm() : 0;

  const AArch64RegisterInfo *TRI = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  unsigned SP = TRI->getStackPointerRegister(MF);
  if (!hasReservedCallFrame(MF)) {
    int64_t Amount = I->getOperand(0).getImm();
    Amount = alignTo(Amount, getStackAlign());
    if (!IsDestroy)
      Amount = -Amount;

    // N.b. if CalleePopAmount is valid but zero (i.e. callee would pop, but it
    // doesn't have to pop anything), then the first operand will be zero too so
    // this adjustment is a no-op.
    if (CalleePopAmount == 0) {
      // FIXME: in-function stack adjustment for calls is limited to 24-bits
      // because there's no guaranteed temporary register available.
      //
      // ADD/SUB (immediate) has only LSL #0 and LSL #12 available.
      // 1) For offset <= 12-bit, we use LSL #0
      // 2) For 12-bit <= offset <= 24-bit, we use two instructions. One uses
      // LSL #0, and the other uses LSL #12.
      //
      // Most call frames will be allocated at the start of a function so
      // this is OK, but it is a limitation that needs dealing with.
      assert(Amount > -0xffffff && Amount < 0xffffff && "call frame too large");
      emitFrameOffset(MBB, I, DL, SP, SP, {Amount, MVT::i8},
                      TII);
    }
  } else if (CalleePopAmount != 0) {
    // If the calling convention demands that the callee pops arguments from the
    // stack, we want to add it back if we have a reserved call frame.
    assert(CalleePopAmount < 0xffffff && "call frame too large");
    emitFrameOffset(MBB, I, DL, SP, SP,
                    {-(int64_t)CalleePopAmount, MVT::i8}, TII);
  }
  return MBB.erase(I);
}

// Convenience function to create a DWARF expression for
//   Expr + NumBytes + NumVGScaledBytes * AArch64::VG
static void appendVGScaledOffsetExpr(SmallVectorImpl<char> &Expr,
                                     int NumBytes, int NumVGScaledBytes, unsigned VG,
                                     llvm::raw_string_ostream &Comment) {
  uint8_t buffer[16];

  if (NumBytes) {
    Expr.push_back(dwarf::DW_OP_consts);
    Expr.append(buffer, buffer + encodeSLEB128(NumBytes, buffer));
    Expr.push_back((uint8_t)dwarf::DW_OP_plus);
    Comment << (NumBytes < 0 ? " - " : " + ") << std::abs(NumBytes);
  }

  if (NumVGScaledBytes) {
    Expr.push_back((uint8_t)dwarf::DW_OP_consts);
    Expr.append(buffer, buffer + encodeSLEB128(NumVGScaledBytes, buffer));

    Expr.push_back((uint8_t)dwarf::DW_OP_bregx);
    Expr.append(buffer, buffer + encodeULEB128(VG, buffer));
    Expr.push_back(0);

    Expr.push_back((uint8_t)dwarf::DW_OP_mul);
    Expr.push_back((uint8_t)dwarf::DW_OP_plus);

    Comment << (NumVGScaledBytes < 0 ? " - " : " + ")
            << std::abs(NumVGScaledBytes) << " * VG";
  }
}

// Creates an MCCFIInstruction:
//    { DW_CFA_def_cfa_expression, ULEB128 (sizeof expr), expr }
MCCFIInstruction AArch64FrameLowering::createDefCFAExpressionFromSP(
    const TargetRegisterInfo &TRI, const StackOffset &OffsetFromSP) const {
  int64_t NumBytes, NumVGScaledBytes;
  OffsetFromSP.getForDwarfOffset(NumBytes, NumVGScaledBytes);

  std::string CommentBuffer = "sp";
  llvm::raw_string_ostream Comment(CommentBuffer);

  // Build up the expression (SP + NumBytes + NumVGScaledBytes * AArch64::VG)
  SmallString<64> Expr;
  Expr.push_back((uint8_t)(dwarf::DW_OP_breg0 + /*SP*/ 31));
  Expr.push_back(0);
  appendVGScaledOffsetExpr(Expr, NumBytes, NumVGScaledBytes,
                           TRI.getDwarfRegNum(AArch64::VG, true), Comment);

  // Wrap this into DW_CFA_def_cfa.
  SmallString<64> DefCfaExpr;
  DefCfaExpr.push_back(dwarf::DW_CFA_def_cfa_expression);
  uint8_t buffer[16];
  DefCfaExpr.append(buffer,
                    buffer + encodeULEB128(Expr.size(), buffer));
  DefCfaExpr.append(Expr.str());
  return MCCFIInstruction::createEscape(nullptr, DefCfaExpr.str(),
                                        Comment.str());
}

MCCFIInstruction AArch64FrameLowering::createCfaOffset(
    const TargetRegisterInfo &TRI, unsigned Reg,
    const StackOffset &OffsetFromDefCFA) const {
  int64_t NumBytes, NumVGScaledBytes;
  OffsetFromDefCFA.getForDwarfOffset(NumBytes, NumVGScaledBytes);

  unsigned DwarfReg = TRI.getDwarfRegNum(Reg, true);

  // Non-scalable offsets can use DW_CFA_offset directly.
  if (!NumVGScaledBytes)
    return MCCFIInstruction::createOffset(nullptr, DwarfReg, NumBytes);

  std::string CommentBuffer;
  llvm::raw_string_ostream Comment(CommentBuffer);
  Comment << printReg(Reg, &TRI) << "  @ cfa";

  // Build up expression (NumBytes + NumVGScaledBytes * AArch64::VG)
  SmallString<64> OffsetExpr;
  appendVGScaledOffsetExpr(OffsetExpr, NumBytes, NumVGScaledBytes,
                           TRI.getDwarfRegNum(AArch64::VG, true), Comment);

  // Wrap this into DW_CFA_expression
  SmallString<64> CfaExpr;
  CfaExpr.push_back(dwarf::DW_CFA_expression);
  uint8_t buffer[16];
  CfaExpr.append(buffer, buffer + encodeULEB128(DwarfReg, buffer));
  CfaExpr.append(buffer, buffer + encodeULEB128(OffsetExpr.size(), buffer));
  CfaExpr.append(OffsetExpr.str());

  return MCCFIInstruction::createEscape(nullptr, CfaExpr.str(), Comment.str());
}

void AArch64FrameLowering::emitCalleeSavedFrameMoves(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetSubtargetInfo &STI = MF.getSubtarget();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();
  const TargetInstrInfo *TII = STI.getInstrInfo();
  DebugLoc DL = MBB.findDebugLoc(MBBI);

  // Add callee saved registers to move list.
  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  if (CSI.empty())
    return;

  for (const auto &Info : CSI) {
    unsigned Reg = Info.getReg();

    // Not all unwinders may know about SVE registers, so assume the lowest
    // common demoninator.
    unsigned NewReg;
    if (static_cast<const AArch64RegisterInfo *>(TRI)->regNeedsCFI(Reg, NewReg))
      Reg = NewReg;
    else
      continue;

    StackOffset Offset;
    if (MFI.getStackID(Info.getFrameIdx()) == TargetStackID::SVEVector) {
      AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
      Offset = StackOffset(MFI.getObjectOffset(Info.getFrameIdx()), MVT::nxv1i8) -
               StackOffset(AFI->getCalleeSavedStackSize(MFI), MVT::i8);
    } else {
      Offset = {MFI.getObjectOffset(Info.getFrameIdx()) -
                    getOffsetOfLocalArea(),
                MVT::i8};
    }
    unsigned CFIIndex = MF.addFrameInst(createCfaOffset(*TRI, Reg, Offset));
    BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex)
        .setMIFlags(MachineInstr::FrameSetup);
  }
}

// Find a scratch register that we can use at the start of the prologue to
// re-align the stack pointer.  We avoid using callee-save registers since they
// may appear to be free when this is called from canUseAsPrologue (during
// shrink wrapping), but then no longer be free when this is called from
// emitPrologue.
//
// FIXME: This is a bit conservative, since in the above case we could use one
// of the callee-save registers as a scratch temp to re-align the stack pointer,
// but we would then have to make sure that we were in fact saving at least one
// callee-save register in the prologue, which is additional complexity that
// doesn't seem worth the benefit.
static unsigned findScratchNonCalleeSaveRegister(MachineBasicBlock *MBB,
    unsigned OldScratch = AArch64::NoRegister) {

  static const MCPhysReg ScratchCapReg1[2] = {AArch64::C6, AArch64::C9};
  static const MCPhysReg ScratchCapReg2[2] = {AArch64::C7, AArch64::C10};

  MachineFunction *MF = MBB->getParent();

  const AArch64Subtarget &Subtarget = MF->getSubtarget<AArch64Subtarget>();
  bool HasPureCap = Subtarget.hasPureCap();
  bool Use32CapRegs = !Subtarget.use16CapRegs();
  // We can use an intra-procedural registers here since aligning the
  // prologue doesn't require having this live across branches.
  unsigned DefaultScratch = HasPureCap ? ScratchCapReg1[Use32CapRegs] : AArch64::X9;
  if (OldScratch == DefaultScratch)
    DefaultScratch = HasPureCap ? ScratchCapReg2[Use32CapRegs] : AArch64::X10;

  const TargetRegisterClass ScratchRegClass =
      HasPureCap ? AArch64::CapRegClass : AArch64::GPR64RegClass;

  // If MBB is an entry block, use X9 as the scratch register
  if (&MF->front() == MBB) {
    assert(DefaultScratch != OldScratch &&
           "Should not reuse scratch register");
    return DefaultScratch;
  }

  const AArch64RegisterInfo &TRI = *Subtarget.getRegisterInfo();
  LivePhysRegs LiveRegs(TRI);
  LiveRegs.addLiveIns(*MBB);

  // Mark callee saved registers as used so we will not choose them.
  const MCPhysReg *CSRegs = MF->getRegInfo().getCalleeSavedRegs();
  for (unsigned i = 0; CSRegs[i]; ++i)
    LiveRegs.addReg(CSRegs[i]);

  if (OldScratch != AArch64::NoRegister)
    LiveRegs.addReg(OldScratch);

  // Prefer X9/C6 since it was historically used for the prologue scratch reg.
  const MachineRegisterInfo &MRI = MF->getRegInfo();
  if (LiveRegs.available(MRI, DefaultScratch))
    return DefaultScratch;

  for (unsigned Reg : ScratchRegClass) {
    if (LiveRegs.available(MRI, Reg))
      return Reg;
  }
  return AArch64::NoRegister;
}

bool AArch64FrameLowering::canUseAsPrologue(
    const MachineBasicBlock &MBB) const {
  const MachineFunction *MF = MBB.getParent();
  MachineBasicBlock *TmpMBB = const_cast<MachineBasicBlock *>(&MBB);
  const AArch64Subtarget &Subtarget = MF->getSubtarget<AArch64Subtarget>();
  const AArch64RegisterInfo *RegInfo = Subtarget.getRegisterInfo();

  // Don't need a scratch register if we're not going to re-align the stack.
  if (!RegInfo->needsStackRealignment(*MF))
    return true;
  // Otherwise, we can use any block as long as it has a scratch register
  // available.
  return findScratchNonCalleeSaveRegister(TmpMBB) != AArch64::NoRegister;
}

static bool windowsRequiresStackProbe(MachineFunction &MF,
                                      uint64_t StackSizeInBytes) {
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  if (!Subtarget.isTargetWindows())
    return false;
  const Function &F = MF.getFunction();
  // TODO: When implementing stack protectors, take that into account
  // for the probe threshold.
  unsigned StackProbeSize = 4096;
  if (F.hasFnAttribute("stack-probe-size"))
    F.getFnAttribute("stack-probe-size")
        .getValueAsString()
        .getAsInteger(0, StackProbeSize);
  return (StackSizeInBytes >= StackProbeSize) &&
         !F.hasFnAttribute("no-stack-arg-probe");
}

bool AArch64FrameLowering::shouldCombineCSRLocalStackBump(
    MachineFunction &MF, uint64_t StackBumpBytes) const {
  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  const AArch64RegisterInfo *RegInfo = Subtarget.getRegisterInfo();

  if (AFI->getLocalStackSize() == 0)
    return false;

  // 512 is the maximum immediate for stp/ldp that will be used for
  // callee-save save/restores
  if (StackBumpBytes >= 512 || windowsRequiresStackProbe(MF, StackBumpBytes))
    return false;

  if (MFI.hasVarSizedObjects())
    return false;

  if (RegInfo->needsStackRealignment(MF))
    return false;

  // This isn't strictly necessary, but it simplifies things a bit since the
  // current RedZone handling code assumes the SP is adjusted by the
  // callee-save save/restore code.
  if (canUseRedZone(MF))
    return false;

  // When there is an SVE area on the stack, always allocate the
  // callee-saves and spills/locals separately.
  if (getSVEStackSize(MF))
    return false;

  return true;
}

bool AArch64FrameLowering::shouldCombineCSRLocalStackBumpInEpilogue(
    MachineBasicBlock &MBB, unsigned StackBumpBytes) const {
  if (!shouldCombineCSRLocalStackBump(*MBB.getParent(), StackBumpBytes))
    return false;

  if (MBB.empty())
    return true;

  // Disable combined SP bump if the last instruction is an MTE tag store. It
  // is almost always better to merge SP adjustment into those instructions.
  MachineBasicBlock::iterator LastI = MBB.getFirstTerminator();
  MachineBasicBlock::iterator Begin = MBB.begin();
  while (LastI != Begin) {
    --LastI;
    if (LastI->isTransient())
      continue;
    if (!LastI->getFlag(MachineInstr::FrameDestroy))
      break;
  }
  switch (LastI->getOpcode()) {
  case AArch64::STGloop:
  case AArch64::STZGloop:
  case AArch64::STGOffset:
  case AArch64::STZGOffset:
  case AArch64::ST2GOffset:
  case AArch64::STZ2GOffset:
    return false;
  default:
    return true;
  }
  llvm_unreachable("unreachable");
}

// Given a load or a store instruction, generate an appropriate unwinding SEH
// code on Windows.
static MachineBasicBlock::iterator InsertSEH(MachineBasicBlock::iterator MBBI,
                                             const TargetInstrInfo &TII,
                                             MachineInstr::MIFlag Flag) {
  unsigned Opc = MBBI->getOpcode();
  MachineBasicBlock *MBB = MBBI->getParent();
  MachineFunction &MF = *MBB->getParent();
  DebugLoc DL = MBBI->getDebugLoc();
  unsigned ImmIdx = MBBI->getNumOperands() - 1;
  int Imm = MBBI->getOperand(ImmIdx).getImm();
  MachineInstrBuilder MIB;
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  const AArch64RegisterInfo *RegInfo = Subtarget.getRegisterInfo();

  switch (Opc) {
  default:
    llvm_unreachable("No SEH Opcode for this instruction");
  case AArch64::LDPDpost:
    Imm = -Imm;
    LLVM_FALLTHROUGH;
  case AArch64::STPDpre: {
    unsigned Reg0 = RegInfo->getSEHRegNum(MBBI->getOperand(1).getReg());
    unsigned Reg1 = RegInfo->getSEHRegNum(MBBI->getOperand(2).getReg());
    MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveFRegP_X))
              .addImm(Reg0)
              .addImm(Reg1)
              .addImm(Imm * 8)
              .setMIFlag(Flag);
    break;
  }
  case AArch64::LDPXpost:
    Imm = -Imm;
    LLVM_FALLTHROUGH;
  case AArch64::STPXpre: {
    Register Reg0 = MBBI->getOperand(1).getReg();
    Register Reg1 = MBBI->getOperand(2).getReg();
    if (Reg0 == AArch64::FP && Reg1 == AArch64::LR)
      MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveFPLR_X))
                .addImm(Imm * 8)
                .setMIFlag(Flag);
    else
      MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveRegP_X))
                .addImm(RegInfo->getSEHRegNum(Reg0))
                .addImm(RegInfo->getSEHRegNum(Reg1))
                .addImm(Imm * 8)
                .setMIFlag(Flag);
    break;
  }
  case AArch64::LDRDpost:
    Imm = -Imm;
    LLVM_FALLTHROUGH;
  case AArch64::STRDpre: {
    unsigned Reg = RegInfo->getSEHRegNum(MBBI->getOperand(1).getReg());
    MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveFReg_X))
              .addImm(Reg)
              .addImm(Imm)
              .setMIFlag(Flag);
    break;
  }
  case AArch64::LDRXpost:
    Imm = -Imm;
    LLVM_FALLTHROUGH;
  case AArch64::STRXpre: {
    unsigned Reg =  RegInfo->getSEHRegNum(MBBI->getOperand(1).getReg());
    MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveReg_X))
              .addImm(Reg)
              .addImm(Imm)
              .setMIFlag(Flag);
    break;
  }
  case AArch64::STPDi:
  case AArch64::LDPDi: {
    unsigned Reg0 =  RegInfo->getSEHRegNum(MBBI->getOperand(0).getReg());
    unsigned Reg1 =  RegInfo->getSEHRegNum(MBBI->getOperand(1).getReg());
    MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveFRegP))
              .addImm(Reg0)
              .addImm(Reg1)
              .addImm(Imm * 8)
              .setMIFlag(Flag);
    break;
  }
  case AArch64::STPXi:
  case AArch64::LDPXi: {
    Register Reg0 = MBBI->getOperand(0).getReg();
    Register Reg1 = MBBI->getOperand(1).getReg();
    if (Reg0 == AArch64::FP && Reg1 == AArch64::LR)
      MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveFPLR))
                .addImm(Imm * 8)
                .setMIFlag(Flag);
    else
      MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveRegP))
                .addImm(RegInfo->getSEHRegNum(Reg0))
                .addImm(RegInfo->getSEHRegNum(Reg1))
                .addImm(Imm * 8)
                .setMIFlag(Flag);
    break;
  }
  case AArch64::STRXui:
  case AArch64::LDRXui: {
    int Reg = RegInfo->getSEHRegNum(MBBI->getOperand(0).getReg());
    MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveReg))
              .addImm(Reg)
              .addImm(Imm * 8)
              .setMIFlag(Flag);
    break;
  }
  case AArch64::STRDui:
  case AArch64::LDRDui: {
    unsigned Reg = RegInfo->getSEHRegNum(MBBI->getOperand(0).getReg());
    MIB = BuildMI(MF, DL, TII.get(AArch64::SEH_SaveFReg))
              .addImm(Reg)
              .addImm(Imm * 8)
              .setMIFlag(Flag);
    break;
  }
  }
  auto I = MBB->insertAfter(MBBI, MIB);
  return I;
}

// Fix up the SEH opcode associated with the save/restore instruction.
static void fixupSEHOpcode(MachineBasicBlock::iterator MBBI,
                           unsigned LocalStackSize) {
  MachineOperand *ImmOpnd = nullptr;
  unsigned ImmIdx = MBBI->getNumOperands() - 1;
  switch (MBBI->getOpcode()) {
  default:
    llvm_unreachable("Fix the offset in the SEH instruction");
  case AArch64::SEH_SaveFPLR:
  case AArch64::SEH_SaveRegP:
  case AArch64::SEH_SaveReg:
  case AArch64::SEH_SaveFRegP:
  case AArch64::SEH_SaveFReg:
    ImmOpnd = &MBBI->getOperand(ImmIdx);
    break;
  }
  if (ImmOpnd)
    ImmOpnd->setImm(ImmOpnd->getImm() + LocalStackSize);
}

// Convert callee-save register save/restore instruction to do stack pointer
// decrement/increment to allocate/deallocate the callee-save stack area by
// converting store/load to use pre/post increment version.
static MachineBasicBlock::iterator convertCalleeSaveRestoreToSPPrePostIncDec(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    const DebugLoc &DL, const TargetInstrInfo *TII, int CSStackSizeInc,
    bool NeedsWinCFI, bool *HasWinCFI, bool InProlog = true) {
  MachineFunction &MF = *MBB.getParent();
  const AArch64RegisterInfo *MRI = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  unsigned SP = MRI->getStackPointerRegister(MF);
  const bool HasCap = MF.getSubtarget<AArch64Subtarget>().hasMorello();
  // Ignore instructions that do not operate on SP, i.e. shadow call stack
  // instructions and associated CFI instruction.
  while (MBBI->getOpcode() == AArch64::STRXpost ||
         MBBI->getOpcode() == AArch64::LDRXpre ||
         MBBI->getOpcode() == AArch64::CFI_INSTRUCTION) {
    if (MBBI->getOpcode() != AArch64::CFI_INSTRUCTION) {
      assert(MBBI->getOperand(0).getReg() != AArch64::SP);
      assert(!HasCap && "shadow call stack not supported with capabilities");
    }
    ++MBBI;
  }
  unsigned NewOpc;
  int Scale = 8;
  switch (MBBI->getOpcode()) {
  default:
    llvm_unreachable("Unexpected callee-save save/restore opcode!");
  case AArch64::STPXi:
    NewOpc = AArch64::STPXpre;
    Scale = 8;
    break;
  case AArch64::ASTPXi:
    NewOpc = AArch64::ASTPXpre;
    break;
  case AArch64::STPDi:
    NewOpc = AArch64::STPDpre;
    Scale = 8;
    break;
  case AArch64::STPQi:
    NewOpc = AArch64::STPQpre;
    Scale = 16;
    break;
  case AArch64::ASTPDi:
    NewOpc = AArch64::ASTPDpre;
    break;
  case AArch64::STRXui:
    NewOpc = AArch64::STRXpre;
    Scale = 1;
    break;
  case AArch64::ASTRXui:
    NewOpc = AArch64::ASTRXpre;
    Scale = 1;
    break;
  case AArch64::STRDui:
    NewOpc = AArch64::STRDpre;
    Scale = 1;
    break;
  case AArch64::ASTRDui:
    NewOpc = AArch64::ASTRDpre;
    Scale = 1;
    break;
  case AArch64::STRQui:
    NewOpc = AArch64::STRQpre;
    Scale = 1;
    break;
  case AArch64::LDPXi:
    NewOpc = AArch64::LDPXpost;
    Scale = 8;
    break;
  case AArch64::ALDPXi:
    NewOpc = AArch64::ALDPXpost;
    break;
  case AArch64::LDPDi:
    NewOpc = AArch64::LDPDpost;
    Scale = 8;
    break;
  case AArch64::LDPQi:
    NewOpc = AArch64::LDPQpost;
    Scale = 16;
    break;
  case AArch64::ALDPDi:
    NewOpc = AArch64::ALDPDpost;
    break;
  case AArch64::LDRXui:
    NewOpc = AArch64::LDRXpost;
    Scale = 1;
    break;
  case AArch64::ALDRXui:
    NewOpc = AArch64::ALDRXpost;
    Scale = 1;
    break;
  case AArch64::LDRDui:
    NewOpc = AArch64::LDRDpost;
    Scale = 1;
    break;
  case AArch64::ALDRDui:
    NewOpc = AArch64::ALDRDpost;
    Scale = 1;
    break;
  case AArch64::LDRQui:
    NewOpc = AArch64::LDRQpost;
    Scale = 1;
    break;
  case AArch64::PCapStorePairImmPre:
    NewOpc = AArch64::PCapStorePairImmPreW;
    Scale = 16;
    break;
  case AArch64::CapStorePairImmPre:
    NewOpc = AArch64::CapStorePairImmPreW;
    Scale = 16;
    break;
  case AArch64::PCapStoreImmPre:
    NewOpc = AArch64::PCapStoreImmPreW;
    Scale = 16;
    break;
  case AArch64::CapStoreImmPre:
    NewOpc = AArch64::CapStoreImmPreW;
    Scale = 16;
    break;
  case AArch64::PCapLoadPairImmPre:
    NewOpc = AArch64::PCapLoadPairImmPost;
    Scale = 16;
    break;
  case AArch64::CapLoadPairImmPre:
    NewOpc = AArch64::CapLoadPairImmPost;
    Scale = 16;
    break;
  case AArch64::CapLoadImmPre:
    NewOpc = AArch64::CapLoadImmPost;
    Scale = 16;
    break;
  case AArch64::PCapLoadImmPre:
    NewOpc = AArch64::PCapLoadImmPost;
    Scale = 16;
    break;
  }
  // Get rid of the SEH code associated with the old instruction.
  if (NeedsWinCFI) {
    auto SEH = std::next(MBBI);
    if (AArch64InstrInfo::isSEHInstruction(*SEH))
      SEH->eraseFromParent();
  }

  MachineInstrBuilder MIB = BuildMI(MBB, MBBI, DL, TII->get(NewOpc));
  MIB.addReg(SP, RegState::Define);

  // Copy all operands other than the immediate offset.
  unsigned OpndIdx = 0;
  for (unsigned OpndEnd = MBBI->getNumOperands() - 1; OpndIdx < OpndEnd;
       ++OpndIdx)
    MIB.add(MBBI->getOperand(OpndIdx));

  assert(MBBI->getOperand(OpndIdx).getImm() == 0 &&
         "Unexpected immediate offset in first/last callee-save save/restore "
         "instruction!");
  assert(MBBI->getOperand(OpndIdx - 1).getReg() == SP &&
         "Unexpected base register in callee-save save/restore instruction!");
  assert(CSStackSizeInc % Scale == 0);
  MIB.addImm(CSStackSizeInc / Scale);

  MIB.setMIFlags(MBBI->getFlags());
  MIB.setMemRefs(MBBI->memoperands());

  // Generate a new SEH code that corresponds to the new instruction.
  if (NeedsWinCFI) {
    *HasWinCFI = true;
    InsertSEH(*MIB, *TII,
              InProlog ? MachineInstr::FrameSetup : MachineInstr::FrameDestroy);
  }

  return std::prev(MBB.erase(MBBI));
}

// Fixup callee-save register save/restore instructions to take into account
// combined SP bump by adding the local stack size to the stack offsets.
static void fixupCalleeSaveRestoreStackOffset(MachineInstr &MI,
                                              uint64_t LocalStackSize,
                                              bool NeedsWinCFI,
                                              bool *HasWinCFI) {
  if (AArch64InstrInfo::isSEHInstruction(MI))
    return;

  unsigned Opc = MI.getOpcode();

  // Ignore instructions that do not operate on SP, i.e. shadow call stack
  // instructions and associated CFI instruction.
  if (Opc == AArch64::STRXpost || Opc == AArch64::LDRXpre ||
      Opc == AArch64::CFI_INSTRUCTION) {
    if (Opc != AArch64::CFI_INSTRUCTION)
      assert(MI.getOperand(0).getReg() != AArch64::SP &&
             MI.getOperand(0).getReg() != AArch64::CSP);
    return;
  }

  unsigned Scale;
  switch (Opc) {
  case AArch64::STPXi:
  case AArch64::ASTPXi:
  case AArch64::STRXui:
  case AArch64::ASTRXui:
  case AArch64::STPDi:
  case AArch64::ASTPDi:
  case AArch64::STRDui:
  case AArch64::ASTRDui:
  case AArch64::LDPXi:
  case AArch64::ALDPXi:
  case AArch64::LDRXui:
  case AArch64::ALDRXui:
  case AArch64::LDPDi:
  case AArch64::ALDPDi:
  case AArch64::LDRDui:
  case AArch64::ALDRDui:
    Scale = 8;
    break;
  case AArch64::STPQi:
  case AArch64::ASTPQi:
  case AArch64::STRQui:
  case AArch64::ASTRQui:
  case AArch64::LDPQi:
  case AArch64::ALDPQi:
  case AArch64::LDRQui:
  case AArch64::ALDRQui:
  case AArch64::PCapStorePairImmPre:
  case AArch64::CapStorePairImmPre:
  case AArch64::PCapStoreImmPre:
  case AArch64::CapStoreImmPre:
  case AArch64::PCapLoadPairImmPre:
  case AArch64::CapLoadPairImmPre:
  case AArch64::CapLoadImmPre:
  case AArch64::PCapLoadImmPre:
    Scale = 16;
    break;
  default:
    llvm_unreachable("Unexpected callee-save save/restore opcode!");
  }

  unsigned OffsetIdx = MI.getNumExplicitOperands() - 1;
  assert((MI.getOperand(OffsetIdx - 1).getReg() == AArch64::SP ||
         MI.getOperand(OffsetIdx - 1).getReg() == AArch64::CSP) &&
         "Unexpected base register in callee-save save/restore instruction!");
  // Last operand is immediate offset that needs fixing.
  MachineOperand &OffsetOpnd = MI.getOperand(OffsetIdx);
  // All generated opcodes have scaled offsets.
  assert(LocalStackSize % Scale == 0);
  OffsetOpnd.setImm(OffsetOpnd.getImm() + LocalStackSize / Scale);

  if (NeedsWinCFI) {
    *HasWinCFI = true;
    auto MBBI = std::next(MachineBasicBlock::iterator(MI));
    assert(MBBI != MI.getParent()->end() && "Expecting a valid instruction");
    assert(AArch64InstrInfo::isSEHInstruction(*MBBI) &&
           "Expecting a SEH instruction");
    fixupSEHOpcode(MBBI, LocalStackSize);
  }
}

static void adaptForLdStOpt(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator FirstSPPopI,
                            MachineBasicBlock::iterator LastPopI) {
  // Sometimes (when we restore in the same order as we save), we can end up
  // with code like this:
  //
  // ldp      x26, x25, [sp]
  // ldp      x24, x23, [sp, #16]
  // ldp      x22, x21, [sp, #32]
  // ldp      x20, x19, [sp, #48]
  // add      sp, sp, #64
  //
  // In this case, it is always better to put the first ldp at the end, so
  // that the load-store optimizer can run and merge the ldp and the add into
  // a post-index ldp.
  // If we managed to grab the first pop instruction, move it to the end.
  if (ReverseCSRRestoreSeq)
    MBB.splice(FirstSPPopI, &MBB, LastPopI);
  // We should end up with something like this now:
  //
  // ldp      x24, x23, [sp, #16]
  // ldp      x22, x21, [sp, #32]
  // ldp      x20, x19, [sp, #48]
  // ldp      x26, x25, [sp]
  // add      sp, sp, #64
  //
  // and the load-store optimizer can merge the last two instructions into:
  //
  // ldp      x26, x25, [sp], #64
  //
}

static bool needsWinCFI(const MachineFunction &MF) {
  const Function &F = MF.getFunction();
  return MF.getTarget().getMCAsmInfo()->usesWindowsCFI() &&
         F.needsUnwindTableEntry();
}

static bool isTargetWindows(const MachineFunction &MF) {
  return MF.getSubtarget<AArch64Subtarget>().isTargetWindows();
}

// Convenience function to determine whether I is an SVE callee save.
static bool IsSVECalleeSave(MachineBasicBlock::iterator I) {
  switch (I->getOpcode()) {
  default:
    return false;
  case AArch64::STR_ZXI:
  case AArch64::STR_PXI:
  case AArch64::LDR_ZXI:
  case AArch64::LDR_PXI:
    return I->getFlag(MachineInstr::FrameSetup) ||
           I->getFlag(MachineInstr::FrameDestroy);
  }
}

void AArch64FrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const Function &F = MF.getFunction();
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  const AArch64RegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  MachineModuleInfo &MMI = MF.getMMI();
  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  const bool HasPureCap = MF.getSubtarget<AArch64Subtarget>().hasPureCap();
  bool needsFrameMoves =
      MF.needsFrameMoves() && !MF.getTarget().getMCAsmInfo()->usesWindowsCFI();
  bool HasFP = hasFP(MF);
  bool NeedsWinCFI = needsWinCFI(MF);
  bool HasWinCFI = false;
  auto Cleanup = make_scope_exit([&]() { MF.setHasWinCFI(HasWinCFI); });

  bool IsFunclet = MBB.isEHFuncletEntry();

  // At this point, we're going to decide whether or not the function uses a
  // redzone. In most cases, the function doesn't have a redzone so let's
  // assume that's false and set it to true in the case that there's a redzone.
  AFI->setHasRedZone(false);

  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc DL;
  unsigned SP = RegInfo->getStackPointerRegister(MF);
  unsigned FP = RegInfo->getFramePointerRegister(MF);

  const auto &MFnI = *MF.getInfo<AArch64FunctionInfo>();
  if (MFnI.shouldSignReturnAddress()) {
    if (MFnI.shouldSignWithBKey()) {
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::EMITBKEY))
          .setMIFlag(MachineInstr::FrameSetup);
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::PACIBSP))
          .setMIFlag(MachineInstr::FrameSetup);
    } else {
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::PACIASP))
          .setMIFlag(MachineInstr::FrameSetup);
    }

    unsigned CFIIndex =
        MF.addFrameInst(MCCFIInstruction::createNegateRAState(nullptr));
    BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex)
        .setMIFlags(MachineInstr::FrameSetup);
  }

  // All calls are tail calls in GHC calling conv, and functions have no
  // prologue/epilogue.
  if (MF.getFunction().getCallingConv() == CallingConv::GHC)
    return;

  // Set tagged base pointer to the bottom of the stack frame.
  // Ideally it should match SP value after prologue.
  AFI->setTaggedBasePointerOffset(MFI.getStackSize());

  const StackOffset &SVEStackSize = getSVEStackSize(MF);

  // getStackSize() includes all the locals in its size calculation. We don't
  // include these locals when computing the stack size of a funclet, as they
  // are allocated in the parent's stack frame and accessed via the frame
  // pointer from the funclet.  We only save the callee saved registers in the
  // funclet, which are really the callee saved registers of the parent
  // function, including the funclet.
  int64_t NumBytes = IsFunclet ? getWinEHFuncletFrameSize(MF)
                               : MFI.getStackSize();
  if (!AFI->hasStackFrame() && !windowsRequiresStackProbe(MF, NumBytes)) {
    assert(!HasFP && "unexpected function without stack frame but with FP");
    assert(!SVEStackSize &&
           "unexpected function without stack frame but with SVE objects");
    // All of the stack allocation is for locals.
    AFI->setLocalStackSize(NumBytes);
    if (!NumBytes)
      return;
    // REDZONE: If the stack size is less than 128 bytes, we don't need
    // to actually allocate.
    if (canUseRedZone(MF)) {
      AFI->setHasRedZone(true);
      ++NumRedZoneFunctions;
    } else {
      emitFrameOffset(MBB, MBBI, DL, SP, SP,
                      {-NumBytes, MVT::i8}, TII, MachineInstr::FrameSetup,
                      false, NeedsWinCFI, &HasWinCFI);
      if (!NeedsWinCFI && needsFrameMoves) {
        // Label used to tie together the PROLOG_LABEL and the MachineMoves.
        MCSymbol *FrameLabel = MMI.getContext().createTempSymbol();
          // Encode the stack size of the leaf function.
        unsigned CFIIndex = MF.addFrameInst(
            MCCFIInstruction::cfiDefCfaOffset(FrameLabel, NumBytes));
        BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex)
            .setMIFlags(MachineInstr::FrameSetup);
      }
    }

    if (NeedsWinCFI) {
      HasWinCFI = true;
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_PrologEnd))
          .setMIFlag(MachineInstr::FrameSetup);
    }

    return;
  }

  bool IsWin64 =
      Subtarget.isCallingConvWin64(MF.getFunction().getCallingConv());
  unsigned FixedObject = getFixedObjectSize(MF, AFI, IsWin64, IsFunclet);

  auto PrologueSaveSize = AFI->getCalleeSavedStackSize() + FixedObject;
  // All of the remaining stack allocations are for locals.
  AFI->setLocalStackSize(NumBytes - PrologueSaveSize);
  bool CombineSPBump = shouldCombineCSRLocalStackBump(MF, NumBytes);
  if (CombineSPBump) {
    assert(!SVEStackSize && "Cannot combine SP bump with SVE");
    emitFrameOffset(MBB, MBBI, DL, SP, SP,
                    {-NumBytes, MVT::i8}, TII, MachineInstr::FrameSetup, false,
                    NeedsWinCFI, &HasWinCFI);
    NumBytes = 0;
  } else if (PrologueSaveSize != 0) {
    MBBI = convertCalleeSaveRestoreToSPPrePostIncDec(
        MBB, MBBI, DL, TII, -PrologueSaveSize, NeedsWinCFI, &HasWinCFI);
    NumBytes -= PrologueSaveSize;
  }
  assert(NumBytes >= 0 && "Negative stack allocation size!?");

  // Move past the saves of the callee-saved registers, fixing up the offsets
  // and pre-inc if we decided to combine the callee-save and local stack
  // pointer bump above.
  MachineBasicBlock::iterator End = MBB.end();
  while (MBBI != End && MBBI->getFlag(MachineInstr::FrameSetup) &&
         !IsSVECalleeSave(MBBI)) {
    if (CombineSPBump)
      fixupCalleeSaveRestoreStackOffset(*MBBI, AFI->getLocalStackSize(),
                                        NeedsWinCFI, &HasWinCFI);
    ++MBBI;
  }

  // For funclets the FP belongs to the containing function.
  if (!IsFunclet && HasFP) {
    // Only set up FP if we actually need to.
    int64_t FPOffset = AFI->getCalleeSaveBaseToFrameRecordOffset();

    if (CombineSPBump)
      FPOffset += AFI->getLocalStackSize();

    // Issue    sub fp, sp, FPOffset or
    //          mov fp,sp          when FPOffset is zero.
    // Note: All stores of callee-saved registers are marked as "FrameSetup".
    // This code marks the instruction(s) that set the FP also.
    emitFrameOffset(MBB, MBBI, DL, FP, SP,
                    {FPOffset, MVT::i8}, TII, MachineInstr::FrameSetup, false,
                    NeedsWinCFI, &HasWinCFI);
  }

  if (windowsRequiresStackProbe(MF, NumBytes)) {
    uint64_t NumWords = NumBytes >> 4;
    if (NeedsWinCFI) {
      HasWinCFI = true;
      // alloc_l can hold at most 256MB, so assume that NumBytes doesn't
      // exceed this amount.  We need to move at most 2^24 - 1 into x15.
      // This is at most two instructions, MOVZ follwed by MOVK.
      // TODO: Fix to use multiple stack alloc unwind codes for stacks
      // exceeding 256MB in size.
      if (NumBytes >= (1 << 28))
        report_fatal_error("Stack size cannot exceed 256MB for stack "
                            "unwinding purposes");

      uint32_t LowNumWords = NumWords & 0xFFFF;
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
            .addImm(LowNumWords)
            .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 0))
            .setMIFlag(MachineInstr::FrameSetup);
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_Nop))
            .setMIFlag(MachineInstr::FrameSetup);
      if ((NumWords & 0xFFFF0000) != 0) {
          BuildMI(MBB, MBBI, DL, TII->get(AArch64::MOVKXi), AArch64::X15)
              .addReg(AArch64::X15)
              .addImm((NumWords & 0xFFFF0000) >> 16) // High half
              .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 16))
              .setMIFlag(MachineInstr::FrameSetup);
          BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_Nop))
            .setMIFlag(MachineInstr::FrameSetup);
      }
    } else {
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::MOVi64imm), AArch64::X15)
          .addImm(NumWords)
          .setMIFlags(MachineInstr::FrameSetup);
    }

    switch (MF.getTarget().getCodeModel()) {
    case CodeModel::Tiny:
    case CodeModel::Small:
    case CodeModel::Medium:
    case CodeModel::Kernel:
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::BL))
          .addExternalSymbol("__chkstk")
          .addReg(AArch64::X15, RegState::Implicit)
          .addReg(AArch64::X16, RegState::Implicit | RegState::Define | RegState::Dead)
          .addReg(AArch64::X17, RegState::Implicit | RegState::Define | RegState::Dead)
          .addReg(AArch64::NZCV, RegState::Implicit | RegState::Define | RegState::Dead)
          .setMIFlags(MachineInstr::FrameSetup);
      if (NeedsWinCFI) {
        HasWinCFI = true;
        BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_Nop))
            .setMIFlag(MachineInstr::FrameSetup);
      }
      break;
    case CodeModel::Large:
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::MOVaddrEXT))
          .addReg(AArch64::X16, RegState::Define)
          .addExternalSymbol("__chkstk")
          .addExternalSymbol("__chkstk")
          .setMIFlags(MachineInstr::FrameSetup);
      if (NeedsWinCFI) {
        HasWinCFI = true;
        BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_Nop))
            .setMIFlag(MachineInstr::FrameSetup);
      }

      BuildMI(MBB, MBBI, DL, TII->get(getBLRCallOpcode(MF)))
          .addReg(AArch64::X16, RegState::Kill)
          .addReg(AArch64::X15, RegState::Implicit | RegState::Define)
          .addReg(AArch64::X16, RegState::Implicit | RegState::Define | RegState::Dead)
          .addReg(AArch64::X17, RegState::Implicit | RegState::Define | RegState::Dead)
          .addReg(AArch64::NZCV, RegState::Implicit | RegState::Define | RegState::Dead)
          .setMIFlags(MachineInstr::FrameSetup);
      if (NeedsWinCFI) {
        HasWinCFI = true;
        BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_Nop))
            .setMIFlag(MachineInstr::FrameSetup);
      }
      break;
    }

    BuildMI(MBB, MBBI, DL, TII->get(AArch64::SUBXrx64), AArch64::SP)
        .addReg(AArch64::SP, RegState::Kill)
        .addReg(AArch64::X15, RegState::Kill)
        .addImm(AArch64_AM::getArithExtendImm(AArch64_AM::UXTX, 4))
        .setMIFlags(MachineInstr::FrameSetup);
    if (NeedsWinCFI) {
      HasWinCFI = true;
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_StackAlloc))
          .addImm(NumBytes)
          .setMIFlag(MachineInstr::FrameSetup);
    }
    NumBytes = 0;
  }

  StackOffset AllocateBefore = SVEStackSize, AllocateAfter = {};
  MachineBasicBlock::iterator CalleeSavesBegin = MBBI, CalleeSavesEnd = MBBI;

  // Process the SVE callee-saves to determine what space needs to be
  // allocated.
  if (int64_t CalleeSavedSize = AFI->getSVECalleeSavedStackSize()) {
    // Find callee save instructions in frame.
    CalleeSavesBegin = MBBI;
    assert(IsSVECalleeSave(CalleeSavesBegin) && "Unexpected instruction");
    while (IsSVECalleeSave(MBBI) && MBBI != MBB.getFirstTerminator())
      ++MBBI;
    CalleeSavesEnd = MBBI;

    AllocateBefore = {CalleeSavedSize, MVT::nxv1i8};
    AllocateAfter = SVEStackSize - AllocateBefore;
  }

  // Allocate space for the callee saves (if any).
  emitFrameOffset(MBB, CalleeSavesBegin, DL, AArch64::SP, AArch64::SP,
                  -AllocateBefore, TII,
                  MachineInstr::FrameSetup);

  // Finally allocate remaining SVE stack space.
  emitFrameOffset(MBB, CalleeSavesEnd, DL, AArch64::SP, AArch64::SP,
                  -AllocateAfter, TII,
                  MachineInstr::FrameSetup);

  // Allocate space for the rest of the frame.
  if (NumBytes) {
    // Alignment is required for the parent frame, not the funclet
    const bool NeedsRealignment =
        !IsFunclet && RegInfo->needsStackRealignment(MF);
    unsigned scratchSPReg = SP;

    if (NeedsRealignment) {
      scratchSPReg = findScratchNonCalleeSaveRegister(&MBB);
      assert(scratchSPReg != AArch64::NoRegister);
    }

    // If we're a leaf function, try using the red zone.
    if (!canUseRedZone(MF))
      // FIXME: in the case of dynamic re-alignment, NumBytes doesn't have
      // the correct value here, as NumBytes also includes padding bytes,
      // which shouldn't be counted here.
      emitFrameOffset(MBB, MBBI, DL, scratchSPReg, SP,
                      {-NumBytes, MVT::i8}, TII, MachineInstr::FrameSetup,
                      false, NeedsWinCFI, &HasWinCFI);

    if (NeedsRealignment) {
      const unsigned NrBitsToZero = Log2(MFI.getMaxAlign());
      assert(NrBitsToZero > 1);
      assert(scratchSPReg != SP);

      // SUB X9, SP, NumBytes
      //   -- X9 is temporary register, so shouldn't contain any live data here,
      //   -- free to use. This is already produced by emitFrameOffset above.
      // AND SP, X9, 0b11111...0000
      // The logical immediates have a non-trivial encoding. The following
      // formula computes the encoded immediate with all ones but
      // NrBitsToZero zero bits as least significant bits.
      uint32_t andMaskEncoded = (1 << 12)                         // = N
                                | ((64 - NrBitsToZero) << 6)      // immr
                                | ((64 - NrBitsToZero - 1) << 0); // imms
      if (!HasPureCap) {
        BuildMI(MBB, MBBI, DL, TII->get(AArch64::ANDXri), SP)
            .addReg(scratchSPReg, RegState::Kill)
            .addImm(andMaskEncoded);
      } else {
        BuildMI(MBB, MBBI, DL, TII->get(AArch64::CapAlignDown), SP)
            .addReg(scratchSPReg, RegState::Kill)
            .addImm(NrBitsToZero);
      }
      AFI->setStackRealigned(true);
      if (NeedsWinCFI) {
        HasWinCFI = true;
        BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_StackAlloc))
            .addImm(NumBytes & andMaskEncoded)
            .setMIFlag(MachineInstr::FrameSetup);
      }
    }
  }

  // If we need a base pointer, set it up here. It's whatever the value of the
  // stack pointer is at this point. Any variable size objects will be allocated
  // after this, so we can still use the base pointer to reference locals.
  //
  // FIXME: Clarify FrameSetup flags here.
  // Note: Use emitFrameOffset() like above for FP if the FrameSetup flag is
  // needed.
  // For funclets the BP belongs to the containing function.
  if (!IsFunclet && RegInfo->hasBasePointer(MF)) {
    TII->copyPhysReg(MBB, MBBI, DL, RegInfo->getBaseRegister(MF), SP, false);
    if (NeedsWinCFI) {
      HasWinCFI = true;
      BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_Nop))
          .setMIFlag(MachineInstr::FrameSetup);
    }
  }

  // The very last FrameSetup instruction indicates the end of prologue. Emit a
  // SEH opcode indicating the prologue end.
  if (NeedsWinCFI && HasWinCFI) {
    BuildMI(MBB, MBBI, DL, TII->get(AArch64::SEH_PrologEnd))
        .setMIFlag(MachineInstr::FrameSetup);
  }

  // SEH funclets are passed the frame pointer in X1.  If the parent
  // function uses the base register, then the base register is used
  // directly, and is not retrieved from X1.
  if (IsFunclet && F.hasPersonalityFn()) {
    EHPersonality Per = classifyEHPersonality(F.getPersonalityFn());
    if (isAsynchronousEHPersonality(Per)) {
      BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::COPY), AArch64::FP)
          .addReg(AArch64::X1)
          .setMIFlag(MachineInstr::FrameSetup);
      MBB.addLiveIn(AArch64::X1);
    }
  }

  if (needsFrameMoves) {
    // An example of the prologue:
    //
    //     .globl __foo
    //     .align 2
    //  __foo:
    // Ltmp0:
    //     .cfi_startproc
    //     .cfi_personality 155, ___gxx_personality_v0
    // Leh_func_begin:
    //     .cfi_lsda 16, Lexception33
    //
    //     stp  xa,bx, [sp, -#offset]!
    //     ...
    //     stp  x28, x27, [sp, #offset-32]
    //     stp  fp, lr, [sp, #offset-16]
    //     add  fp, sp, #offset - 16
    //     sub  sp, sp, #1360
    //
    // The Stack:
    //       +-------------------------------------------+
    // 10000 | ........ | ........ | ........ | ........ |
    // 10004 | ........ | ........ | ........ | ........ |
    //       +-------------------------------------------+
    // 10008 | ........ | ........ | ........ | ........ |
    // 1000c | ........ | ........ | ........ | ........ |
    //       +===========================================+
    // 10010 |                X28 Register               |
    // 10014 |                X28 Register               |
    //       +-------------------------------------------+
    // 10018 |                X27 Register               |
    // 1001c |                X27 Register               |
    //       +===========================================+
    // 10020 |                Frame Pointer              | (16 bytes in the
    // 10024 |                Frame Pointer              |  sandbox mode)
    //       +-------------------------------------------+
    // 10028 |                Link Register              | (16 bytes in the
    // 1002c |                Link Register              |  sandbox mode)
    //       +===========================================+
    // 10030 | ........ | ........ | ........ | ........ |
    // 10034 | ........ | ........ | ........ | ........ |
    //       +-------------------------------------------+
    // 10038 | ........ | ........ | ........ | ........ |
    // 1003c | ........ | ........ | ........ | ........ |
    //       +-------------------------------------------+
    //
    //     [sp] = 10030        ::    >>initial value<<
    //     sp = 10020          ::  stp fp, lr, [sp, #-16]!
    //     fp = sp == 10020    ::  mov fp, sp
    //     [sp] == 10020       ::  stp x28, x27, [sp, #-16]!
    //     sp == 10010         ::    >>final value<<
    //
    // The frame pointer (w29) points to address 10020. If we use an offset of
    // '16' from 'w29', we get the CFI offsets of -8 for w30, -16 for w29, -24
    // for w27, and -32 for w28:
    //
    //  Ltmp1:
    //     .cfi_def_cfa w29, 16
    //  Ltmp2:
    //     .cfi_offset w30, -8
    //  Ltmp3:
    //     .cfi_offset w29, -16
    //  Ltmp4:
    //     .cfi_offset w27, -24
    //  Ltmp5:
    //     .cfi_offset w28, -32

    if (HasFP) {
      const int OffsetToFirstCalleeSaveFromFP =
          AFI->getCalleeSaveBaseToFrameRecordOffset() -
          AFI->getCalleeSavedStackSize();
      Register FramePtr = RegInfo->getFrameRegister(MF);

      // Define the current CFA rule to use the provided FP.
      unsigned Reg = RegInfo->getDwarfRegNum(FramePtr, true);
      unsigned CFIIndex = MF.addFrameInst(
          MCCFIInstruction::cfiDefCfa(nullptr, Reg, FixedObject - OffsetToFirstCalleeSaveFromFP));
      BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
          .addCFIIndex(CFIIndex)
          .setMIFlags(MachineInstr::FrameSetup);
    } else {
      unsigned CFIIndex;
      if (SVEStackSize) {
        const TargetSubtargetInfo &STI = MF.getSubtarget();
        const TargetRegisterInfo &TRI = *STI.getRegisterInfo();
        StackOffset TotalSize =
            SVEStackSize + StackOffset((int64_t)MFI.getStackSize(), MVT::i8);
        CFIIndex = MF.addFrameInst(createDefCFAExpressionFromSP(TRI, TotalSize));
      } else {
        // Encode the stack size of the leaf function.
        CFIIndex = MF.addFrameInst(
            MCCFIInstruction::cfiDefCfaOffset(nullptr, MFI.getStackSize()));
      }
      BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
          .addCFIIndex(CFIIndex)
          .setMIFlags(MachineInstr::FrameSetup);
    }

    // Now emit the moves for whatever callee saved regs we have (including FP,
    // LR if those are saved).
    emitCalleeSavedFrameMoves(MBB, MBBI);
  }
}

static void InsertReturnAddressAuth(MachineFunction &MF,
                                    MachineBasicBlock &MBB) {
  const auto &MFI = *MF.getInfo<AArch64FunctionInfo>();
  if (!MFI.shouldSignReturnAddress())
    return;
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();

  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  DebugLoc DL;
  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  // The AUTIASP instruction assembles to a hint instruction before v8.3a so
  // this instruction can safely used for any v8a architecture.
  // From v8.3a onwards there are optimised authenticate LR and return
  // instructions, namely RETA{A,B}, that can be used instead.
  if (Subtarget.hasV8_3aOps() && MBBI != MBB.end() &&
      MBBI->getOpcode() == AArch64::RET_ReallyLR) {
    BuildMI(MBB, MBBI, DL,
            TII->get(MFI.shouldSignWithBKey() ? AArch64::RETAB : AArch64::RETAA))
        .copyImplicitOps(*MBBI);
    MBB.erase(MBBI);
  } else {
    BuildMI(
        MBB, MBBI, DL,
        TII->get(MFI.shouldSignWithBKey() ? AArch64::AUTIBSP : AArch64::AUTIASP))
        .setMIFlag(MachineInstr::FrameDestroy);
  }
}

static bool isFuncletReturnInstr(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  default:
    return false;
  case AArch64::CATCHRET:
  case AArch64::CLEANUPRET:
    return true;
  }
}

void AArch64FrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  const AArch64RegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  DebugLoc DL;
  bool NeedsWinCFI = needsWinCFI(MF);
  bool HasWinCFI = false;
  bool IsFunclet = false;
  auto WinCFI = make_scope_exit([&]() {
    if (!MF.hasWinCFI())
      MF.setHasWinCFI(HasWinCFI);
  });

  if (MBB.end() != MBBI) {
    DL = MBBI->getDebugLoc();
    IsFunclet = isFuncletReturnInstr(*MBBI);
  }

  int64_t NumBytes = IsFunclet ? getWinEHFuncletFrameSize(MF)
                               : MFI.getStackSize();
  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  const bool HasCap = MF.getSubtarget<AArch64Subtarget>().hasMorello();

  // All calls are tail calls in GHC calling conv, and functions have no
  // prologue/epilogue.
  if (MF.getFunction().getCallingConv() == CallingConv::GHC)
    return;

  // Initial and residual are named for consistency with the prologue. Note that
  // in the epilogue, the residual adjustment is executed first.
  uint64_t ArgumentPopSize = getArgumentPopSize(MF, MBB);

  // The stack frame should be like below,
  //
  //      ----------------------                     ---
  //      |                    |                      |
  //      | BytesInStackArgArea|              CalleeArgStackSize
  //      | (NumReusableBytes) |                (of tail call)
  //      |                    |                     ---
  //      |                    |                      |
  //      ---------------------|        ---           |
  //      |                    |         |            |
  //      |   CalleeSavedReg   |         |            |
  //      | (CalleeSavedStackSize)|      |            |
  //      |                    |         |            |
  //      ---------------------|         |         NumBytes
  //      |                    |     StackSize  (StackAdjustUp)
  //      |   LocalStackSize   |         |            |
  //      | (covering callee   |         |            |
  //      |       args)        |         |            |
  //      |                    |         |            |
  //      ----------------------        ---          ---
  //
  // So NumBytes = StackSize + BytesInStackArgArea - CalleeArgStackSize
  //             = StackSize + ArgumentPopSize
  //
  // AArch64TargetLowering::LowerCall figures out ArgumentPopSize and keeps
  // it as the 2nd argument of AArch64ISD::TC_RETURN.

  auto Cleanup = make_scope_exit([&] { InsertReturnAddressAuth(MF, MBB); });

  bool IsWin64 =
      Subtarget.isCallingConvWin64(MF.getFunction().getCallingConv());
  unsigned FixedObject = getFixedObjectSize(MF, AFI, IsWin64, IsFunclet);

  uint64_t AfterCSRPopSize = ArgumentPopSize;
  auto PrologueSaveSize = AFI->getCalleeSavedStackSize() + FixedObject;
  // We cannot rely on the local stack size set in emitPrologue if the function
  // has funclets, as funclets have different local stack size requirements, and
  // the current value set in emitPrologue may be that of the containing
  // function.
  if (MF.hasEHFunclets())
    AFI->setLocalStackSize(NumBytes - PrologueSaveSize);
  bool CombineSPBump = shouldCombineCSRLocalStackBumpInEpilogue(MBB, NumBytes);
  // Assume we can't combine the last pop with the sp restore.

  if (!CombineSPBump && PrologueSaveSize != 0) {
    MachineBasicBlock::iterator Pop = std::prev(MBB.getFirstTerminator());
    while (AArch64InstrInfo::isSEHInstruction(*Pop))
      Pop = std::prev(Pop);

    // The size of the callee saved area in the pure capability ABI can be 256,
    // which is just enough to not be handled by a post-indexed variant of
    // ALDRDui. If that's the case do an add instead.
    bool CanConvertToDec = true;
    if (Pop->getOpcode() == AArch64::ALDRDui && PrologueSaveSize >= 256)
      CanConvertToDec = false;

    // Converting the last ldp to a post-index ldp is valid only if the last
    // ldp's offset is 0.
    const MachineOperand &OffsetOp = Pop->getOperand(Pop->getNumOperands() - 1);
    // If the offset is 0, convert it to a post-index ldp.
    if (OffsetOp.getImm() == 0 && CanConvertToDec)
      convertCalleeSaveRestoreToSPPrePostIncDec(
          MBB, Pop, DL, TII, PrologueSaveSize, NeedsWinCFI, &HasWinCFI, false);
    else {
      // If not, make sure to emit an add after the last ldp.
      // We're doing this by transfering the size to be restored from the
      // adjustment *before* the CSR pops to the adjustment *after* the CSR
      // pops.
      AfterCSRPopSize += PrologueSaveSize;
    }
  }

  // Move past the restores of the callee-saved registers.
  // If we plan on combining the sp bump of the local stack size and the callee
  // save stack size, we might need to adjust the CSR save and restore offsets.
  MachineBasicBlock::iterator LastPopI = MBB.getFirstTerminator();
  MachineBasicBlock::iterator Begin = MBB.begin();
  while (LastPopI != Begin) {
    --LastPopI;
    if (!LastPopI->getFlag(MachineInstr::FrameDestroy) ||
        IsSVECalleeSave(LastPopI)) {
      ++LastPopI;
      break;
    } else if (CombineSPBump)
      fixupCalleeSaveRestoreStackOffset(*LastPopI, AFI->getLocalStackSize(),
                                        NeedsWinCFI, &HasWinCFI);
  }

  unsigned SP = RegInfo->getStackPointerRegister(MF);
  unsigned FP = RegInfo->getFramePointerRegister(MF);

  if (NeedsWinCFI) {
    HasWinCFI = true;
    BuildMI(MBB, LastPopI, DL, TII->get(AArch64::SEH_EpilogStart))
        .setMIFlag(MachineInstr::FrameDestroy);
  }

  const StackOffset &SVEStackSize = getSVEStackSize(MF);

  // If there is a single SP update, insert it before the ret and we're done.
  if (CombineSPBump) {
    assert(!SVEStackSize && "Cannot combine SP bump with SVE");
    emitFrameOffset(MBB, MBB.getFirstTerminator(), DL, SP, SP,
                    {NumBytes + (int64_t)AfterCSRPopSize, MVT::i8}, TII,
                    MachineInstr::FrameDestroy, false, NeedsWinCFI, &HasWinCFI);
    if (NeedsWinCFI && HasWinCFI)
      BuildMI(MBB, MBB.getFirstTerminator(), DL,
              TII->get(AArch64::SEH_EpilogEnd))
          .setMIFlag(MachineInstr::FrameDestroy);
    return;
  }

  NumBytes -= PrologueSaveSize;
  assert(NumBytes >= 0 && "Negative stack allocation size!?");

  // Process the SVE callee-saves to determine what space needs to be
  // deallocated.
  StackOffset DeallocateBefore = {}, DeallocateAfter = SVEStackSize;
  MachineBasicBlock::iterator RestoreBegin = LastPopI, RestoreEnd = LastPopI;
  if (int64_t CalleeSavedSize = AFI->getSVECalleeSavedStackSize()) {
    RestoreBegin = std::prev(RestoreEnd);
    while (RestoreBegin != MBB.begin() &&
           IsSVECalleeSave(std::prev(RestoreBegin)))
      --RestoreBegin;

    assert(IsSVECalleeSave(RestoreBegin) &&
           IsSVECalleeSave(std::prev(RestoreEnd)) && "Unexpected instruction");

    StackOffset CalleeSavedSizeAsOffset = {CalleeSavedSize, MVT::nxv1i8};
    DeallocateBefore = SVEStackSize - CalleeSavedSizeAsOffset;
    DeallocateAfter = CalleeSavedSizeAsOffset;
  }

  // Deallocate the SVE area.
  if (SVEStackSize) {
    if (AFI->isStackRealigned()) {
      if (int64_t CalleeSavedSize = AFI->getSVECalleeSavedStackSize())
        // Set SP to start of SVE callee-save area from which they can
        // be reloaded. The code below will deallocate the stack space
        // space by moving FP -> SP.
        emitFrameOffset(MBB, RestoreBegin, DL, AArch64::SP, AArch64::FP,
                        {-CalleeSavedSize, MVT::nxv1i8}, TII,
                        MachineInstr::FrameDestroy);
    } else {
      if (AFI->getSVECalleeSavedStackSize()) {
        // Deallocate the non-SVE locals first before we can deallocate (and
        // restore callee saves) from the SVE area.
        emitFrameOffset(MBB, RestoreBegin, DL, AArch64::SP, AArch64::SP,
                        {NumBytes, MVT::i8}, TII, MachineInstr::FrameDestroy);
        NumBytes = 0;
      }

      emitFrameOffset(MBB, RestoreBegin, DL, AArch64::SP, AArch64::SP,
                      DeallocateBefore, TII, MachineInstr::FrameDestroy);

      emitFrameOffset(MBB, RestoreEnd, DL, AArch64::SP, AArch64::SP,
                      DeallocateAfter, TII, MachineInstr::FrameDestroy);
    }
  }

  if (!hasFP(MF)) {
    bool RedZone = canUseRedZone(MF);
    // If this was a redzone leaf function, we don't need to restore the
    // stack pointer (but we may need to pop stack args for fastcc).
    if (RedZone && AfterCSRPopSize == 0)
      return;

    bool NoCalleeSaveRestore = PrologueSaveSize == 0;
    int64_t StackRestoreBytes = RedZone ? 0 : NumBytes;
    if (NoCalleeSaveRestore)
      StackRestoreBytes += AfterCSRPopSize;

    // If we were able to combine the local stack pop with the argument pop,
    // then we're done.
    bool Done = NoCalleeSaveRestore || AfterCSRPopSize == 0;

    // If we're done after this, make sure to help the load store optimizer.
    if (Done)
      adaptForLdStOpt(MBB, MBB.getFirstTerminator(), LastPopI);

    emitFrameOffset(MBB, LastPopI, DL, SP, SP,
                    {StackRestoreBytes, MVT::i8}, TII,
                    MachineInstr::FrameDestroy, false, NeedsWinCFI, &HasWinCFI);
    if (Done) {
      if (NeedsWinCFI) {
        HasWinCFI = true;
        BuildMI(MBB, MBB.getFirstTerminator(), DL,
                TII->get(AArch64::SEH_EpilogEnd))
            .setMIFlag(MachineInstr::FrameDestroy);
      }
      return;
    }

    NumBytes = 0;
  }

  // Restore the original stack pointer.
  // FIXME: Rather than doing the math here, we should instead just use
  // non-post-indexed loads for the restores if we aren't actually going to
  // be able to save any instructions.
  if (!IsFunclet && (MFI.hasVarSizedObjects() || AFI->isStackRealigned())) {
    emitFrameOffset(MBB, LastPopI, DL, SP, FP,
                    {-AFI->getCalleeSaveBaseToFrameRecordOffset(), MVT::i8},
                    TII, MachineInstr::FrameDestroy, false, NeedsWinCFI);
  } else if (NumBytes)
    emitFrameOffset(MBB, LastPopI, DL, SP, SP,
                    {NumBytes, MVT::i8}, TII, MachineInstr::FrameDestroy, false,
                    NeedsWinCFI);

  // This must be placed after the callee-save restore code because that code
  // assumes the SP is at the same location as it was after the callee-save save
  // code in the prologue.
  if (AfterCSRPopSize) {
    // Find an insertion point for the first ldp so that it goes before the
    // shadow call stack epilog instruction. This ensures that the restore of
    // lr from x18 is placed after the restore from sp.
    auto FirstSPPopI = MBB.getFirstTerminator();
    while (FirstSPPopI != Begin) {
      auto Prev = std::prev(FirstSPPopI);
      if (Prev->getOpcode() != AArch64::LDRXpre ||
          Prev->getOperand(0).getReg() == SP ||
          HasCap)
        break;
      FirstSPPopI = Prev;
    }

    adaptForLdStOpt(MBB, FirstSPPopI, LastPopI);

    emitFrameOffset(MBB, FirstSPPopI, DL, SP, SP,
                    {(int64_t)AfterCSRPopSize, MVT::i8}, TII,
                    MachineInstr::FrameDestroy, false, NeedsWinCFI, &HasWinCFI);
  }
  if (NeedsWinCFI && HasWinCFI)
    BuildMI(MBB, MBB.getFirstTerminator(), DL, TII->get(AArch64::SEH_EpilogEnd))
        .setMIFlag(MachineInstr::FrameDestroy);
}

/// getFrameIndexReference - Provide a base+offset reference to an FI slot for
/// debug info.  It's the same as what we use for resolving the code-gen
/// references for now.  FIXME: This can go wrong when references are
/// SP-relative and simple call frames aren't used.
int AArch64FrameLowering::getFrameIndexReference(const MachineFunction &MF,
                                                 int FI,
                                                 Register &FrameReg) const {
  return resolveFrameIndexReference(
             MF, FI, FrameReg,
             /*PreferFP=*/
             MF.getFunction().hasFnAttribute(Attribute::SanitizeHWAddress),
             /*ForSimm=*/false)
      .getBytes();
}

int AArch64FrameLowering::getNonLocalFrameIndexReference(
  const MachineFunction &MF, int FI) const {
  return getSEHFrameIndexOffset(MF, FI);
}

static StackOffset getFPOffset(const MachineFunction &MF, int64_t ObjectOffset) {
  const auto *AFI = MF.getInfo<AArch64FunctionInfo>();
  const auto &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  bool IsWin64 =
      Subtarget.isCallingConvWin64(MF.getFunction().getCallingConv());
  unsigned FixedObject =
      getFixedObjectSize(MF, AFI, IsWin64, /*IsFunclet=*/false);
  int64_t CalleeSaveSize = AFI->getCalleeSavedStackSize(MF.getFrameInfo());
  int64_t FPAdjust =
      CalleeSaveSize - AFI->getCalleeSaveBaseToFrameRecordOffset();
  return {ObjectOffset + FixedObject + FPAdjust, MVT::i8};
}

static StackOffset getStackOffset(const MachineFunction &MF, int64_t ObjectOffset) {
  const auto &MFI = MF.getFrameInfo();
  return {ObjectOffset + (int64_t)MFI.getStackSize(), MVT::i8};
}

int AArch64FrameLowering::getSEHFrameIndexOffset(const MachineFunction &MF,
                                                 int FI) const {
  const auto *RegInfo = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  int ObjectOffset = MF.getFrameInfo().getObjectOffset(FI);
  return RegInfo->getLocalAddressRegister(MF) == AArch64::FP
             ? getFPOffset(MF, ObjectOffset).getBytes()
             : getStackOffset(MF, ObjectOffset).getBytes();
}

StackOffset AArch64FrameLowering::resolveFrameIndexReference(
    const MachineFunction &MF, int FI, Register &FrameReg, bool PreferFP,
    bool ForSimm) const {
  const auto &MFI = MF.getFrameInfo();
  int64_t ObjectOffset = MFI.getObjectOffset(FI);
  bool isFixed = MFI.isFixedObjectIndex(FI);
  bool isSVE = MFI.getStackID(FI) == TargetStackID::SVEVector;
  return resolveFrameOffsetReference(MF, ObjectOffset, isFixed, isSVE, FrameReg,
                                     PreferFP, ForSimm);
}

StackOffset AArch64FrameLowering::resolveFrameOffsetReference(
    const MachineFunction &MF, int64_t ObjectOffset, bool isFixed, bool isSVE,
    Register &FrameReg, bool PreferFP, bool ForSimm) const {
  const auto &MFI = MF.getFrameInfo();
  const auto *RegInfo = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  const auto *AFI = MF.getInfo<AArch64FunctionInfo>();
  const auto &Subtarget = MF.getSubtarget<AArch64Subtarget>();

  int64_t FPOffset = getFPOffset(MF, ObjectOffset).getBytes();
  int64_t Offset = getStackOffset(MF, ObjectOffset).getBytes();
  bool isCSR =
      !isFixed && ObjectOffset >= -((int)AFI->getCalleeSavedStackSize(MFI));

  const StackOffset &SVEStackSize = getSVEStackSize(MF);

  // Use frame pointer to reference fixed objects. Use it for locals if
  // there are VLAs or a dynamically realigned SP (and thus the SP isn't
  // reliable as a base). Make sure useFPForScavengingIndex() does the
  // right thing for the emergency spill slot.
  bool UseFP = false;
  if (AFI->hasStackFrame() && !isSVE) {
    // We shouldn't prefer using the FP when there is an SVE area
    // in between the FP and the non-SVE locals/spills.
    PreferFP &= !SVEStackSize;

    // Note: Keeping the following as multiple 'if' statements rather than
    // merging to a single expression for readability.
    //
    // Argument access should always use the FP.
    if (isFixed) {
      UseFP = hasFP(MF);
    } else if (isCSR && RegInfo->needsStackRealignment(MF)) {
      // References to the CSR area must use FP if we're re-aligning the stack
      // since the dynamically-sized alignment padding is between the SP/BP and
      // the CSR area.
      assert(hasFP(MF) && "Re-aligned stack must have frame pointer");
      UseFP = true;
    } else if (hasFP(MF) && !RegInfo->needsStackRealignment(MF)) {
      // If the FPOffset is negative and we're producing a signed immediate, we
      // have to keep in mind that the available offset range for negative
      // offsets is smaller than for positive ones. If an offset is available
      // via the FP and the SP, use whichever is closest.
      bool FPOffsetFits = !ForSimm || FPOffset >= -256;
      PreferFP |= Offset > -FPOffset;

      if (MFI.hasVarSizedObjects()) {
        // If we have variable sized objects, we can use either FP or BP, as the
        // SP offset is unknown. We can use the base pointer if we have one and
        // FP is not preferred. If not, we're stuck with using FP.
        bool CanUseBP = RegInfo->hasBasePointer(MF);
        if (FPOffsetFits && CanUseBP) // Both are ok. Pick the best.
          UseFP = PreferFP;
        else if (!CanUseBP) // Can't use BP. Forced to use FP.
          UseFP = true;
        // else we can use BP and FP, but the offset from FP won't fit.
        // That will make us scavenge registers which we can probably avoid by
        // using BP. If it won't fit for BP either, we'll scavenge anyway.
      } else if (FPOffset >= 0) {
        // Use SP or FP, whichever gives us the best chance of the offset
        // being in range for direct access. If the FPOffset is positive,
        // that'll always be best, as the SP will be even further away.
        UseFP = true;
      } else if (MF.hasEHFunclets() && !RegInfo->hasBasePointer(MF)) {
        // Funclets access the locals contained in the parent's stack frame
        // via the frame pointer, so we have to use the FP in the parent
        // function.
        (void) Subtarget;
        assert(
            Subtarget.isCallingConvWin64(MF.getFunction().getCallingConv()) &&
            "Funclets should only be present on Win64");
        UseFP = true;
      } else {
        // We have the choice between FP and (SP or BP).
        if (FPOffsetFits && PreferFP) // If FP is the best fit, use it.
          UseFP = true;
      }
    }
  }

  assert(((isFixed || isCSR) || !RegInfo->needsStackRealignment(MF) || !UseFP) &&
         "In the presence of dynamic stack pointer realignment, "
         "non-argument/CSR objects cannot be accessed through the frame pointer");

  if (isSVE) {
    int64_t OffsetFromSPToSVEArea =
        MFI.getStackSize() - AFI->getCalleeSavedStackSize();
    int64_t OffsetFromFPToSVEArea =
        -AFI->getCalleeSaveBaseToFrameRecordOffset();
    StackOffset FPOffset = StackOffset(OffsetFromFPToSVEArea, MVT::i8) +
                           StackOffset(ObjectOffset, MVT::nxv1i8);
    StackOffset SPOffset = SVEStackSize +
                           StackOffset(ObjectOffset, MVT::nxv1i8) +
                           StackOffset(OffsetFromSPToSVEArea, MVT::i8);
    // Always use the FP for SVE spills if available and beneficial.
    if (hasFP(MF) &&
        (SPOffset.getBytes() ||
         FPOffset.getScalableBytes() < SPOffset.getScalableBytes() ||
         RegInfo->needsStackRealignment(MF))) {
      FrameReg = RegInfo->getFrameRegister(MF);
      return FPOffset;
    }

    FrameReg = RegInfo->hasBasePointer(MF) ? RegInfo->getBaseRegister(MF)
                                           : (unsigned)AArch64::SP;
    return SPOffset;
  }

  StackOffset ScalableOffset = {};
  if (UseFP && !(isFixed || isCSR))
    ScalableOffset = -SVEStackSize;
  if (!UseFP && (isFixed || isCSR))
    ScalableOffset = SVEStackSize;

  if (UseFP) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return StackOffset(FPOffset, MVT::i8) + ScalableOffset;
  }

  // Use the base pointer if we have one.
  if (RegInfo->hasBasePointer(MF))
    FrameReg = RegInfo->getBaseRegister(MF);
  else {
    assert(!MFI.hasVarSizedObjects() &&
           "Can't use SP when we have var sized objects.");
    FrameReg = RegInfo->getStackPointerRegister(MF);
    // If we're using the red zone for this function, the SP won't actually
    // be adjusted, so the offsets will be negative. They're also all
    // within range of the signed 9-bit immediate instructions.
    if (canUseRedZone(MF))
      Offset -= AFI->getLocalStackSize();
  }

  return StackOffset(Offset, MVT::i8) + ScalableOffset;
}

static unsigned getPrologueDeath(MachineFunction &MF, unsigned Reg) {
  // Do not set a kill flag on values that are also marked as live-in. This
  // happens with the @llvm-returnaddress intrinsic and with arguments passed in
  // callee saved registers.
  // Omitting the kill flags is conservatively correct even if the live-in
  // is not used after all.
  bool IsLiveIn = MF.getRegInfo().isLiveIn(Reg);
  return getKillRegState(!IsLiveIn);
}

static bool produceCompactUnwindFrame(MachineFunction &MF) {
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  AttributeList Attrs = MF.getFunction().getAttributes();
  return Subtarget.isTargetMachO() &&
         !(Subtarget.getTargetLowering()->supportSwiftError() &&
           Attrs.hasAttrSomewhere(Attribute::SwiftError));
}

static bool invalidateWindowsRegisterPairing(unsigned Reg1, unsigned Reg2,
                                             bool NeedsWinCFI) {
  // If we are generating register pairs for a Windows function that requires
  // EH support, then pair consecutive registers only.  There are no unwind
  // opcodes for saves/restores of non-consectuve register pairs.
  // The unwind opcodes are save_regp, save_regp_x, save_fregp, save_frepg_x.
  // https://docs.microsoft.com/en-us/cpp/build/arm64-exception-handling

  // TODO: LR can be paired with any register.  We don't support this yet in
  // the MCLayer.  We need to add support for the save_lrpair unwind code.
  if (Reg2 == AArch64::FP)
    return true;
  if (!NeedsWinCFI)
    return false;
  if (Reg2 == Reg1 + 1)
    return false;
  return true;
}

/// Returns true if Reg1 and Reg2 cannot be paired using a ldp/stp instruction.
/// WindowsCFI requires that only consecutive registers can be paired.
/// LR and FP need to be allocated together when the frame needs to save
/// the frame-record. This means any other register pairing with LR is invalid.
static bool invalidateRegisterPairing(unsigned Reg1, unsigned Reg2,
                                      bool UsesWinAAPCS, bool NeedsWinCFI, bool NeedsFrameRecord,
                                      unsigned LR) {
  if (UsesWinAAPCS)
    return invalidateWindowsRegisterPairing(Reg1, Reg2, NeedsWinCFI);

  // If we need to store the frame record, don't pair any register
  // with LR other than FP.
  if (NeedsFrameRecord)
    return Reg2 == LR;

  return false;
}

namespace {

struct RegPairInfo {
  unsigned Reg1 = AArch64::NoRegister;
  unsigned Reg2 = AArch64::NoRegister;
  int FrameIdx;
  int Offset;
  enum RegType { GPR, Cap, FPR64, FPR128, PPR, ZPR } Type;

  RegPairInfo() = default;

  bool isPaired() const { return Reg2 != AArch64::NoRegister; }

  unsigned getScale() const {
    switch (Type) {
    case PPR:
      return 2;
    case GPR:
    case FPR64:
      return 8;
    case ZPR:
    case FPR128:
    case Cap:
      return 16;
    }
    llvm_unreachable("Unsupported type");
  }

  bool isScalable() const { return Type == PPR || Type == ZPR; }
};

} // end anonymous namespace

static void computeCalleeSaveRegisterPairs(
    MachineFunction &MF, ArrayRef<CalleeSavedInfo> CSI,
    const TargetRegisterInfo *TRI, SmallVectorImpl<RegPairInfo> &RegPairs,
    bool &NeedShadowCallStackProlog, bool NeedsFrameRecord) {

  if (CSI.empty())
    return;

  bool IsWindows = isTargetWindows(MF);
  bool NeedsWinCFI = needsWinCFI(MF);
  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  CallingConv::ID CC = MF.getFunction().getCallingConv();
  unsigned Count = CSI.size();
  (void)CC;
  // MachO's compact unwind format relies on all registers being stored in
  // pairs.
  assert((!produceCompactUnwindFrame(MF) ||
          CC == CallingConv::PreserveMost ||
          (Count & 1) == 0) &&
         "Odd number of callee-saved regs to spill!");
  int ByteOffset = AFI->getCalleeSavedStackSize();
  const AArch64RegisterInfo *RegInfo = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  unsigned FP = RegInfo->getFramePointerRegister(MF);
  unsigned LR = RegInfo->getLinkRegister(MF);
  int ScalableByteOffset = AFI->getSVECalleeSavedStackSize();
  // On Linux, we will have either one or zero non-paired register.  On Windows
  // with CFI, we can have multiple unpaired registers in order to utilize the
  // available unwind codes.  This flag assures that the alignment fixup is done
  // only once, as intened.
  bool FixupDone = false;

  for (unsigned i = 0; i < Count; ++i) {
    RegPairInfo RPI;
    RPI.Reg1 = CSI[i].getReg();

    if (AArch64::GPR64RegClass.contains(RPI.Reg1))
      RPI.Type = RegPairInfo::GPR;
    else if (AArch64::CapRegClass.contains(RPI.Reg1))
      RPI.Type = RegPairInfo::Cap;
    else if (AArch64::FPR64RegClass.contains(RPI.Reg1))
      RPI.Type = RegPairInfo::FPR64;
    else if (AArch64::FPR128RegClass.contains(RPI.Reg1))
      RPI.Type = RegPairInfo::FPR128;
    else if (AArch64::ZPRRegClass.contains(RPI.Reg1))
      RPI.Type = RegPairInfo::ZPR;
    else if (AArch64::PPRRegClass.contains(RPI.Reg1))
      RPI.Type = RegPairInfo::PPR;
    else
      llvm_unreachable("Unsupported register class.");

    // Add the next reg to the pair if it is in the same register class.
    if (i + 1 < Count) {
      unsigned NextReg = CSI[i + 1].getReg();
      switch (RPI.Type) {
      case RegPairInfo::GPR:
        if (AArch64::GPR64RegClass.contains(NextReg) &&
            !invalidateRegisterPairing(RPI.Reg1, NextReg, IsWindows, NeedsWinCFI,
                                       NeedsFrameRecord, LR))
          RPI.Reg2 = NextReg;
        break;
      case RegPairInfo::Cap:
        if (AArch64::CapRegClass.contains(NextReg) &&
            !invalidateRegisterPairing(RPI.Reg1, NextReg, IsWindows, NeedsWinCFI,
                                       NeedsFrameRecord, LR))
          RPI.Reg2 = NextReg;
        break;
      case RegPairInfo::FPR64:
        if (AArch64::FPR64RegClass.contains(NextReg) &&
            !invalidateWindowsRegisterPairing(RPI.Reg1, NextReg, NeedsWinCFI))
          RPI.Reg2 = NextReg;
        break;
      case RegPairInfo::FPR128:
        if (AArch64::FPR128RegClass.contains(NextReg))
          RPI.Reg2 = NextReg;
        break;
      case RegPairInfo::PPR:
      case RegPairInfo::ZPR:
        break;
      }
    }

    if (MF.getFunction().hasFnAttribute(Attribute::ShadowCallStack) &&
        MF.getSubtarget<AArch64Subtarget>().hasPureCap()) {
      report_fatal_error("Shadow call stack not supported with Morello");
    }

    // If either of the registers to be saved is the lr register, it means that
    // we also need to save lr in the shadow call stack.
    if ((RPI.Reg1 == AArch64::LR || RPI.Reg2 == AArch64::LR) &&
        MF.getFunction().hasFnAttribute(Attribute::ShadowCallStack)) {
      if (!MF.getSubtarget<AArch64Subtarget>().isXRegisterReserved(18))
        report_fatal_error("Must reserve x18 to use shadow call stack");
      NeedShadowCallStackProlog = true;
    }

    // GPRs and FPRs are saved in pairs of 64-bit regs. We expect the CSI
    // list to come in sorted by frame index so that we can issue the store
    // pair instructions directly. Assert if we see anything otherwise.
    //
    // The order of the registers in the list is controlled by
    // getCalleeSavedRegs(), so they will always be in-order, as well.
    assert((!RPI.isPaired() ||
            (CSI[i].getFrameIdx() + 1 == CSI[i + 1].getFrameIdx())) &&
           "Out of order callee saved regs!");

    assert((!RPI.isPaired() || !NeedsFrameRecord || RPI.Reg2 != FP ||
            RPI.Reg1 == LR) &&
           "FrameRecord must be allocated together with LR");

    // Windows AAPCS has FP and LR reversed.
    assert((!RPI.isPaired() || !NeedsFrameRecord || RPI.Reg1 != AArch64::FP ||
            RPI.Reg2 == AArch64::LR) &&
           "FrameRecord must be allocated together with LR");

    // MachO's compact unwind format relies on all registers being stored in
    // adjacent register pairs.
    assert((!produceCompactUnwindFrame(MF) ||
            CC == CallingConv::PreserveMost ||
            (RPI.isPaired() &&
             ((RPI.Reg1 == LR && RPI.Reg2 == FP) ||
              RPI.Reg1 + 1 == RPI.Reg2))) &&
           "Callee-save registers not saved as adjacent register pair!");

    RPI.FrameIdx = CSI[i].getFrameIdx();

    int Scale = RPI.getScale();
    if (RPI.isScalable())
      ScalableByteOffset -= Scale;
    else
      ByteOffset -= RPI.isPaired() ? 2 * Scale : Scale;

    assert(!(RPI.isScalable() && RPI.isPaired()) &&
           "Paired spill/fill instructions don't exist for SVE vectors");

    // We need to align to 16 bytes if:
    //   - this is an unpaired register and the last CSR, with size 8
    //    or
    //   - the next CSR requires 16 bytes alignment.
    bool NeedsAlign = false;
    if (!RPI.isPaired()) {
      if (i + 1 != Count) {
        unsigned NextReg = CSI[i + 1].getReg();
        if (AArch64::CapRegClass.contains(NextReg))
          NeedsAlign = true;
      }
    }

    // Round up size of non-pair to pair size if we need to pad the
    // callee-save area to ensure 16-byte alignment.
    if ((AFI->hasCalleeSaveStackFreeSpace() || NeedsAlign) && !FixupDone &&
        !RPI.isScalable() && Scale == 8 && ByteOffset % 16 == 8 &&
        !RPI.isPaired()) {
      FixupDone = !NeedsAlign;
      ByteOffset -= 8;
      assert(ByteOffset % 16 == 0);
      assert(MFI.getObjectAlign(RPI.FrameIdx) <= Align(16));
      MFI.setObjectAlignment(RPI.FrameIdx, Align(16));
    }

    int Offset = RPI.isScalable() ? ScalableByteOffset : ByteOffset;
    assert(Offset % Scale == 0);
    RPI.Offset = Offset / Scale;

    assert(((!RPI.isScalable() && RPI.Offset >= -64 && RPI.Offset <= 63) ||
            (RPI.isScalable() && RPI.Offset >= -256 && RPI.Offset <= 255)) &&
           "Offset out of bounds for LDP/STP immediate");

    // Save the offset to frame record so that the FP register can point to the
    // innermost frame record (spilled FP and LR registers).
    if (NeedsFrameRecord && ((!IsWindows && RPI.Reg1 == LR &&
                              RPI.Reg2 == FP) ||
                             (IsWindows && RPI.Reg1 == FP &&
                              RPI.Reg2 == LR)))
      AFI->setCalleeSaveBaseToFrameRecordOffset(Offset);

    RegPairs.push_back(RPI);
    if (RPI.isPaired())
      ++i;
  }
}

bool AArch64FrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  const AArch64RegisterInfo &RegInfo = *static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  bool NeedsWinCFI = needsWinCFI(MF);
  DebugLoc DL;
  SmallVector<RegPairInfo, 8> RegPairs;

  bool NeedShadowCallStackProlog = false;
  computeCalleeSaveRegisterPairs(MF, CSI, TRI, RegPairs,
                                 NeedShadowCallStackProlog, hasFP(MF));
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  unsigned SP = RegInfo.getStackPointerRegister(MF);

  if (NeedShadowCallStackProlog) {
    // Shadow call stack prolog: str x30, [x18], #8
    BuildMI(MBB, MI, DL, TII.get(AArch64::STRXpost))
        .addReg(AArch64::X18, RegState::Define)
        .addReg(AArch64::LR)
        .addReg(AArch64::X18)
        .addImm(8)
        .setMIFlag(MachineInstr::FrameSetup);

    if (NeedsWinCFI)
      BuildMI(MBB, MI, DL, TII.get(AArch64::SEH_Nop))
          .setMIFlag(MachineInstr::FrameSetup);

    if (!MF.getFunction().hasFnAttribute(Attribute::NoUnwind)) {
      // Emit a CFI instruction that causes 8 to be subtracted from the value of
      // x18 when unwinding past this frame.
      static const char CFIInst[] = {
          dwarf::DW_CFA_val_expression,
          18, // register
          2,  // length
          static_cast<char>(unsigned(dwarf::DW_OP_breg18)),
          static_cast<char>(-8) & 0x7f, // addend (sleb128)
      };
      unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createEscape(
          nullptr, StringRef(CFIInst, sizeof(CFIInst))));
      BuildMI(MBB, MI, DL, TII.get(AArch64::CFI_INSTRUCTION))
          .addCFIIndex(CFIIndex)
          .setMIFlag(MachineInstr::FrameSetup);
    }

    // This instruction also makes x18 live-in to the entry block.
    MBB.addLiveIn(AArch64::X18);
  }

  const bool HasC64 = MF.getSubtarget<AArch64Subtarget>().hasC64();

  for (auto RPII = RegPairs.rbegin(), RPIE = RegPairs.rend(); RPII != RPIE;
       ++RPII) {
    RegPairInfo RPI = *RPII;
    unsigned Reg1 = RPI.Reg1;
    unsigned Reg2 = RPI.Reg2;
    unsigned StrOpc;

    // Issue sequence of spills for cs regs.  The first spill may be converted
    // to a pre-decrement store later by emitPrologue if the callee-save stack
    // area allocation can't be combined with the local stack area allocation.
    // For example:
    //    stp     x22, x21, [sp, #0]     // addImm(+0)
    //    stp     x20, x19, [sp, #16]    // addImm(+2)
    //    stp     fp, lr, [sp, #32]      // addImm(+4)
    // Rationale: This sequence saves uop updates compared to a sequence of
    // pre-increment spills like stp xi,xj,[sp,#-16]!
    // Note: Similar rationale and sequence for restores in epilog.
    unsigned Size;
    Align Alignment;
    switch (RPI.Type) {
    case RegPairInfo::GPR:
       XRegsSpills += (RPI.isPaired() ? 2 : 1);
       StrOpc = RPI.isPaired() ?
                 (HasC64 ? AArch64::ASTPXi :AArch64::STPXi) :
                 (HasC64 ? AArch64::ASTRXui : AArch64::STRXui);

       Size = 8;
       Alignment = Align(8);
       break;
    case RegPairInfo::Cap:
       CapabilitySpills += (RPI.isPaired() ? 2 : 1);
       StrOpc = RPI.isPaired() ?
              (HasC64 ? AArch64::PCapStorePairImmPre
                      : AArch64::CapStorePairImmPre) :
              (HasC64 ? AArch64::PCapStoreImmPre
                      : AArch64::CapStoreImmPre);

       Size = 16;
       Alignment = Align(16);
       break;
    case RegPairInfo::FPR64:
       StrOpc = RPI.isPaired() ?
                 (HasC64 ? AArch64::ASTPDi : AArch64::STPDi) :
                 (HasC64 ? AArch64::ASTRDui : AArch64::STRDui);
       Size = 8;
       Alignment = Align(8);
       break;
    case RegPairInfo::FPR128:
       StrOpc = RPI.isPaired() ? AArch64::STPQi : AArch64::STRQui;
       assert(!MF.getSubtarget<AArch64Subtarget>().hasMorello() &&
              "Vector ABI not suppprted with capabilities");
       Size = 16;
       Alignment = Align(16);
       break;
    case RegPairInfo::ZPR:
       StrOpc = AArch64::STR_ZXI;
       Size = 16;
       Alignment = Align(16);
       break;
    case RegPairInfo::PPR:
       StrOpc = AArch64::STR_PXI;
       Size = 2;
       Alignment = Align(2);
       break;
    }
    LLVM_DEBUG(dbgs() << "CSR spill: (" << printReg(Reg1, TRI);
               if (RPI.isPaired()) dbgs() << ", " << printReg(Reg2, TRI);
               dbgs() << ") -> fi#(" << RPI.FrameIdx;
               if (RPI.isPaired()) dbgs() << ", " << RPI.FrameIdx + 1;
               dbgs() << ")\n");

    assert((!NeedsWinCFI || !(Reg1 == AArch64::LR && Reg2 == AArch64::FP)) &&
           "Windows unwdinding requires a consecutive (FP,LR) pair");
    // Windows unwind codes require consecutive registers if registers are
    // paired.  Make the switch here, so that the code below will save (x,x+1)
    // and not (x+1,x).
    unsigned FrameIdxReg1 = RPI.FrameIdx;
    unsigned FrameIdxReg2 = RPI.FrameIdx + 1;
    if (NeedsWinCFI && RPI.isPaired()) {
      std::swap(Reg1, Reg2);
      std::swap(FrameIdxReg1, FrameIdxReg2);
    }
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, TII.get(StrOpc));
    if (!MRI.isReserved(Reg1))
      MBB.addLiveIn(Reg1);
    if (RPI.isPaired()) {
      if (!MRI.isReserved(Reg2))
        MBB.addLiveIn(Reg2);
      MIB.addReg(Reg2, getPrologueDeath(MF, Reg2));
      MIB.addMemOperand(MF.getMachineMemOperand(
          MachinePointerInfo::getFixedStack(MF, FrameIdxReg2),
          MachineMemOperand::MOStore, Size, Alignment));
    }

    MIB.addReg(Reg1, getPrologueDeath(MF, Reg1))
        .addReg(SP)
        .addImm(RPI.Offset) // [sp, #offset*scale],
                            // where factor*scale is implicit
        .setMIFlag(MachineInstr::FrameSetup);
    MIB.addMemOperand(MF.getMachineMemOperand(
        MachinePointerInfo::getFixedStack(MF, FrameIdxReg1),
        MachineMemOperand::MOStore, Size, Alignment));
    if (NeedsWinCFI)
      InsertSEH(MIB, TII, MachineInstr::FrameSetup);

    // Update the StackIDs of the SVE stack slots.
    MachineFrameInfo &MFI = MF.getFrameInfo();
    if (RPI.Type == RegPairInfo::ZPR || RPI.Type == RegPairInfo::PPR)
      MFI.setStackID(RPI.FrameIdx, TargetStackID::SVEVector);

  }
  return true;
}

bool AArch64FrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  const AArch64RegisterInfo *MRI = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  DebugLoc DL;
  SmallVector<RegPairInfo, 8> RegPairs;
  bool NeedsWinCFI = needsWinCFI(MF);

  unsigned SP = MRI->getStackPointerRegister(MF);
  const bool HasC64 = MF.getSubtarget<AArch64Subtarget>().hasC64();

  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  bool NeedShadowCallStackProlog = false;
  computeCalleeSaveRegisterPairs(MF, CSI, TRI, RegPairs,
                                 NeedShadowCallStackProlog, hasFP(MF));

  auto EmitMI = [&](const RegPairInfo &RPI) {
    unsigned Reg1 = RPI.Reg1;
    unsigned Reg2 = RPI.Reg2;

    // Issue sequence of restores for cs regs. The last restore may be converted
    // to a post-increment load later by emitEpilogue if the callee-save stack
    // area allocation can't be combined with the local stack area allocation.
    // For example:
    //    ldp     fp, lr, [sp, #32]       // addImm(+4)
    //    ldp     x20, x19, [sp, #16]     // addImm(+2)
    //    ldp     x22, x21, [sp, #0]      // addImm(+0)
    // Note: see comment in spillCalleeSavedRegisters()
    unsigned LdrOpc;
    unsigned Size;
    Align Alignment;
    switch (RPI.Type) {
    case RegPairInfo::GPR:
       LdrOpc = RPI.isPaired() ?
                (HasC64 ? AArch64::ALDPXi : AArch64::LDPXi) :
                (HasC64 ? AArch64::ALDRXui : AArch64::LDRXui);
       Size = 8;
       Alignment = Align(8);
       break;
    case RegPairInfo::Cap:
       LdrOpc = RPI.isPaired() ?
                (HasC64 ? AArch64::PCapLoadPairImmPre
                        : AArch64::CapLoadPairImmPre) :
                (HasC64 ? AArch64::PCapLoadImmPre
                        : AArch64::CapLoadImmPre);
       Size = 16;
       Alignment = Align(16);
       break;
    case RegPairInfo::FPR64:
       LdrOpc = RPI.isPaired() ?
                (HasC64 ? AArch64::ALDPDi : AArch64::LDPDi) :
                (HasC64 ? AArch64::ALDRDui :AArch64::LDRDui);
       Size = 8;
       Alignment = Align(8);
       break;
    case RegPairInfo::FPR128:
       LdrOpc = RPI.isPaired() ? AArch64::LDPQi : AArch64::LDRQui;
       assert(!MF.getSubtarget<AArch64Subtarget>().hasMorello() &&
              "Vector ABI not suppprted with Morello");
       Size = 16;
       Alignment = Align(16);
       break;
    case RegPairInfo::ZPR:
       LdrOpc = AArch64::LDR_ZXI;
       Size = 16;
       Alignment = Align(16);
       break;
    case RegPairInfo::PPR:
       LdrOpc = AArch64::LDR_PXI;
       Size = 2;
       Alignment = Align(2);
       break;
    }
    LLVM_DEBUG(dbgs() << "CSR restore: (" << printReg(Reg1, TRI);
               if (RPI.isPaired()) dbgs() << ", " << printReg(Reg2, TRI);
               dbgs() << ") -> fi#(" << RPI.FrameIdx;
               if (RPI.isPaired()) dbgs() << ", " << RPI.FrameIdx + 1;
               dbgs() << ")\n");

    // Windows unwind codes require consecutive registers if registers are
    // paired.  Make the switch here, so that the code below will save (x,x+1)
    // and not (x+1,x).
    unsigned FrameIdxReg1 = RPI.FrameIdx;
    unsigned FrameIdxReg2 = RPI.FrameIdx + 1;
    if (NeedsWinCFI && RPI.isPaired()) {
      std::swap(Reg1, Reg2);
      std::swap(FrameIdxReg1, FrameIdxReg2);
    }
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, TII.get(LdrOpc));
    if (RPI.isPaired()) {
      MIB.addReg(Reg2, getDefRegState(true));
      MIB.addMemOperand(MF.getMachineMemOperand(
          MachinePointerInfo::getFixedStack(MF, FrameIdxReg2),
          MachineMemOperand::MOLoad, Size, Alignment));
    }
    int64_t Imm = RPI.Offset;

    MIB.addReg(Reg1, getDefRegState(true))
        .addReg(SP)
        .addImm(Imm) // [sp, #offset*scale]
                            // where factor*scale is implicit
        .setMIFlag(MachineInstr::FrameDestroy);
    MIB.addMemOperand(MF.getMachineMemOperand(
        MachinePointerInfo::getFixedStack(MF, FrameIdxReg1),
        MachineMemOperand::MOLoad, Size, Alignment));
    if (NeedsWinCFI)
      InsertSEH(MIB, TII, MachineInstr::FrameDestroy);
  };

  // SVE objects are always restored in reverse order.
  for (const RegPairInfo &RPI : reverse(RegPairs))
    if (RPI.isScalable())
      EmitMI(RPI);

  if (ReverseCSRRestoreSeq) {
    for (const RegPairInfo &RPI : reverse(RegPairs))
      if (!RPI.isScalable())
        EmitMI(RPI);
  } else
    for (const RegPairInfo &RPI : RegPairs)
      if (!RPI.isScalable())
        EmitMI(RPI);

  if (NeedShadowCallStackProlog) {
    // Shadow call stack epilog: ldr x30, [x18, #-8]!
    BuildMI(MBB, MI, DL, TII.get(AArch64::LDRXpre))
        .addReg(AArch64::X18, RegState::Define)
        .addReg(AArch64::LR, RegState::Define)
        .addReg(AArch64::X18)
        .addImm(-8)
        .setMIFlag(MachineInstr::FrameDestroy);
  }

  return true;
}

void AArch64FrameLowering::determineCalleeSaves(MachineFunction &MF,
                                                BitVector &SavedRegs,
                                                RegScavenger *RS) const {
  // All calls are tail calls in GHC calling conv, and functions have no
  // prologue/epilogue.
  if (MF.getFunction().getCallingConv() == CallingConv::GHC)
    return;

  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  const AArch64RegisterInfo *RegInfo = static_cast<const AArch64RegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());
  const AArch64Subtarget &Subtarget = MF.getSubtarget<AArch64Subtarget>();
  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  unsigned UnspilledCSGPR = AArch64::NoRegister;
  unsigned UnspilledCSGPRPaired = AArch64::NoRegister;
  const bool HasCapRegs = MF.getSubtarget<AArch64Subtarget>().hasMorello();
  const bool HasPureCap = MF.getSubtarget<AArch64Subtarget>().hasPureCap();

  MachineFrameInfo &MFI = MF.getFrameInfo();
  const MCPhysReg *CSRegs = MF.getRegInfo().getCalleeSavedRegs();

  unsigned BasePointerReg = RegInfo->hasBasePointer(MF)
                                ? RegInfo->getBaseRegister(MF)
                                : (unsigned)AArch64::NoRegister;

  unsigned ExtraCSSpill = 0;

  // Figure out which callee-saved registers to save/restore.
  for (unsigned i = 0; CSRegs[i]; ++i) {
    const unsigned Reg = CSRegs[i];

    // Add the base pointer register to SavedRegs if it is callee-save.
    if (RegInfo->isSuperOrSubRegisterEq(Reg, BasePointerReg))
      SavedRegs.set(Reg);

    bool RegUsed = SavedRegs.test(Reg);
    unsigned PairedReg = AArch64::NoRegister;
    if (AArch64::GPR64RegClass.contains(Reg) ||
        AArch64::FPR64RegClass.contains(Reg) ||
        AArch64::FPR128RegClass.contains(Reg))
      PairedReg = CSRegs[i ^ 1];

    if (!RegUsed) {
      if (((AArch64::GPR64RegClass.contains(Reg) && !HasPureCap) ||
           (AArch64::CapRegClass.contains(Reg) && HasPureCap)) &&
          !RegInfo->isReservedReg(MF, Reg)) {
        UnspilledCSGPR = Reg;
        UnspilledCSGPRPaired = PairedReg;
        if (HasPureCap && !AArch64::CapRegClass.contains(Reg))
          UnspilledCSGPRPaired = AArch64::NoRegister;
      }
      continue;
    }

    // MachO's compact unwind format relies on all registers being stored in
    // pairs.
    // FIXME: the usual format is actually better if unwinding isn't needed.
    if (produceCompactUnwindFrame(MF) && PairedReg != AArch64::NoRegister &&
        !SavedRegs.test(PairedReg)) {
      SavedRegs.set(PairedReg);
      if (((AArch64::GPR64RegClass.contains(PairedReg)  && !HasPureCap) ||
           (AArch64::CapRegClass.contains(PairedReg) && HasPureCap)) &&
          !RegInfo->isReservedReg(MF, PairedReg))
        ExtraCSSpill = PairedReg;
    }
  }

  if (MF.getFunction().getCallingConv() == CallingConv::Win64 &&
      !Subtarget.isTargetWindows()) {
    // For Windows calling convention on a non-windows OS, where X18 is treated
    // as reserved, back up X18 when entering non-windows code (marked with the
    // Windows calling convention) and restore when returning regardless of
    // whether the individual function uses it - it might call other functions
    // that clobber it.
    SavedRegs.set(AArch64::X18);
  }

  // Calculates the callee saved stack size.
  unsigned CSStackSize = 0;
  unsigned SVECSStackSize = 0;
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  for (unsigned Reg : SavedRegs.set_bits()) {
    auto RegSize = TRI->getRegSizeInBits(Reg, MRI) / 8;
    if (AArch64::PPRRegClass.contains(Reg) ||
        AArch64::ZPRRegClass.contains(Reg))
      SVECSStackSize += RegSize;
    else
      CSStackSize += RegSize;
  }

  // GPRs are saved first and then capabilities in order to get the frame
  // record just above the FP register spill area. If we have an odd number of
  // GPRs and at least one capability, adjust the callee saved area size to
  // make up for the padding. Don't do this for the alternate mode (A64 + pure
  // capabilitiy ABI, as we are spilling capability registers first there).
  bool NeedsGPRPadding = false;
  bool HasCap = hasFP(MF);
  if (HasPureCap) {
    unsigned NumGPRs = 0;
    for (unsigned Reg : SavedRegs.set_bits()) {
      if (AArch64::GPR64RegClass.contains(Reg))
        NumGPRs++;
      if (AArch64::CapRegClass.contains(Reg))
        HasCap = true;
    }
    if (NumGPRs % 2 == 1)
      NeedsGPRPadding = true;
    if (HasCap && NumGPRs % 2 == 1) {
        CSStackSize += 8;
        HasCap = true;
    }
  }

  // Save number of saved regs, so we can easily update CSStackSize later.
  unsigned NumSavedRegs = SavedRegs.count();

  // The frame record needs to be created by saving the appropriate registers
  uint64_t EstimatedStackSize = MFI.estimateStackSize(MF);
  if (hasFP(MF) ||
      windowsRequiresStackProbe(MF, EstimatedStackSize + CSStackSize + 16)) {
    if (HasPureCap) {
      AFI->setFrameRecordSize(32);
      SavedRegs.set(AArch64::CFP);
      SavedRegs.set(AArch64::CLR);
    } else {
      AFI->setFrameRecordSize(16);
      SavedRegs.set(AArch64::FP);
      SavedRegs.set(AArch64::LR);
    }
  }

  LLVM_DEBUG(dbgs() << "*** determineCalleeSaves\nSaved CSRs:";
             for (unsigned Reg
                  : SavedRegs.set_bits()) dbgs()
             << ' ' << printReg(Reg, RegInfo);
             dbgs() << "\n";);

  // If any callee-saved registers are used, the frame cannot be eliminated.
  int64_t SVEStackSize =
      alignTo(SVECSStackSize + estimateSVEStackObjectOffsets(MFI), 16);
  bool CanEliminateFrame = (SavedRegs.count() == 0) && !SVEStackSize;

  // The CSR spill slots have not been allocated yet, so estimateStackSize
  // won't include them.
  unsigned EstimatedStackSizeLimit = estimateRSStackSizeLimit(MF);

  // Conservatively always assume BigStack when there are SVE spills.
  bool BigStack = SVEStackSize ||
                  (EstimatedStackSize + CSStackSize) > EstimatedStackSizeLimit;
  if (BigStack || !CanEliminateFrame || RegInfo->cannotEliminateFrame(MF))
    AFI->setHasStackFrame(true);

  // Estimate if we might need to scavenge a register at some point in order
  // to materialize a stack offset. If so, either spill one additional
  // callee-saved register or reserve a special spill slot to facilitate
  // register scavenging. If we already spilled an extra callee-saved register
  // above to keep the number of spills even, we don't need to do anything else
  // here.
  if (BigStack) {
    if (!ExtraCSSpill && UnspilledCSGPR != AArch64::NoRegister) {
      LLVM_DEBUG(dbgs() << "Spilling " << printReg(UnspilledCSGPR, RegInfo)
                        << " to get a scratch register.\n");
      SavedRegs.set(UnspilledCSGPR);
      // MachO's compact unwind format relies on all registers being stored in
      // pairs, so if we need to spill one extra for BigStack, then we need to
      // store the pair.
      if (produceCompactUnwindFrame(MF))
        SavedRegs.set(UnspilledCSGPRPaired);
      ExtraCSSpill = UnspilledCSGPR;
    }

    // If we didn't find an extra callee-saved register to spill, create
    // an emergency spill slot.
    if (!ExtraCSSpill || MF.getRegInfo().isPhysRegUsed(ExtraCSSpill)) {
      const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
      const TargetRegisterClass &RC =
          HasCapRegs ? AArch64::CapRegClass : AArch64::GPR64RegClass;
      unsigned Size = TRI->getSpillSize(RC);
      Align Alignment = TRI->getSpillAlign(RC);
      int FI = MFI.CreateStackObject(Size, Alignment, false);
      RS->addScavengingFrameIndex(FI);
      LLVM_DEBUG(dbgs() << "No available CS registers, allocated fi#" << FI
                   << " as the emergency spill slot.\n");
    }
  }

  // Adding the size of additional GPR/Cap saves.
  CSStackSize += (HasPureCap ? 16 : 8) * (SavedRegs.count() - NumSavedRegs);
  // If we've pushed any capability registers and we need padding, add this now.
  if (HasPureCap && NeedsGPRPadding && !HasCap &&
      SavedRegs.count() - NumSavedRegs != (hasFP(MF) ? 2 : 0))
    CSStackSize += 8;
  uint64_t AlignedCSStackSize = alignTo(CSStackSize, 16);
  LLVM_DEBUG(dbgs() << "Estimated stack frame size: "
               << EstimatedStackSize + AlignedCSStackSize
               << " bytes.\n");

  assert((!MFI.isCalleeSavedInfoValid() ||
          AFI->getCalleeSavedStackSize() == AlignedCSStackSize) &&
         "Should not invalidate callee saved info");

  // Round up to register pair alignment to avoid additional SP adjustment
  // instructions.
  AFI->setCalleeSavedStackSize(AlignedCSStackSize);
  AFI->setCalleeSaveStackHasFreeSpace(AlignedCSStackSize != CSStackSize);
  AFI->setSVECalleeSavedStackSize(alignTo(SVECSStackSize, 16));
}

bool AArch64FrameLowering::enableStackSlotScavenging(
    const MachineFunction &MF) const {
  const AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  return AFI->hasCalleeSaveStackFreeSpace();
}

/// returns true if there are any SVE callee saves.
static bool getSVECalleeSaveSlotRange(const MachineFrameInfo &MFI,
                                      int &Min, int &Max) {
  Min = std::numeric_limits<int>::max();
  Max = std::numeric_limits<int>::min();

  if (!MFI.isCalleeSavedInfoValid())
    return false;

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  for (auto &CS : CSI) {
    if (AArch64::ZPRRegClass.contains(CS.getReg()) ||
        AArch64::PPRRegClass.contains(CS.getReg())) {
      assert((Max == std::numeric_limits<int>::min() ||
              Max + 1 == CS.getFrameIdx()) &&
             "SVE CalleeSaves are not consecutive");

      Min = std::min(Min, CS.getFrameIdx());
      Max = std::max(Max, CS.getFrameIdx());
    }
  }
  return Min != std::numeric_limits<int>::max();
}

// Process all the SVE stack objects and determine offsets for each
// object. If AssignOffsets is true, the offsets get assigned.
// Fills in the first and last callee-saved frame indices into
// Min/MaxCSFrameIndex, respectively.
// Returns the size of the stack.
static int64_t determineSVEStackObjectOffsets(MachineFrameInfo &MFI,
                                              int &MinCSFrameIndex,
                                              int &MaxCSFrameIndex,
                                              bool AssignOffsets) {
#ifndef NDEBUG
  // First process all fixed stack objects.
  for (int I = MFI.getObjectIndexBegin(); I != 0; ++I)
    assert(MFI.getStackID(I) != TargetStackID::SVEVector &&
           "SVE vectors should never be passed on the stack by value, only by "
           "reference.");
#endif

  auto Assign = [&MFI](int FI, int64_t Offset) {
    LLVM_DEBUG(dbgs() << "alloc FI(" << FI << ") at SP[" << Offset << "]\n");
    MFI.setObjectOffset(FI, Offset);
  };

  int64_t Offset = 0;

  // Then process all callee saved slots.
  if (getSVECalleeSaveSlotRange(MFI, MinCSFrameIndex, MaxCSFrameIndex)) {
    // Assign offsets to the callee save slots.
    for (int I = MinCSFrameIndex; I <= MaxCSFrameIndex; ++I) {
      Offset += MFI.getObjectSize(I);
      Offset = alignTo(Offset, MFI.getObjectAlign(I));
      if (AssignOffsets)
        Assign(I, -Offset);
    }
  }

  // Ensure that the Callee-save area is aligned to 16bytes.
  Offset = alignTo(Offset, Align(16U));

  // Create a buffer of SVE objects to allocate and sort it.
  SmallVector<int, 8> ObjectsToAllocate;
  for (int I = 0, E = MFI.getObjectIndexEnd(); I != E; ++I) {
    unsigned StackID = MFI.getStackID(I);
    if (StackID != TargetStackID::SVEVector)
      continue;
    if (MaxCSFrameIndex >= I && I >= MinCSFrameIndex)
      continue;
    if (MFI.isDeadObjectIndex(I))
      continue;

    ObjectsToAllocate.push_back(I);
  }

  // Allocate all SVE locals and spills
  for (unsigned FI : ObjectsToAllocate) {
    Align Alignment = MFI.getObjectAlign(FI);
    // FIXME: Given that the length of SVE vectors is not necessarily a power of
    // two, we'd need to align every object dynamically at runtime if the
    // alignment is larger than 16. This is not yet supported.
    if (Alignment > Align(16))
      report_fatal_error(
          "Alignment of scalable vectors > 16 bytes is not yet supported");

    Offset = alignTo(Offset + MFI.getObjectSize(FI), Alignment);
    if (AssignOffsets)
      Assign(FI, -Offset);
  }

  return Offset;
}

int64_t AArch64FrameLowering::estimateSVEStackObjectOffsets(
    MachineFrameInfo &MFI) const {
  int MinCSFrameIndex, MaxCSFrameIndex;
  return determineSVEStackObjectOffsets(MFI, MinCSFrameIndex, MaxCSFrameIndex, false);
}

int64_t AArch64FrameLowering::assignSVEStackObjectOffsets(
    MachineFrameInfo &MFI, int &MinCSFrameIndex, int &MaxCSFrameIndex) const {
  return determineSVEStackObjectOffsets(MFI, MinCSFrameIndex, MaxCSFrameIndex,
                                        true);
}

void AArch64FrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();

  assert(getStackGrowthDirection() == TargetFrameLowering::StackGrowsDown &&
         "Upwards growing stack unsupported");

  int MinCSFrameIndex, MaxCSFrameIndex;
  int64_t SVEStackSize =
      assignSVEStackObjectOffsets(MFI, MinCSFrameIndex, MaxCSFrameIndex);

  AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
  AFI->setStackSizeSVE(alignTo(SVEStackSize, 16U));
  AFI->setMinMaxSVECSFrameIndex(MinCSFrameIndex, MaxCSFrameIndex);

  // If this function isn't doing Win64-style C++ EH, we don't need to do
  // anything.
  if (!MF.hasEHFunclets())
    return;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  WinEHFuncInfo &EHInfo = *MF.getWinEHFuncInfo();

  MachineBasicBlock &MBB = MF.front();
  auto MBBI = MBB.begin();
  while (MBBI != MBB.end() && MBBI->getFlag(MachineInstr::FrameSetup))
    ++MBBI;

  // Create an UnwindHelp object.
  // The UnwindHelp object is allocated at the start of the fixed object area
  int64_t FixedObject =
      getFixedObjectSize(MF, AFI, /*IsWin64*/ true, /*IsFunclet*/ false);
  int UnwindHelpFI = MFI.CreateFixedObject(/*Size*/ 8,
                                           /*SPOffset*/ -FixedObject,
                                           /*IsImmutable=*/false);
  EHInfo.UnwindHelpFrameIdx = UnwindHelpFI;

  // We need to store -2 into the UnwindHelp object at the start of the
  // function.
  DebugLoc DL;
  RS->enterBasicBlockEnd(MBB);
  RS->backward(std::prev(MBBI));
  unsigned DstReg = RS->FindUnusedReg(&AArch64::GPR64commonRegClass);
  assert(DstReg && "There must be a free register after frame setup");
  BuildMI(MBB, MBBI, DL, TII.get(AArch64::MOVi64imm), DstReg).addImm(-2);
  BuildMI(MBB, MBBI, DL, TII.get(AArch64::STURXi))
      .addReg(DstReg, getKillRegState(true))
      .addFrameIndex(UnwindHelpFI)
      .addImm(0);
}

namespace {
struct TagStoreInstr {
  MachineInstr *MI;
  int64_t Offset, Size;
  explicit TagStoreInstr(MachineInstr *MI, int64_t Offset, int64_t Size)
      : MI(MI), Offset(Offset), Size(Size) {}
};

class TagStoreEdit {
  MachineFunction *MF;
  MachineBasicBlock *MBB;
  MachineRegisterInfo *MRI;
  // Tag store instructions that are being replaced.
  SmallVector<TagStoreInstr, 8> TagStores;
  // Combined memref arguments of the above instructions.
  SmallVector<MachineMemOperand *, 8> CombinedMemRefs;

  // Replace allocation tags in [FrameReg + FrameRegOffset, FrameReg +
  // FrameRegOffset + Size) with the address tag of SP.
  Register FrameReg;
  StackOffset FrameRegOffset;
  int64_t Size;
  // If not None, move FrameReg to (FrameReg + FrameRegUpdate) at the end.
  Optional<int64_t> FrameRegUpdate;
  // MIFlags for any FrameReg updating instructions.
  unsigned FrameRegUpdateFlags;

  // Use zeroing instruction variants.
  bool ZeroData;
  DebugLoc DL;

  void emitUnrolled(MachineBasicBlock::iterator InsertI);
  void emitLoop(MachineBasicBlock::iterator InsertI);

public:
  TagStoreEdit(MachineBasicBlock *MBB, bool ZeroData)
      : MBB(MBB), ZeroData(ZeroData) {
    MF = MBB->getParent();
    MRI = &MF->getRegInfo();
  }
  // Add an instruction to be replaced. Instructions must be added in the
  // ascending order of Offset, and have to be adjacent.
  void addInstruction(TagStoreInstr I) {
    assert((TagStores.empty() ||
            TagStores.back().Offset + TagStores.back().Size == I.Offset) &&
           "Non-adjacent tag store instructions.");
    TagStores.push_back(I);
  }
  void clear() { TagStores.clear(); }
  // Emit equivalent code at the given location, and erase the current set of
  // instructions. May skip if the replacement is not profitable. May invalidate
  // the input iterator and replace it with a valid one.
  void emitCode(MachineBasicBlock::iterator &InsertI,
                const AArch64FrameLowering *TFI, bool IsLast);
};

void TagStoreEdit::emitUnrolled(MachineBasicBlock::iterator InsertI) {
  const AArch64InstrInfo *TII =
      MF->getSubtarget<AArch64Subtarget>().getInstrInfo();

  const int64_t kMinOffset = -256 * 16;
  const int64_t kMaxOffset = 255 * 16;

  Register BaseReg = FrameReg;
  int64_t BaseRegOffsetBytes = FrameRegOffset.getBytes();
  if (BaseRegOffsetBytes < kMinOffset ||
      BaseRegOffsetBytes + (Size - Size % 32) > kMaxOffset) {
    Register ScratchReg = MRI->createVirtualRegister(&AArch64::GPR64RegClass);
    emitFrameOffset(*MBB, InsertI, DL, ScratchReg, BaseReg,
                    {BaseRegOffsetBytes, MVT::i8}, TII);
    BaseReg = ScratchReg;
    BaseRegOffsetBytes = 0;
  }

  MachineInstr *LastI = nullptr;
  while (Size) {
    int64_t InstrSize = (Size > 16) ? 32 : 16;
    unsigned Opcode =
        InstrSize == 16
            ? (ZeroData ? AArch64::STZGOffset : AArch64::STGOffset)
            : (ZeroData ? AArch64::STZ2GOffset : AArch64::ST2GOffset);
    MachineInstr *I = BuildMI(*MBB, InsertI, DL, TII->get(Opcode))
                          .addReg(AArch64::SP)
                          .addReg(BaseReg)
                          .addImm(BaseRegOffsetBytes / 16)
                          .setMemRefs(CombinedMemRefs);
    // A store to [BaseReg, #0] should go last for an opportunity to fold the
    // final SP adjustment in the epilogue.
    if (BaseRegOffsetBytes == 0)
      LastI = I;
    BaseRegOffsetBytes += InstrSize;
    Size -= InstrSize;
  }

  if (LastI)
    MBB->splice(InsertI, MBB, LastI);
}

void TagStoreEdit::emitLoop(MachineBasicBlock::iterator InsertI) {
  const AArch64InstrInfo *TII =
      MF->getSubtarget<AArch64Subtarget>().getInstrInfo();

  Register BaseReg = FrameRegUpdate
                         ? FrameReg
                         : MRI->createVirtualRegister(&AArch64::GPR64RegClass);
  Register SizeReg = MRI->createVirtualRegister(&AArch64::GPR64RegClass);

  emitFrameOffset(*MBB, InsertI, DL, BaseReg, FrameReg, FrameRegOffset, TII);

  int64_t LoopSize = Size;
  // If the loop size is not a multiple of 32, split off one 16-byte store at
  // the end to fold BaseReg update into.
  if (FrameRegUpdate && *FrameRegUpdate)
    LoopSize -= LoopSize % 32;
  MachineInstr *LoopI = BuildMI(*MBB, InsertI, DL,
                                TII->get(ZeroData ? AArch64::STZGloop_wback
                                                  : AArch64::STGloop_wback))
                            .addDef(SizeReg)
                            .addDef(BaseReg)
                            .addImm(LoopSize)
                            .addReg(BaseReg)
                            .setMemRefs(CombinedMemRefs);
  if (FrameRegUpdate)
    LoopI->setFlags(FrameRegUpdateFlags);

  int64_t ExtraBaseRegUpdate =
      FrameRegUpdate ? (*FrameRegUpdate - FrameRegOffset.getBytes() - Size) : 0;
  if (LoopSize < Size) {
    assert(FrameRegUpdate);
    assert(Size - LoopSize == 16);
    // Tag 16 more bytes at BaseReg and update BaseReg.
    BuildMI(*MBB, InsertI, DL,
            TII->get(ZeroData ? AArch64::STZGPostIndex : AArch64::STGPostIndex))
        .addDef(BaseReg)
        .addReg(BaseReg)
        .addReg(BaseReg)
        .addImm(1 + ExtraBaseRegUpdate / 16)
        .setMemRefs(CombinedMemRefs)
        .setMIFlags(FrameRegUpdateFlags);
  } else if (ExtraBaseRegUpdate) {
    // Update BaseReg.
    BuildMI(
        *MBB, InsertI, DL,
        TII->get(ExtraBaseRegUpdate > 0 ? AArch64::ADDXri : AArch64::SUBXri))
        .addDef(BaseReg)
        .addReg(BaseReg)
        .addImm(std::abs(ExtraBaseRegUpdate))
        .addImm(0)
        .setMIFlags(FrameRegUpdateFlags);
  }
}

// Check if *II is a register update that can be merged into STGloop that ends
// at (Reg + Size). RemainingOffset is the required adjustment to Reg after the
// end of the loop.
bool canMergeRegUpdate(MachineBasicBlock::iterator II, unsigned Reg,
                       int64_t Size, int64_t *TotalOffset) {
  MachineInstr &MI = *II;
  if ((MI.getOpcode() == AArch64::ADDXri ||
       MI.getOpcode() == AArch64::SUBXri) &&
      MI.getOperand(0).getReg() == Reg && MI.getOperand(1).getReg() == Reg) {
    unsigned Shift = AArch64_AM::getShiftValue(MI.getOperand(3).getImm());
    int64_t Offset = MI.getOperand(2).getImm() << Shift;
    if (MI.getOpcode() == AArch64::SUBXri)
      Offset = -Offset;
    int64_t AbsPostOffset = std::abs(Offset - Size);
    const int64_t kMaxOffset =
        0xFFF; // Max encoding for unshifted ADDXri / SUBXri
    if (AbsPostOffset <= kMaxOffset && AbsPostOffset % 16 == 0) {
      *TotalOffset = Offset;
      return true;
    }
  }
  return false;
}

void mergeMemRefs(const SmallVectorImpl<TagStoreInstr> &TSE,
                  SmallVectorImpl<MachineMemOperand *> &MemRefs) {
  MemRefs.clear();
  for (auto &TS : TSE) {
    MachineInstr *MI = TS.MI;
    // An instruction without memory operands may access anything. Be
    // conservative and return an empty list.
    if (MI->memoperands_empty()) {
      MemRefs.clear();
      return;
    }
    MemRefs.append(MI->memoperands_begin(), MI->memoperands_end());
  }
}

void TagStoreEdit::emitCode(MachineBasicBlock::iterator &InsertI,
                            const AArch64FrameLowering *TFI, bool IsLast) {
  if (TagStores.empty())
    return;
  TagStoreInstr &FirstTagStore = TagStores[0];
  TagStoreInstr &LastTagStore = TagStores[TagStores.size() - 1];
  Size = LastTagStore.Offset - FirstTagStore.Offset + LastTagStore.Size;
  DL = TagStores[0].MI->getDebugLoc();

  Register Reg;
  FrameRegOffset = TFI->resolveFrameOffsetReference(
      *MF, FirstTagStore.Offset, false /*isFixed*/, false /*isSVE*/, Reg,
      /*PreferFP=*/false, /*ForSimm=*/true);
  FrameReg = Reg;
  FrameRegUpdate = None;

  mergeMemRefs(TagStores, CombinedMemRefs);

  LLVM_DEBUG(dbgs() << "Replacing adjacent STG instructions:\n";
             for (const auto &Instr
                  : TagStores) { dbgs() << "  " << *Instr.MI; });

  // Size threshold where a loop becomes shorter than a linear sequence of
  // tagging instructions.
  const int kSetTagLoopThreshold = 176;
  if (Size < kSetTagLoopThreshold) {
    if (TagStores.size() < 2)
      return;
    emitUnrolled(InsertI);
  } else {
    MachineInstr *UpdateInstr = nullptr;
    int64_t TotalOffset;
    if (IsLast) {
      // See if we can merge base register update into the STGloop.
      // This is done in AArch64LoadStoreOptimizer for "normal" stores,
      // but STGloop is way too unusual for that, and also it only
      // realistically happens in function epilogue. Also, STGloop is expanded
      // before that pass.
      if (InsertI != MBB->end() &&
          canMergeRegUpdate(InsertI, FrameReg, FrameRegOffset.getBytes() + Size,
                            &TotalOffset)) {
        UpdateInstr = &*InsertI++;
        LLVM_DEBUG(dbgs() << "Folding SP update into loop:\n  "
                          << *UpdateInstr);
      }
    }

    if (!UpdateInstr && TagStores.size() < 2)
      return;

    if (UpdateInstr) {
      FrameRegUpdate = TotalOffset;
      FrameRegUpdateFlags = UpdateInstr->getFlags();
    }
    emitLoop(InsertI);
    if (UpdateInstr)
      UpdateInstr->eraseFromParent();
  }

  for (auto &TS : TagStores)
    TS.MI->eraseFromParent();
}

bool isMergeableStackTaggingInstruction(MachineInstr &MI, int64_t &Offset,
                                        int64_t &Size, bool &ZeroData) {
  MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  unsigned Opcode = MI.getOpcode();
  ZeroData = (Opcode == AArch64::STZGloop || Opcode == AArch64::STZGOffset ||
              Opcode == AArch64::STZ2GOffset);

  if (Opcode == AArch64::STGloop || Opcode == AArch64::STZGloop) {
    if (!MI.getOperand(0).isDead() || !MI.getOperand(1).isDead())
      return false;
    if (!MI.getOperand(2).isImm() || !MI.getOperand(3).isFI())
      return false;
    Offset = MFI.getObjectOffset(MI.getOperand(3).getIndex());
    Size = MI.getOperand(2).getImm();
    return true;
  }

  if (Opcode == AArch64::STGOffset || Opcode == AArch64::STZGOffset)
    Size = 16;
  else if (Opcode == AArch64::ST2GOffset || Opcode == AArch64::STZ2GOffset)
    Size = 32;
  else
    return false;

  if (MI.getOperand(0).getReg() != AArch64::SP || !MI.getOperand(1).isFI())
    return false;

  Offset = MFI.getObjectOffset(MI.getOperand(1).getIndex()) +
           16 * MI.getOperand(2).getImm();
  return true;
}

// Detect a run of memory tagging instructions for adjacent stack frame slots,
// and replace them with a shorter instruction sequence:
// * replace STG + STG with ST2G
// * replace STGloop + STGloop with STGloop
// This code needs to run when stack slot offsets are already known, but before
// FrameIndex operands in STG instructions are eliminated.
MachineBasicBlock::iterator tryMergeAdjacentSTG(MachineBasicBlock::iterator II,
                                                const AArch64FrameLowering *TFI,
                                                RegScavenger *RS) {
  bool FirstZeroData;
  int64_t Size, Offset;
  MachineInstr &MI = *II;
  MachineBasicBlock *MBB = MI.getParent();
  MachineBasicBlock::iterator NextI = ++II;
  if (&MI == &MBB->instr_back())
    return II;
  if (!isMergeableStackTaggingInstruction(MI, Offset, Size, FirstZeroData))
    return II;

  SmallVector<TagStoreInstr, 4> Instrs;
  Instrs.emplace_back(&MI, Offset, Size);

  constexpr int kScanLimit = 10;
  int Count = 0;
  for (MachineBasicBlock::iterator E = MBB->end();
       NextI != E && Count < kScanLimit; ++NextI) {
    MachineInstr &MI = *NextI;
    bool ZeroData;
    int64_t Size, Offset;
    // Collect instructions that update memory tags with a FrameIndex operand
    // and (when applicable) constant size, and whose output registers are dead
    // (the latter is almost always the case in practice). Since these
    // instructions effectively have no inputs or outputs, we are free to skip
    // any non-aliasing instructions in between without tracking used registers.
    if (isMergeableStackTaggingInstruction(MI, Offset, Size, ZeroData)) {
      if (ZeroData != FirstZeroData)
        break;
      Instrs.emplace_back(&MI, Offset, Size);
      continue;
    }

    // Only count non-transient, non-tagging instructions toward the scan
    // limit.
    if (!MI.isTransient())
      ++Count;

    // Just in case, stop before the epilogue code starts.
    if (MI.getFlag(MachineInstr::FrameSetup) ||
        MI.getFlag(MachineInstr::FrameDestroy))
      break;

    // Reject anything that may alias the collected instructions.
    if (MI.mayLoadOrStore() || MI.hasUnmodeledSideEffects())
      break;
  }

  // New code will be inserted after the last tagging instruction we've found.
  MachineBasicBlock::iterator InsertI = Instrs.back().MI;
  InsertI++;

  llvm::stable_sort(Instrs,
                    [](const TagStoreInstr &Left, const TagStoreInstr &Right) {
                      return Left.Offset < Right.Offset;
                    });

  // Make sure that we don't have any overlapping stores.
  int64_t CurOffset = Instrs[0].Offset;
  for (auto &Instr : Instrs) {
    if (CurOffset > Instr.Offset)
      return NextI;
    CurOffset = Instr.Offset + Instr.Size;
  }

  // Find contiguous runs of tagged memory and emit shorter instruction
  // sequencies for them when possible.
  TagStoreEdit TSE(MBB, FirstZeroData);
  Optional<int64_t> EndOffset;
  for (auto &Instr : Instrs) {
    if (EndOffset && *EndOffset != Instr.Offset) {
      // Found a gap.
      TSE.emitCode(InsertI, TFI, /*IsLast = */ false);
      TSE.clear();
    }

    TSE.addInstruction(Instr);
    EndOffset = Instr.Offset + Instr.Size;
  }

  TSE.emitCode(InsertI, TFI, /*IsLast = */ true);

  return InsertI;
}
} // namespace

void AArch64FrameLowering::processFunctionBeforeFrameIndicesReplaced(
    MachineFunction &MF, RegScavenger *RS = nullptr) const {
  if (StackTaggingMergeSetTag)
    for (auto &BB : MF)
      for (MachineBasicBlock::iterator II = BB.begin(); II != BB.end();)
        II = tryMergeAdjacentSTG(II, this, RS);
}

/// For Win64 AArch64 EH, the offset to the Unwind object is from the SP
/// before the update.  This is easily retrieved as it is exactly the offset
/// that is set in processFunctionBeforeFrameFinalized.
int AArch64FrameLowering::getFrameIndexReferencePreferSP(
    const MachineFunction &MF, int FI, Register &FrameReg,
    bool IgnoreSPUpdates) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if (IgnoreSPUpdates) {
    LLVM_DEBUG(dbgs() << "Offset from the SP for " << FI << " is "
                      << MFI.getObjectOffset(FI) << "\n");
    FrameReg = AArch64::SP;
    return MFI.getObjectOffset(FI);
  }

  return getFrameIndexReference(MF, FI, FrameReg);
}

/// The parent frame offset (aka dispFrame) is only used on X86_64 to retrieve
/// the parent's frame pointer
unsigned AArch64FrameLowering::getWinEHParentFrameOffset(
    const MachineFunction &MF) const {
  return 0;
}

/// Funclets only need to account for space for the callee saved registers,
/// as the locals are accounted for in the parent's stack frame.
unsigned AArch64FrameLowering::getWinEHFuncletFrameSize(
    const MachineFunction &MF) const {
  // This is the size of the pushed CSRs.
  unsigned CSSize =
      MF.getInfo<AArch64FunctionInfo>()->getCalleeSavedStackSize();
  // This is the amount of stack a funclet needs to allocate.
  return alignTo(CSSize + MF.getFrameInfo().getMaxCallFrameSize(),
                 getStackAlign());
}
