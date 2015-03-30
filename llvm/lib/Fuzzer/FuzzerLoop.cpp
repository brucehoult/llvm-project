//===- FuzzerLoop.cpp - Fuzzer's main loop --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Fuzzer's main loop.
//===----------------------------------------------------------------------===//

#include "FuzzerInternal.h"
#include <sanitizer/coverage_interface.h>
#include <algorithm>
#include <iostream>

namespace fuzzer {

// static
Unit Fuzzer::CurrentUnit;
system_clock::time_point Fuzzer::UnitStartTime;

void Fuzzer::SetDeathCallback() {
  __sanitizer_set_death_callback(DeathCallback);
}

void Fuzzer::DeathCallback() {
  std::cerr << "DEATH: " <<  std::endl;
  Print(CurrentUnit, "\n");
  PrintASCII(CurrentUnit, "\n");
  WriteToCrash(CurrentUnit, "crash-");
}

void Fuzzer::AlarmCallback() {
  size_t Seconds =
      duration_cast<seconds>(system_clock::now() - UnitStartTime).count();
  std::cerr << "ALARM: working on the last Unit for " << Seconds << " seconds"
            << std::endl;
  if (Seconds >= 3) {
    Print(CurrentUnit, "\n");
    PrintASCII(CurrentUnit, "\n");
    WriteToCrash(CurrentUnit, "timeout-");
  }
  exit(1);
}

void Fuzzer::PrintStats(const char *Where, size_t Cov, const char *End) {
  if (!Options.Verbosity) return;
  size_t Seconds = secondsSinceProcessStartUp();
  size_t ExecPerSec = (Seconds ? TotalNumberOfRuns / Seconds : 0);
  std::cerr
      << "#" << TotalNumberOfRuns
      << "\t" << Where
      << " cov " << Cov
      << " bits " << TotalBits()
      << " units " << Corpus.size()
      << " exec/s " << ExecPerSec
      << End;
}

void Fuzzer::ShuffleAndMinimize() {
  size_t MaxCov = 0;
  bool PreferSmall =
      (Options.PreferSmallDuringInitialShuffle == 1 ||
       (Options.PreferSmallDuringInitialShuffle == -1 && rand() % 2));
  if (Options.Verbosity)
    std::cerr << "PreferSmall: " << PreferSmall << "\n";
  PrintStats("READ  ", 0);
  std::vector<Unit> NewCorpus;
  std::random_shuffle(Corpus.begin(), Corpus.end());
  if (PreferSmall)
    std::stable_sort(
        Corpus.begin(), Corpus.end(),
        [](const Unit &A, const Unit &B) { return A.size() < B.size(); });
  Unit &U = CurrentUnit;
  for (const auto &C : Corpus) {
    for (size_t First = 0; First < 1; First++) {
      U.clear();
      size_t Last = std::min(First + Options.MaxLen, C.size());
      U.insert(U.begin(), C.begin() + First, C.begin() + Last);
      size_t NewCoverage = RunOne(U);
      if (NewCoverage) {
        MaxCov = NewCoverage;
        NewCorpus.push_back(U);
        if (Options.Verbosity >= 2)
          std::cerr << "NEW0: " << NewCoverage
                    << " L " << U.size()
                    << "\n";
      }
    }
  }
  Corpus = NewCorpus;
  PrintStats("INITED", MaxCov);
}

size_t Fuzzer::RunOne(const Unit &U) {
  UnitStartTime = system_clock::now();
  TotalNumberOfRuns++;
  size_t Res = 0;
  if (Options.UseFullCoverageSet)
    Res = RunOneMaximizeFullCoverageSet(U);
  else if (Options.UseCoveragePairs)
    Res = RunOneMaximizeCoveragePairs(U);
  else
    Res = RunOneMaximizeTotalCoverage(U);
  auto UnitStopTime = system_clock::now();
  auto TimeOfUnit =
      duration_cast<seconds>(UnitStopTime - UnitStartTime).count();
  if (TimeOfUnit > TimeOfLongestUnitInSeconds) {
    TimeOfLongestUnitInSeconds = TimeOfUnit;
    std::cerr << "Longest unit: " << TimeOfLongestUnitInSeconds
              << " s:\n";
    Print(U, "\n");
  }
  return Res;
}

static uintptr_t HashOfArrayOfPCs(uintptr_t *PCs, uintptr_t NumPCs) {
  uintptr_t Res = 0;
  for (uintptr_t i = 0; i < NumPCs; i++) {
    Res = (Res + PCs[i]) * 7;
  }
  return Res;
}

// Experimental. Does not yet scale.
// Fuly reset the current coverage state, run a single unit,
// collect all coverage pairs and return non-zero if a new pair is observed.
size_t Fuzzer::RunOneMaximizeCoveragePairs(const Unit &U) {
  __sanitizer_reset_coverage();
  Callback(U.data(), U.size());
  uintptr_t *PCs;
  uintptr_t NumPCs = __sanitizer_get_coverage_guards(&PCs);
  bool HasNewPairs = false;
  for (uintptr_t i = 0; i < NumPCs; i++) {
    if (!PCs[i]) continue;
    for (uintptr_t j = 0; j < NumPCs; j++) {
      if (!PCs[j]) continue;
      uint64_t Pair = (i << 32) | j;
      HasNewPairs |= CoveragePairs.insert(Pair).second;
    }
  }
  if (HasNewPairs)
    return CoveragePairs.size();
  return 0;
}

// Experimental.
// Fuly reset the current coverage state, run a single unit,
// compute a hash function from the full coverage set,
// return non-zero if the hash value is new.
// This produces tons of new units and as is it's only suitable for small tests,
// e.g. test/FullCoverageSetTest.cpp. FIXME: make it scale.
size_t Fuzzer::RunOneMaximizeFullCoverageSet(const Unit &U) {
  __sanitizer_reset_coverage();
  Callback(U.data(), U.size());
  uintptr_t *PCs;
  uintptr_t NumPCs =__sanitizer_get_coverage_guards(&PCs);
  if (FullCoverageSets.insert(HashOfArrayOfPCs(PCs, NumPCs)).second)
    return FullCoverageSets.size();
  return 0;
}

size_t Fuzzer::RunOneMaximizeTotalCoverage(const Unit &U) {
  size_t NumCounters = __sanitizer_get_number_of_counters();
  if (Options.UseCounters) {
    CounterBitmap.resize(NumCounters);
    __sanitizer_update_counter_bitset_and_clear_counters(0);
  }
  size_t OldCoverage = __sanitizer_get_total_unique_coverage();
  Callback(U.data(), U.size());
  size_t NewCoverage = __sanitizer_get_total_unique_coverage();
  size_t NumNewBits = 0;
  if (Options.UseCounters)
    NumNewBits = __sanitizer_update_counter_bitset_and_clear_counters(
        CounterBitmap.data());

  if (!(TotalNumberOfRuns & (TotalNumberOfRuns - 1)) && Options.Verbosity)
    PrintStats("pulse ", NewCoverage);

  if (NewCoverage > OldCoverage || NumNewBits)
    return NewCoverage;
  return 0;
}

void Fuzzer::WriteToOutputCorpus(const Unit &U) {
  if (Options.OutputCorpus.empty()) return;
  std::string Path = DirPlusFile(Options.OutputCorpus, Hash(U));
  WriteToFile(U, Path);
  if (Options.Verbosity >= 2)
    std::cerr << "Written to " << Path << std::endl;
}

void Fuzzer::WriteToCrash(const Unit &U, const char *Prefix) {
  std::string Path = Prefix + Hash(U);
  WriteToFile(U, Path);
  std::cerr << "CRASHED; file written to " << Path << std::endl;
}

void Fuzzer::SaveCorpus() {
  if (Options.OutputCorpus.empty()) return;
  for (const auto &U : Corpus)
    WriteToFile(U, DirPlusFile(Options.OutputCorpus, Hash(U)));
  if (Options.Verbosity)
    std::cerr << "Written corpus of " << Corpus.size() << " files to "
              << Options.OutputCorpus << "\n";
}

size_t Fuzzer::MutateAndTestOne(Unit *U) {
  size_t NewUnits = 0;
  for (int i = 0; i < Options.MutateDepth; i++) {
    if (TotalNumberOfRuns >= Options.MaxNumberOfRuns)
      return NewUnits;
    MutateWithDFSan(U);
    Mutate(U, Options.MaxLen);
    size_t NewCoverage = RunOne(*U);
    if (NewCoverage) {
      Corpus.push_back(*U);
      NewUnits++;
      PrintStats("NEW   ", NewCoverage, "");
      if (Options.Verbosity) {
        std::cerr << " L: " << U->size();
        if (U->size() < 30) {
          std::cerr << " ";
          PrintASCII(*U);
          std::cerr << "\t";
          Print(*U);
        }
        std::cerr << "\n";
      }
      WriteToOutputCorpus(*U);
      if (Options.ExitOnFirst)
        exit(0);
    }
  }
  return NewUnits;
}

size_t Fuzzer::Loop(size_t NumIterations) {
  size_t NewUnits = 0;
  for (size_t i = 1; i <= NumIterations; i++) {
    for (size_t J1 = 0; J1 < Corpus.size(); J1++) {
      if (TotalNumberOfRuns >= Options.MaxNumberOfRuns)
        return NewUnits;
      // First, simply mutate the unit w/o doing crosses.
      CurrentUnit = Corpus[J1];
      NewUnits += MutateAndTestOne(&CurrentUnit);
      // Now, cross with others.
      if (Options.DoCrossOver) {
        for (size_t J2 = 0; J2 < Corpus.size(); J2++) {
          CurrentUnit.clear();
          CrossOver(Corpus[J1], Corpus[J2], &CurrentUnit, Options.MaxLen);
          NewUnits += MutateAndTestOne(&CurrentUnit);
        }
      }
    }
  }
  return NewUnits;
}

}  // namespace fuzzer
