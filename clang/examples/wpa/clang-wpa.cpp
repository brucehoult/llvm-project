//===--- clang-wpa.cpp - clang whole program analyzer ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This tool reads a sequence of precompiled AST files, and do various
// cross translation unit analyses.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Checker/PathSensitive/AnalysisManager.h"
#include "clang/Checker/PathSensitive/GRExprEngine.h"
#include "clang/Checker/PathSensitive/GRTransferFuncs.h"
#include "clang/Checker/Checkers/LocalCheckers.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Index/CallGraph.h"
#include "clang/Index/Indexer.h"
#include "clang/Index/TranslationUnit.h"
#include "clang/Index/DeclReferenceMap.h"
#include "clang/Index/SelectorMap.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;
using namespace idx;

static llvm::cl::list<std::string>
InputFilenames(llvm::cl::Positional, llvm::cl::desc("<input AST files>"));

static llvm::cl::opt<bool> 
ViewCallGraph("view-call-graph", llvm::cl::desc("Display the call graph."));

static llvm::cl::opt<std::string>
AnalyzeFunction("analyze-function", 
                llvm::cl::desc("Specify the entry function."));

namespace {
// A thin wrapper over ASTUnit implementing the TranslationUnit interface.
class ASTUnitTU : public TranslationUnit {
  ASTUnit *AST;
  DeclReferenceMap DeclRefMap;
  SelectorMap SelMap;
  
public:
  ASTUnitTU(ASTUnit *ast) 
    : AST(ast), DeclRefMap(AST->getASTContext()), SelMap(AST->getASTContext()) {
  }

  virtual ASTContext &getASTContext() {
    return AST->getASTContext();
  }
  
  virtual Preprocessor &getPreprocessor() {
    return AST->getPreprocessor();
  }

  virtual DeclReferenceMap &getDeclReferenceMap() {
    return DeclRefMap;
  }

  virtual SelectorMap &getSelectorMap() {
    return SelMap;
  }
};
}

int main(int argc, char **argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv, "clang-wpa");
  std::vector<ASTUnit*> ASTUnits;

  Program Prog;
  Indexer Idxer(Prog);

  if (InputFilenames.empty())
    return 0;

  DiagnosticOptions DiagOpts;
  llvm::IntrusiveRefCntPtr<Diagnostic> Diags
    = CompilerInstance::createDiagnostics(DiagOpts, argc, argv);
  for (unsigned i = 0, e = InputFilenames.size(); i != e; ++i) {
    const std::string &InFile = InputFilenames[i];
    llvm::OwningPtr<ASTUnit> AST(ASTUnit::LoadFromPCHFile(InFile, Diags));
    if (!AST)
      return 1;

    ASTUnits.push_back(AST.take());
  }

  if (ViewCallGraph) {
    llvm::OwningPtr<CallGraph> CG;
    CG.reset(new CallGraph(Prog));

    for (unsigned i = 0, e = ASTUnits.size(); i != e; ++i)
      CG->addTU(ASTUnits[i]->getASTContext());

    CG->ViewCallGraph();
    return 0;
  }

  if (AnalyzeFunction.empty())
    return 0;

  // Feed all ASTUnits to the Indexer.
  for (unsigned i = 0, e = ASTUnits.size(); i != e; ++i) {
    ASTUnitTU *TU = new ASTUnitTU(ASTUnits[i]);
    Idxer.IndexAST(TU);
  }

  Entity Ent = Entity::get(AnalyzeFunction, Prog);
  FunctionDecl *FD;
  TranslationUnit *TU;
  llvm::tie(FD, TU) = Idxer.getDefinitionFor(Ent);

  if (!FD)
    return 0;

  // Create an analysis engine.
  Preprocessor &PP = TU->getPreprocessor();

  // Hard code options for now.
  AnalysisManager AMgr(TU->getASTContext(), PP.getDiagnostics(),
                       PP.getLangOptions(), /* PathDiagnostic */ 0,
                       CreateRegionStoreManager,
                       CreateRangeConstraintManager,
                       /* MaxNodes */ 300000, /* MaxLoop */ 3,
                       /* VisualizeEG */ false, /* VisualizeEGUbi */ false,
                       /* PurgeDead */ true, /* EagerlyAssume */ false,
                       /* TrimGraph */ false, /* InlineCall */ true);

  GRTransferFuncs* TF = MakeCFRefCountTF(AMgr.getASTContext(), /*GC*/false,
                                         AMgr.getLangOptions());
  GRExprEngine Eng(AMgr, TF);

  Eng.ExecuteWorkList(AMgr.getStackFrame(FD), AMgr.getMaxNodes());
  
  return 0;
}
