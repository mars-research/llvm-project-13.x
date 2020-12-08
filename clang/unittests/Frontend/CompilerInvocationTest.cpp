//===- unittests/Frontend/CompilerInvocationTest.cpp - CI tests //---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/Host.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace clang;

using ::testing::Contains;
using ::testing::Each;
using ::testing::StrEq;
using ::testing::StrNe;

namespace {
class CommandLineTest : public ::testing::Test {
public:
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
  SmallVector<const char *, 32> GeneratedArgs;
  SmallVector<std::string, 32> GeneratedArgsStorage;
  CompilerInvocation CInvok;

  const char *operator()(const Twine &Arg) {
    return GeneratedArgsStorage.emplace_back(Arg.str()).c_str();
  }

  CommandLineTest()
      : Diags(CompilerInstance::createDiagnostics(new DiagnosticOptions())) {}
};

TEST_F(CommandLineTest, OptIsInitializedWithCustomDefaultValue) {
  const char *Args[] = {"clang", "-xc++"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  ASSERT_TRUE(CInvok.getFrontendOpts().UseTemporary);
}

TEST_F(CommandLineTest, OptOfNegativeFlagIsPopulatedWithFalse) {
  const char *Args[] = {"clang", "-xc++", "-fno-temp-file"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  ASSERT_FALSE(CInvok.getFrontendOpts().UseTemporary);
}

TEST_F(CommandLineTest, OptsOfImpliedPositiveFlagArePopulatedWithTrue) {
  const char *Args[] = {"clang", "-xc++", "-cl-unsafe-math-optimizations"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  // Explicitly provided flag.
  ASSERT_TRUE(CInvok.getLangOpts()->CLUnsafeMath);

  // Flags directly implied by explicitly provided flag.
  ASSERT_TRUE(CInvok.getCodeGenOpts().LessPreciseFPMAD);
  ASSERT_TRUE(CInvok.getLangOpts()->UnsafeFPMath);

  // Flag transitively implied by explicitly provided flag.
  ASSERT_TRUE(CInvok.getLangOpts()->AllowRecip);
}

TEST_F(CommandLineTest, CanGenerateCC1CommandLineFlag) {
  const char *Args[] = {"clang", "-xc++", "-fmodules-strict-context-hash", "-"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  ASSERT_THAT(GeneratedArgs, Contains(StrEq("-fmodules-strict-context-hash")));
}

TEST_F(CommandLineTest, CanGenerateCC1CommandLineSeparate) {
  const char *TripleCStr = "i686-apple-darwin9";
  const char *Args[] = {"clang", "-xc++", "-triple", TripleCStr, "-"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  ASSERT_THAT(GeneratedArgs, Contains(StrEq(TripleCStr)));
}

TEST_F(CommandLineTest,  CanGenerateCC1CommandLineSeparateRequiredPresent) {
  const std::string DefaultTriple =
      llvm::Triple::normalize(llvm::sys::getDefaultTargetTriple());
  const char *Args[] = {"clang", "-xc++", "-triple", DefaultTriple.c_str(),
                        "-"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Triple should always be emitted even if it is the default
  ASSERT_THAT(GeneratedArgs, Contains(StrEq(DefaultTriple.c_str())));
}

TEST_F(CommandLineTest, CanGenerateCC1CommandLineSeparateRequiredAbsent) {
  const std::string DefaultTriple =
      llvm::Triple::normalize(llvm::sys::getDefaultTargetTriple());
  const char *Args[] = {"clang", "-xc++", "-"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Triple should always be emitted even if it is the default
  ASSERT_THAT(GeneratedArgs, Contains(StrEq(DefaultTriple.c_str())));
}

TEST_F(CommandLineTest, CanGenerateCC1CommandLineSeparateEnum) {
  const char *RelocationModelCStr = "static";
  const char *Args[] = {"clang", "-xc++", "-mrelocation-model",
                        RelocationModelCStr, "-"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Non default relocation model
  ASSERT_THAT(GeneratedArgs, Contains(StrEq(RelocationModelCStr)));
  GeneratedArgs.clear();

  RelocationModelCStr = "pic";
  Args[3] = RelocationModelCStr;

  CompilerInvocation CInvok1;
  CompilerInvocation::CreateFromArgs(CInvok1, Args, *Diags);

  CInvok1.generateCC1CommandLine(GeneratedArgs, *this);
  ASSERT_THAT(GeneratedArgs, Each(StrNe(RelocationModelCStr)));
}

TEST_F(CommandLineTest, NotPresentNegativeFlagNotGenerated) {
  const char *Args[] = {"clang", "-xc++"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-fno-temp-file"))));
}

TEST_F(CommandLineTest, PresentNegativeFlagGenerated) {
  const char *Args[] = {"clang", "-xc++", "-fno-temp-file"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  ASSERT_THAT(GeneratedArgs, Contains(StrEq("-fno-temp-file")));
}

TEST_F(CommandLineTest, NotPresentAndNotImpliedNotGenerated) {
  const char *Args[] = {"clang", "-xc++"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Missing options are not generated.
  ASSERT_THAT(GeneratedArgs,
              Not(Contains(StrEq("-cl-unsafe-math-optimizations"))));
  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-cl-mad-enable"))));
  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-menable-unsafe-fp-math"))));
}

TEST_F(CommandLineTest, NotPresentAndImpliedNotGenerated) {
  const char *Args[] = {"clang", "-xc++", "-cl-unsafe-math-optimizations"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Missing options that were implied are not generated.
  ASSERT_THAT(GeneratedArgs, Contains(StrEq("-cl-unsafe-math-optimizations")));
  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-cl-mad-enable"))));
  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-menable-unsafe-fp-math"))));
}

TEST_F(CommandLineTest, PresentAndImpliedNotGenerated) {
  const char *Args[] = {"clang", "-xc++", "-cl-unsafe-math-optimizations",
                        "-cl-mad-enable", "-menable-unsafe-fp-math"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Present options that were also implied are not generated.
  ASSERT_THAT(GeneratedArgs, Contains(StrEq("-cl-unsafe-math-optimizations")));
  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-cl-mad-enable"))));
  ASSERT_THAT(GeneratedArgs, Not(Contains(StrEq("-menable-unsafe-fp-math"))));
}

TEST_F(CommandLineTest, PresentAndNotImpliedGenerated) {
  const char *Args[] = {"clang", "-xc++", "-cl-mad-enable",
                        "-menable-unsafe-fp-math"};

  CompilerInvocation::CreateFromArgs(CInvok, Args, *Diags);

  CInvok.generateCC1CommandLine(GeneratedArgs, *this);

  // Present options that were not implied are generated.
  ASSERT_THAT(GeneratedArgs, Contains(StrEq("-cl-mad-enable")));
  ASSERT_THAT(GeneratedArgs, Contains(StrEq("-menable-unsafe-fp-math")));
}
} // anonymous namespace
