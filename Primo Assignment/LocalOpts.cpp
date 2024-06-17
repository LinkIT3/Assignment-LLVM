//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

//Addition 

bool addBy0 (BasicBlock::iterator Iter) {
   BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
   if (not binIter) return false;

   ConstantInt *ci = dyn_cast<ConstantInt>(Iter -> getOperand(0));
   Value *Other = binIter -> getOperand(1);

   if (not ci or not ci -> isZero()) {
      ci = dyn_cast<ConstantInt>(Iter -> getOperand(1));
      
      if (not ci or not ci -> isZero()) return false;

      Other = binIter -> getOperand(0);
   }

   binIter -> replaceAllUsesWith(Other);

   return true;
}


//Subtraction 

bool subBy0 (BasicBlock::iterator Iter) {
   BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
   if (not binIter) return false;

   ConstantInt *ci = dyn_cast<ConstantInt>(Iter -> getOperand(1));
   Value *Other = binIter -> getOperand(0);

   if (not ci or not ci -> isZero()) return false;

   binIter -> replaceAllUsesWith(Other);

   return true;
}


// Multiplication

bool mulByPowOf2 (BinaryOperator *binIter, ConstantInt *ci, Value *Other) {
   Constant *val = ConstantInt::get(ci -> getType(), ci -> getValue().exactLogBase2());
   Instruction *NewInst = BinaryOperator::Create(Instruction::Shl, Other, val);
   NewInst -> insertAfter(binIter);
   binIter -> replaceAllUsesWith(NewInst);
   return true;
}


bool mulToShift (BinaryOperator *binIter, ConstantInt *ci, Value *Other) {
   unsigned near = ci -> getValue().nearestLogBase2();
   APInt diff = (ci -> getValue()) - (1 << near);
   unsigned log = diff.abs().logBase2();

   Instruction *NewShlInst, *NewInst;
   Instruction::BinaryOps Operation;

   if(diff.slt(0))
      Operation = Instruction::Sub;
   else
      Operation = Instruction::Add;

   if (log == 0) {
      NewShlInst = BinaryOperator::Create(Instruction::Shl, Other, ConstantInt::get(ci -> getType(), near));
      NewShlInst -> insertAfter(binIter);
      NewInst = BinaryOperator::Create(Operation, NewShlInst, Other);
      NewInst -> insertAfter(NewShlInst);
      binIter -> replaceAllUsesWith(NewInst);
      return true; 
   }

   return false;
}


bool mulBy1 (BinaryOperator *binIter, Value *Other) {
   binIter -> replaceAllUsesWith(Other);
   return true;
}


bool zeroMul (BinaryOperator *binIter, ConstantInt *ci) {
   unsigned zero = 0;
   binIter -> replaceAllUsesWith(ConstantInt::get(ci -> getType(), zero));
   return true;
}


bool mulOptimization (BasicBlock::iterator Iter) {
   BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
   if (not binIter) return false;

   ConstantInt *ci = dyn_cast<ConstantInt>(binIter -> getOperand(0));
   Value *Other = binIter -> getOperand(1);

   if (not ci) {
      ci = dyn_cast<ConstantInt>(binIter -> getOperand(1));
      
      if (not ci) return false;
      
      Other = binIter -> getOperand(0);    
   }

   if (ci -> isZero())  return zeroMul(binIter, ci);

   if (not ci -> isOne()) {
      if (not ci -> getValue().isPowerOf2()) return mulToShift(binIter, ci, Other);
      
      return mulByPowOf2(binIter, ci, Other);
   }
   return mulBy1(binIter, Other);
}


// Division

bool divByPowOf2 (BinaryOperator *binIter, ConstantInt *ci, Value *Other) {
   Constant *val = ConstantInt::get(ci -> getType(), ci -> getValue().exactLogBase2());
   Instruction *NewInst = BinaryOperator::Create(Instruction::LShr, Other, val);
   NewInst -> insertAfter(binIter);
   binIter -> replaceAllUsesWith(NewInst);
   return true;
}


