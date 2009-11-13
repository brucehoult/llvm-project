//===--- FrontendOptions.h --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_FRONTENDOPTIONS_H
#define LLVM_CLANG_FRONTEND_FRONTENDOPTIONS_H

#include "clang/Frontend/CommandLineSourceLoc.h"
#include <string>
#include <vector>

namespace clang {

/// FrontendOptions - Options for controlling the behavior of the frontend.
class FrontendOptions {
public:
  unsigned DebugCodeCompletionPrinter : 1; ///< Use the debug printer for code
                                           /// completion results.
  unsigned DisableFree : 1;                ///< Disable memory freeing on exit.
  unsigned EmptyInputOnly : 1;             ///< Force input files to be treated
                                           /// as if they were empty, for timing
                                           /// the frontend startup.
  unsigned FixItAll : 1;                   ///< Apply FIX-IT advice to the input
                                           /// source files.
  unsigned RelocatablePCH : 1;             ///< When generating PCH files,
                                           /// instruct the PCH writer to create
                                           /// relocatable PCH files.
  unsigned ShowMacrosInCodeCompletion : 1; ///< Show macros in code completion
                                           /// results.
  unsigned ShowStats : 1;                  ///< Show frontend performance
                                           /// metrics and statistics.
  unsigned ShowTimers : 1;                 ///< Show timers for individual
                                           /// actions.

  /// If given, the name of the target triple to compile for. If not given the
  /// target will be selected to match the host.
  std::string TargetTriple;

  /// If given, the name of the target ABI to use.
  std::string TargetABI;

  /// The input files.
  std::vector<std::string> InputFilenames;

  /// The output file, if any.
  std::string OutputFile;

  /// If given, the name for a C++ class to view the inheritance of.
  std::string ViewClassInheritance;

  /// A list of locations to apply fix-its at.
  std::vector<ParsedSourceLocation> FixItLocations;

  /// If given, enable code completion at the provided location.
  ParsedSourceLocation CodeCompletionAt;

public:
  FrontendOptions() {
    DebugCodeCompletionPrinter = 0;
    DisableFree = 0;
    EmptyInputOnly = 0;
    FixItAll = 0;
    RelocatablePCH = 0;
    ShowMacrosInCodeCompletion = 0;
    ShowStats = 0;
    ShowTimers = 0;
  }
};

}  // end namespace clang

#endif
