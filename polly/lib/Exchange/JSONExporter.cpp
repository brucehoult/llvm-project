//===-- JSONExporter.cpp  - Export Scops as JSON  -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Export the Scops build by ScopInfo pass as a JSON file.
//
//===----------------------------------------------------------------------===//

#include "polly/DependenceInfo.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/ScopInfo.h"
#include "polly/ScopPass.h"
#include "polly/Support/ScopLocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "isl/constraint.h"
#include "isl/map.h"
#include "isl/printer.h"
#include "isl/set.h"
#include "isl/union_map.h"
#include "json/reader.h"
#include "json/writer.h"
#include <memory>
#include <string>
#include <system_error>

using namespace llvm;
using namespace polly;

#define DEBUG_TYPE "polly-import-jscop"

STATISTIC(NewAccessMapFound, "Number of updated access functions");

namespace {
static cl::opt<std::string>
    ImportDir("polly-import-jscop-dir",
              cl::desc("The directory to import the .jscop files from."),
              cl::Hidden, cl::value_desc("Directory path"), cl::ValueRequired,
              cl::init("."), cl::cat(PollyCategory));

static cl::opt<std::string>
    ImportPostfix("polly-import-jscop-postfix",
                  cl::desc("Postfix to append to the import .jsop files."),
                  cl::Hidden, cl::value_desc("File postfix"), cl::ValueRequired,
                  cl::init(""), cl::cat(PollyCategory));

struct JSONExporter : public ScopPass {
  static char ID;
  explicit JSONExporter() : ScopPass(ID) {}

  std::string getFileName(Scop &S) const;
  Json::Value getJSON(Scop &S) const;

  /// @brief Export the SCoP @p S to a JSON file.
  bool runOnScop(Scop &S) override;

  /// @brief Print the SCoP @p S as it is exported.
  void printScop(raw_ostream &OS, Scop &S) const override;

  /// @brief Register all analyses and transformation required.
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

struct JSONImporter : public ScopPass {
  static char ID;
  std::vector<std::string> NewAccessStrings;
  explicit JSONImporter() : ScopPass(ID) {}

  /// Import a new context from JScop.
  ///
  /// @param S The scop to update.
  /// @param JScop The JScop file describing the new schedule.
  ///
  /// @returns True if the import succeeded, otherwise False.
  bool importContext(Scop &S, Json::Value &JScop);

  /// Import a new schedule from JScop.
  ///
  /// ... and verify that the new schedule does preserve existing data
  /// dependences.
  ///
  /// @param S The scop to update.
  /// @param JScop The JScop file describing the new schedule.
  /// @param D The data dependences of the @p S.
  ///
  /// @returns True if the import succeeded, otherwise False.
  bool importSchedule(Scop &S, Json::Value &JScop, const Dependences &D);

  /// Import new memory accesses from JScop.
  ///
  /// @param S The scop to update.
  /// @param JScop The JScop file describing the new schedule.
  /// @param DL The datalayout to assume.
  ///
  /// @returns True if the import succeeded, otherwise False.
  bool importAccesses(Scop &S, Json::Value &JScop, const DataLayout &DL);

  std::string getFileName(Scop &S) const;

  /// @brief Import new access functions for SCoP @p S from a JSON file.
  bool runOnScop(Scop &S) override;

  /// @brief Print the SCoP @p S and the imported access functions.
  void printScop(raw_ostream &OS, Scop &S) const override;

