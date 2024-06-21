#ifndef LLVM_TRANSFORMS_LOOPICM_H
#define LLVM_TRANSFORMS_LOOPICM_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {

class LoopICM : public PassInfoMixin<LoopICM> {
public:
    PreservedAnalyses run(Loop &L, LoopAnalysisManager &LAM, LoopStandardAnalysisResults &LAR, LPMUpdater &LU);
};
}

#endif 