bool divBy1 (BinaryOperator *binIter, Value *Other) {
   binIter -> replaceAllUsesWith(Other);
   return true;
}


bool zeroDiv (BinaryOperator *binIter, ConstantInt *ci) {
   unsigned zero = 0;
   binIter -> replaceAllUsesWith(ConstantInt::get(ci -> getType(), zero));
   return true;
}


bool divOptimization (BasicBlock::iterator Iter) {
   BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
   if (not binIter) return false;

   ConstantInt *ci = dyn_cast<ConstantInt>(binIter -> getOperand(0));
   Value *Other = binIter -> getOperand(1);

   if (not ci) {
      ci = dyn_cast<ConstantInt>(binIter -> getOperand(1));

      if (not ci) return false;

      Other = binIter -> getOperand(0);

      if(not ci -> getValue().isPowerOf2()) return false;


      if(not ci -> getValue().isOne()) return divByPowOf2(binIter, ci, Other);
      
      return divBy1(binIter, Other);
   }

   if (not ci -> getValue().isZero()) return false;

   return zeroDiv(binIter, ci);
}


// Multi Instruction Optimization

bool isOpposite (unsigned int op1, unsigned int op2) {
   switch (op1) {
      case Instruction::Add:
         return op2 == Instruction::Sub;
      
      case Instruction::Sub:
         return op2 == Instruction::Add;
      
      case Instruction::Mul:
         return (op2 == Instruction::UDiv or op2 == Instruction::SDiv);
      
      case Instruction::UDiv:
      case Instruction::SDiv:
         return op2 == Instruction::Mul;
      
      default:
         return false;
   }
}


bool multiInstructionOptimization(BasicBlock::iterator Iter, BasicBlock &B)
{
   BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
   if (not binIter) return false;

   ConstantInt *ci = dyn_cast<ConstantInt>(binIter -> getOperand(1));

   if (not ci) return false;

   Value *Other = binIter -> getOperand(0);

   BinaryOperator *binIter2;
   ConstantInt *ci2;

   bool modified = false;

   for (Use &U: binIter->uses()) {
      Instruction *User = cast<Instruction>(U.getUser());
      if (isOpposite(binIter -> getOpcode(), User -> getOpcode())) {
         binIter2 = dyn_cast<BinaryOperator>(User);
         if (not binIter2) continue;

         ci2 = dyn_cast<ConstantInt>(binIter2 -> getOperand(1));

         if (not ci2 or ci != ci2) continue;
         
         User -> replaceAllUsesWith(Other);

         modified = true;
      }
   }

   return modified;
}


bool runOnBasicBlock (BasicBlock &B) {
   bool modified = false;

   for (BasicBlock::iterator Iter = B.begin(); Iter != B.end(); ++Iter) {
      if (multiInstructionOptimization(Iter,B)) modified = true;
      switch (Iter -> getOpcode()) {

      case Instruction::Add:
         if (addBy0(Iter)) {
            modified = true;
            continue;
         } 

         break;
      
      case Instruction::Sub:
         if (subBy0(Iter)) {
            modified = true;
            continue;
         } 

         break;

      case Instruction::Mul:
         if (mulOptimization(Iter)) {
            modified = true;
            continue;
         }

         break;

      case Instruction::UDiv:
      case Instruction::SDiv:
         if (divOptimization(Iter)) {
            modified = true;
            continue;
         }

         break;
      }
   }

   return modified;
}


bool runOnFunction (Function &F) {
   bool Transformed = false;

   for (Function::iterator Iter = F.begin(); Iter != F.end(); ++Iter) {
      if (runOnBasicBlock(*Iter)) {
         Transformed = true;
      }
   }

   return Transformed;
}


PreservedAnalyses LocalOpts::run (Module &M, ModuleAnalysisManager &AM) {
   for (Module::iterator Fiter = M.begin(); Fiter != M.end(); ++Fiter)
      if (runOnFunction(*Fiter))
         return PreservedAnalyses::none();

   return PreservedAnalyses::all();
}