  /// @brief Register all analyses and transformation required.
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // namespace

char JSONExporter::ID = 0;
std::string JSONExporter::getFileName(Scop &S) const {
  std::string FunctionName = S.getFunction().getName();
  std::string FileName = FunctionName + "___" + S.getNameStr() + ".jscop";
  return FileName;
}

void JSONExporter::printScop(raw_ostream &OS, Scop &S) const { S.print(OS); }

Json::Value JSONExporter::getJSON(Scop &S) const {
  Json::Value root;
  unsigned LineBegin, LineEnd;
  std::string FileName;

  getDebugLocation(&S.getRegion(), LineBegin, LineEnd, FileName);
  std::string Location;
  if (LineBegin != (unsigned)-1)
    Location = FileName + ":" + std::to_string(LineBegin) + "-" +
               std::to_string(LineEnd);

  root["name"] = S.getNameStr();
  root["context"] = S.getContextStr();
  if (LineBegin != (unsigned)-1)
    root["location"] = Location;
  root["statements"];

  for (ScopStmt &Stmt : S) {
    Json::Value statement;

    statement["name"] = Stmt.getBaseName();
    statement["domain"] = Stmt.getDomainStr();
    statement["schedule"] = Stmt.getScheduleStr();
    statement["accesses"];

    for (MemoryAccess *MA : Stmt) {
      Json::Value access;

      access["kind"] = MA->isRead() ? "read" : "write";
      access["relation"] = MA->getOriginalAccessRelationStr();

      statement["accesses"].append(access);
    }

    root["statements"].append(statement);
  }

  return root;
}

bool JSONExporter::runOnScop(Scop &S) {
  std::string FileName = ImportDir + "/" + getFileName(S);

  Json::Value jscop = getJSON(S);
  Json::StyledWriter writer;
  std::string fileContent = writer.write(jscop);

  // Write to file.
  std::error_code EC;
  tool_output_file F(FileName, EC, llvm::sys::fs::F_Text);

  std::string FunctionName = S.getFunction().getName();
  errs() << "Writing JScop '" << S.getNameStr() << "' in function '"
         << FunctionName << "' to '" << FileName << "'.\n";

  if (!EC) {
    F.os() << fileContent;
    F.os().close();
    if (!F.os().has_error()) {
      errs() << "\n";
      F.keep();
      return false;
    }
  }

  errs() << "  error opening file for writing!\n";
  F.os().clear_error();

  return false;
}

void JSONExporter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<ScopInfoRegionPass>();
}

Pass *polly::createJSONExporterPass() { return new JSONExporter(); }

char JSONImporter::ID = 0;
std::string JSONImporter::getFileName(Scop &S) const {
  std::string FunctionName = S.getFunction().getName();
  std::string FileName = FunctionName + "___" + S.getNameStr() + ".jscop";

  if (ImportPostfix != "")
    FileName += "." + ImportPostfix;

  return FileName;
}

void JSONImporter::printScop(raw_ostream &OS, Scop &S) const {
  S.print(OS);
  for (std::vector<std::string>::const_iterator I = NewAccessStrings.begin(),
                                                E = NewAccessStrings.end();
       I != E; I++)
    OS << "New access function '" << *I << "'detected in JSCOP file\n";
}

typedef Dependences::StatementToIslMapTy StatementToIslMapTy;

bool JSONImporter::importContext(Scop &S, Json::Value &JScop) {
  isl_set *OldContext = S.getContext();
  isl_set *NewContext =
      isl_set_read_from_str(S.getIslCtx(), JScop["context"].asCString());

  for (unsigned i = 0; i < isl_set_dim(OldContext, isl_dim_param); i++) {
    isl_id *Id = isl_set_get_dim_id(OldContext, isl_dim_param, i);
    NewContext = isl_set_set_dim_id(NewContext, isl_dim_param, i, Id);
  }

  isl_set_free(OldContext);
  S.setContext(NewContext);
  return true;
}

bool JSONImporter::importSchedule(Scop &S, Json::Value &JScop,
                                  const Dependences &D) {
  StatementToIslMapTy NewSchedule;

  int Index = 0;
  for (ScopStmt &Stmt : S) {
    Json::Value Schedule = JScop["statements"][Index]["schedule"];
    isl_map *Map = isl_map_read_from_str(S.getIslCtx(), Schedule.asCString());
    isl_space *Space = Stmt.getDomainSpace();

    // Copy the old tuple id. This is necessary to retain the user pointer,
    // that stores the reference to the ScopStmt this schedule belongs to.
    Map = isl_map_set_tuple_id(Map, isl_dim_in,
                               isl_space_get_tuple_id(Space, isl_dim_set));
    for (unsigned i = 0; i < isl_space_dim(Space, isl_dim_param); i++) {
      isl_id *Id = isl_space_get_dim_id(Space, isl_dim_param, i);
      Map = isl_map_set_dim_id(Map, isl_dim_param, i, Id);
    }
    isl_space_free(Space);
    NewSchedule[&Stmt] = Map;
    Index++;
  }

  if (!D.isValidSchedule(S, &NewSchedule)) {
    errs() << "JScop file contains a schedule that changes the "
           << "dependences. Use -disable-polly-legality to continue anyways\n";
    for (auto Element : NewSchedule)
      isl_map_free(Element.second);
    return false;
  }

  auto ScheduleMap = isl_union_map_empty(S.getParamSpace());
  for (ScopStmt &Stmt : S) {
    if (NewSchedule.find(&Stmt) != NewSchedule.end())
      ScheduleMap = isl_union_map_add_map(ScheduleMap, NewSchedule[&Stmt]);
    else
      ScheduleMap = isl_union_map_add_map(ScheduleMap, Stmt.getSchedule());
  }

  S.setSchedule(ScheduleMap);

  return true;
}

bool JSONImporter::importAccesses(Scop &S, Json::Value &JScop,
                                  const DataLayout &DL) {
  int StatementIdx = 0;
  for (ScopStmt &Stmt : S) {
    int MemoryAccessIdx = 0;
    for (MemoryAccess *MA : Stmt) {
      Json::Value Accesses = JScop["statements"][StatementIdx]["accesses"]
                                  [MemoryAccessIdx]["relation"];
      isl_map *NewAccessMap =
          isl_map_read_from_str(S.getIslCtx(), Accesses.asCString());
      isl_map *CurrentAccessMap = MA->getAccessRelation();

      if (isl_map_dim(NewAccessMap, isl_dim_param) !=
          isl_map_dim(CurrentAccessMap, isl_dim_param)) {
        errs() << "JScop file changes the number of parameter dimensions\n";
        isl_map_free(CurrentAccessMap);
        isl_map_free(NewAccessMap);
        return false;
      }

      isl_id *OutId = isl_map_get_tuple_id(CurrentAccessMap, isl_dim_out);
      NewAccessMap = isl_map_set_tuple_id(NewAccessMap, isl_dim_out, OutId);

      if (MA->isArrayKind()) {
        // We keep the old alignment, thus we cannot allow accesses to memory
        // locations that were not accessed before if the alignment of the
        // access is not the default alignment.
        bool SpecialAlignment = true;
        if (LoadInst *LoadI = dyn_cast<LoadInst>(MA->getAccessInstruction())) {
          SpecialAlignment =
              DL.getABITypeAlignment(LoadI->getType()) != LoadI->getAlignment();
        } else if (StoreInst *StoreI =
                       dyn_cast<StoreInst>(MA->getAccessInstruction())) {
          SpecialAlignment =
              DL.getABITypeAlignment(StoreI->getValueOperand()->getType()) !=
              StoreI->getAlignment();
        }

        if (SpecialAlignment) {
          isl_set *NewAccessSet = isl_map_range(isl_map_copy(NewAccessMap));
          isl_set *CurrentAccessSet =
              isl_map_range(isl_map_copy(CurrentAccessMap));
          bool IsSubset = isl_set_is_subset(NewAccessSet, CurrentAccessSet);
          isl_set_free(NewAccessSet);
          isl_set_free(CurrentAccessSet);

          if (!IsSubset) {
            errs() << "JScop file changes the accessed memory\n";
            isl_map_free(CurrentAccessMap);
            isl_map_free(NewAccessMap);
            return false;
          }
        }
      }

      // We need to copy the isl_ids for the parameter dimensions to the new
      // map. Without doing this the current map would have different
      // ids then the new one, even though both are named identically.
      for (unsigned i = 0; i < isl_map_dim(CurrentAccessMap, isl_dim_param);
           i++) {
        isl_id *Id = isl_map_get_dim_id(CurrentAccessMap, isl_dim_param, i);
        NewAccessMap = isl_map_set_dim_id(NewAccessMap, isl_dim_param, i, Id);
      }

      // Copy the old tuple id. This is necessary to retain the user pointer,
      // that stores the reference to the ScopStmt this access belongs to.
      isl_id *Id = isl_map_get_tuple_id(CurrentAccessMap, isl_dim_in);
      NewAccessMap = isl_map_set_tuple_id(NewAccessMap, isl_dim_in, Id);

      if (!isl_map_has_equal_space(CurrentAccessMap, NewAccessMap)) {
        errs() << "JScop file contains access function with incompatible "
               << "dimensions\n";
        isl_map_free(CurrentAccessMap);
        isl_map_free(NewAccessMap);
        return false;
      }

      auto NewAccessDomain = isl_map_domain(isl_map_copy(NewAccessMap));
      auto CurrentAccessDomain = isl_map_domain(isl_map_copy(CurrentAccessMap));

      NewAccessDomain =
          isl_set_intersect_params(NewAccessDomain, S.getContext());
      CurrentAccessDomain =
          isl_set_intersect_params(CurrentAccessDomain, S.getContext());

      if (isl_set_is_subset(CurrentAccessDomain, NewAccessDomain) ==
          isl_bool_false) {
        errs() << "Mapping not defined for all iteration domain elements\n";
        isl_set_free(CurrentAccessDomain);
        isl_set_free(NewAccessDomain);
        isl_map_free(CurrentAccessMap);
        isl_map_free(NewAccessMap);
        return false;
      }

      isl_set_free(CurrentAccessDomain);
      isl_set_free(NewAccessDomain);

      if (!isl_map_is_equal(NewAccessMap, CurrentAccessMap)) {
        // Statistics.
        ++NewAccessMapFound;
        NewAccessStrings.push_back(Accesses.asCString());
        MA->setNewAccessRelation(NewAccessMap);
      } else {
        isl_map_free(NewAccessMap);
      }
      isl_map_free(CurrentAccessMap);
      MemoryAccessIdx++;
    }
    StatementIdx++;
  }

  return true;
}

bool JSONImporter::runOnScop(Scop &S) {
  const Dependences &D =
      getAnalysis<DependenceInfo>().getDependences(Dependences::AL_Statement);
  const DataLayout &DL = S.getFunction().getParent()->getDataLayout();

  std::string FileName = ImportDir + "/" + getFileName(S);

  std::string FunctionName = S.getFunction().getName();
  errs() << "Reading JScop '" << S.getNameStr() << "' in function '"
         << FunctionName << "' from '" << FileName << "'.\n";
  ErrorOr<std::unique_ptr<MemoryBuffer>> result =
      MemoryBuffer::getFile(FileName);
  std::error_code ec = result.getError();

  if (ec) {
    errs() << "File could not be read: " << ec.message() << "\n";
    return false;
  }

  Json::Reader reader;
  Json::Value jscop;

  bool parsingSuccessful = reader.parse(result.get()->getBufferStart(), jscop);

  if (!parsingSuccessful) {
    errs() << "JSCoP file could not be parsed\n";
    return false;
  }

  bool Success = importContext(S, jscop);

  if (!Success)
    return false;

  Success = importSchedule(S, jscop, D);

  if (!Success)
    return false;

  Success = importAccesses(S, jscop, DL);

  if (!Success)
    return false;

  return false;
}

void JSONImporter::getAnalysisUsage(AnalysisUsage &AU) const {
  ScopPass::getAnalysisUsage(AU);
  AU.addRequired<DependenceInfo>();
}

Pass *polly::createJSONImporterPass() { return new JSONImporter(); }

INITIALIZE_PASS_BEGIN(JSONExporter, "polly-export-jscop",
                      "Polly - Export Scops as JSON"
                      " (Writes a .jscop file for each Scop)",
                      false, false);
INITIALIZE_PASS_DEPENDENCY(DependenceInfo)
INITIALIZE_PASS_END(JSONExporter, "polly-export-jscop",
                    "Polly - Export Scops as JSON"
                    " (Writes a .jscop file for each Scop)",
                    false, false)

INITIALIZE_PASS_BEGIN(JSONImporter, "polly-import-jscop",
                      "Polly - Import Scops from JSON"
                      " (Reads a .jscop file for each Scop)",
                      false, false);
INITIALIZE_PASS_DEPENDENCY(DependenceInfo)
INITIALIZE_PASS_END(JSONImporter, "polly-import-jscop",
                    "Polly - Import Scops from JSON"
                    " (Reads a .jscop file for each Scop)",
                    false, false)
