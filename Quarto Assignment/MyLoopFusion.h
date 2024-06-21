#ifndef LLVM_TRANSFORMS_MYLOOPFUSION_H
#define LLVM_TRANSFORMS_MYLOOPFUSION_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"

namespace llvm {

class MyLoopFusionPass : public PassInfoMixin<MyLoopFusionPass> {
   public:
      PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

}

#endif