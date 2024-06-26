#include "llvm/Transforms/Utils/MyLoopFusion.h"

using namespace llvm;

bool isAdjacent(Loop *l0, Loop *l1) {
   if (l0->isGuarded() && l1->isGuarded())
      return (l0->getExitBlock()->getSingleSuccessor()==l1->getLoopGuardBranch()->getParent());
   else if (!l0->isGuarded() && !l1->isGuarded())
      return (l0->getExitBlock()==l1->getLoopPreheader());
   return false;
}

bool isSameIterations(Loop *l0, Loop *l1, ScalarEvolution &SE) {
   if (l0->isGuarded())
      if (Instruction *L0CmpInst = dyn_cast<Instruction>(l0->getLoopGuardBranch()->getCondition()))
         if (Instruction *L1CmpInst = dyn_cast<Instruction>(l1->getLoopGuardBranch()->getCondition()))
               if (!L0CmpInst->isIdenticalTo(L1CmpInst))
                  return false;
   
   const SCEV *trip_count0 = SE.getBackedgeTakenCount(l0);
   const SCEV *trip_count1 = SE.getBackedgeTakenCount(l1);

   if (trip_count0->getSCEVType()==SCEVCouldNotCompute().getSCEVType() || trip_count1->getSCEVType()==SCEVCouldNotCompute().getSCEVType())   return false;

   return (trip_count0 == trip_count1);
}

bool isCFEq(Loop *l0, Loop *l1, DominatorTree &DT, PostDominatorTree &PDT) {
   if (l0->isGuarded())
      return (DT.dominates(l0->getLoopGuardBranch()->getParent(), l1->getLoopGuardBranch()->getParent()) &&
      PDT.dominates(l1->getLoopGuardBranch()->getParent(), l0->getLoopGuardBranch()->getParent()));
   else
      return (DT.dominates(l0->getHeader(), l1->getHeader()) &&
      PDT.dominates(l1->getHeader(), l0->getHeader()));
}

bool isNotNegDep(Loop *l0, Loop *l1) {
   for (BasicBlock *BB0: l0->getBlocks())
      for (Instruction &I0: *BB0)
         if (I0.getOpcode() == Instruction::Store)
            for (BasicBlock *BB1: l1->getBlocks())
               for (Instruction &I1: *BB1)
                  if (I1.getOpcode() == Instruction::Load) {
                     GetElementPtrInst *GEP0 = dyn_cast<GetElementPtrInst>(I0.getOperand(1));
                     GetElementPtrInst *GEP1 = dyn_cast<GetElementPtrInst>(I1.getOperand(0));
                     // Controllo dei puntatori agli array
                     if (GEP0->getPointerOperand() == GEP1->getPointerOperand()) {
                        // Valutazione degli indici delle load e delle store 
                        // Se sono PHINode rispettano il flusso del for
                        SExtInst *S0 = dyn_cast<SExtInst>(GEP0->getOperand(1));
                        SExtInst *S1 = dyn_cast<SExtInst>(GEP1->getOperand(1));
                        PHINode *PN0 = dyn_cast<PHINode>(S0->getOperand(0));
                        PHINode *PN1 = dyn_cast<PHINode>(S1->getOperand(0));
                        if (!PN0)
                           return false;
                        if (!PN1) {
                           Instruction *inst = dyn_cast<Instruction>(S1->getOperand(0));
                           if (!inst || inst->getOpcode() != Instruction::Sub)
                              return false;
                        }
                     }
                  }
   return true;
}

void loopFusion(Loop *l0, Loop *l1) {
   PHINode *IV0 = l0->getCanonicalInductionVariable();
   PHINode *IV1 = l1->getCanonicalInductionVariable();
   IV1->replaceAllUsesWith(IV0);

   BasicBlock *header0 = l0->getHeader();
   BasicBlock *header1 = l1->getHeader();
   BasicBlock *latch0 = l0->getLoopLatch();
   BasicBlock *latch1 = l1->getLoopLatch();
   BasicBlock *exit = l1->getUniqueExitBlock();
   
   if (!l0->isGuarded()) {
      // Modify CFG as follows:
      // header 0 --> L1 exit
      // body 0 --> body 1
      // body 1 --> latch 0
      // header 1 --> latch 1
      BasicBlock* lastL0BodyBB = l0->getBlocks().drop_back(1).back();

      // Attach body 1 to body 0
      lastL0BodyBB->getTerminator()->setSuccessor(0, l1->getBlocks().drop_front(1).drop_back(1).front());

      // Attach latch 0 to body 1
      l1->getBlocks().drop_front(1).drop_back(1).back()->getTerminator()->setSuccessor(0, latch0);

      // Attach header 1 to latch 1
      BranchInst::Create(latch1, header1->getTerminator());
      header1->getTerminator()->eraseFromParent();

      // Attach header 0 to L1 exit
      BranchInst::Create(l0->getBlocks().drop_front(1).front(), exit, header0->back().getOperand(0), header0->getTerminator());
      header0->getTerminator()->eraseFromParent();
   } else {
      // guard0 --> L1 exit
      // latch0 --> L1 exit
      // header0 --> header1
      // header1 --> latch0

      BasicBlock *guard0 = l0->getLoopGuardBranch()->getParent();
      BasicBlock *guard1 = l1->getLoopGuardBranch()->getParent();
      BasicBlock *preheader1 = l1->getLoopPreheader();

      // Attach guard 0 to L1 exit
      BranchInst::Create(l0->getLoopPreheader(), exit->getSingleSuccessor(), guard0->back().getOperand(0), guard0->getTerminator());
      guard0->getTerminator()->eraseFromParent();

      // Attach latch 0 to L1 exit
      BranchInst::Create(l0->getBlocks().front(), exit, latch0->back().getOperand(0), latch0->getTerminator());
      latch0->getTerminator()->eraseFromParent();

      // Attach header 0 to header 1
      l0->getBlocks().drop_back(1).back()->getTerminator()->setSuccessor(0, l1->getBlocks().front());

      // Attach header 1 to latch 0
      l1->getBlocks().drop_back(1).back()->getTerminator()->setSuccessor(0, latch0);

      // Remove header 1 PHI node
      header1->front().eraseFromParent();

      // Attach preheader 1 to latch 1
      BranchInst::Create(latch1, preheader1->getTerminator());
      preheader1->getTerminator()->eraseFromParent();

      // Attach latch 1 to guard 1
      BranchInst::Create(guard1, latch1->getTerminator());
      latch1->getTerminator()->eraseFromParent();

      // Attachh guard 1 to preheader 1
      BranchInst::Create(preheader1, guard1->getTerminator());
      guard1->getTerminator()->eraseFromParent();
   }
}

PreservedAnalyses MyLoopFusionPass::run(Function &F, FunctionAnalysisManager &AM) {
   DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
   PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
   LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
   ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
   bool modified = false;

   for (Loop *L0 : LI)
      for (Loop *L1 : LI)
         if (L0 != L1)
            if (isAdjacent(L0, L1) && isSameIterations(L0, L1, SE) && isCFEq(L0, L1, DT, PDT) && isNotNegDep(L0, L1)) {
               loopFusion(L0, L1);
               modified = true;
            }

   if (modified) return PreservedAnalyses::none();
   return PreservedAnalyses::all();
}