#include "llvm/Transforms/Utils/LoopICM.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/STLExtras.h"
#include <unordered_set>

using namespace llvm;

std::unordered_set<Instruction*> loopInvariantInstructionSet{};
std::vector<Instruction*> loopInvariantInstructionVector{};

SmallVector<BasicBlock*> exitBlocks{};
std::unordered_set<BasicBlock*> outBlocks{}; // successori fuori dal loop

std::unordered_set<Instruction*> InstructionsToDelete{};

void printStats(Loop &L) {
   outs() << "The loop '" << L.getName() << "' is ";

   if (L.isLoopSimplifyForm())   outs() << "in simply form\n";
   else  outs() << "not in simply form\n";

   if (L.getLoopPreheader())  outs() << "\nPreheader:" << *L.getLoopPreheader() << "\n";

   if (L.getHeader())   outs() << "Header:" << *L.getHeader() << "\n";

   for (BasicBlock *BB : L.blocks()) {
      outs() << "\nThe block '" << BB -> getName() << "' in loop '" << L.getName() << "' has " << BB -> size() << " instructions\n";
      
      for (Instruction &I : *BB)  
         outs() << I << "\n";
   }
}

bool isLoopInvariantOperand(Use &O, Loop &L) { 
   if (dyn_cast<Constant>(O) || dyn_cast<Argument>(O))   return true;

   Instruction *op = dyn_cast<Instruction>(O);

   if (op && op -> getOpcode() != Instruction::PHI) {
      if (loopInvariantInstructionSet.count(op))   return true;
      if (!L.contains(op)) return true;
   }

   return false;
}

bool isLoopInvariantInstruction(Instruction &I, Loop &L) {
   if (I.getOpcode() == Instruction::PHI) return false;

   for (Use &O : I.operands())
      if (!isLoopInvariantOperand(O, L))  return false;

   return true;
}

bool isDeadInstruction(Instruction *I) {
   for (Value *U : I -> users())
      if (is_contained(outBlocks, dyn_cast<Instruction>(U) -> getParent()))  return false;

   return true;
}

bool codeMotionCheck(Instruction *I, DominatorTree &DT) {
   if (isDeadInstruction(I))  return true;

   for (BasicBlock *BB : outBlocks)
      if (!DT.dominates(I, BB))  return false;

   return true;
}

void printInfo() {
   outs() << "Loop invariant instructions\n";

   for (Instruction *inst : loopInvariantInstructionSet) 
      outs() << *inst << "\n";

   outs() << "Instructions to delete\n";

   for (Instruction *inst : InstructionsToDelete) 
      outs() << *inst << "\n";
}

void setOutBlocks() {
   for (BasicBlock *exitBlock : exitBlocks) 
      for (BasicBlock *successor : successors(exitBlock))
         outBlocks.insert(successor);
}

// Eventuali rimozioni o spostamenti
void action(Loop &L) {
   for (Instruction *inst : InstructionsToDelete) 
      inst -> eraseFromParent();

   Instruction *T = L.getLoopPreheader() -> getTerminator();

   for (Instruction *I : loopInvariantInstructionVector) 
      I -> moveBefore(T);
}

PreservedAnalyses LoopICM::run(Loop &L, LoopAnalysisManager &LAM, LoopStandardAnalysisResults &LAR, LPMUpdater &LU) {
   // printStats(L);

   // Recupero dell'albero di dominanza
   DominatorTree &DT = LAR.DT;

   // Recupero dei blocchi di uscita
   L.getExitBlocks(exitBlocks);
   setOutBlocks();

   for (BasicBlock *BB : L.blocks()) {
      for (Instruction &I : *BB) { 
         if (I.isBinaryOp() && isLoopInvariantInstruction(I, L)) {     
            // Se l'istruzione non viene usata, la elimino
            if (std::distance(I.user_begin(), I.user_end()) == 0) {
               InstructionsToDelete.insert(&I);          
               continue;
            }    
            
            // Se l'istruzione è loop invariant e movable, la si salva
            if (codeMotionCheck(&I, DT)) {
               loopInvariantInstructionSet.insert(&I);
               loopInvariantInstructionVector.push_back(&I);
            }
         }
      }
   }

   if (std::size(loopInvariantInstructionVector) > 0 || std::size(InstructionsToDelete) > 0) {
      // printInfo();
      action(L);
      return PreservedAnalyses::none();
   }
  
  return PreservedAnalyses::all();
}
