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
#include <map>

using namespace llvm;

const std::map<Instruction::BinaryOps, Instruction::BinaryOps> inverseOperations = {
    {Instruction::Add, Instruction::Sub}, {Instruction::Sub, Instruction::Add},
    {Instruction::Mul, Instruction::UDiv}, {Instruction::UDiv, Instruction::Mul}
};


//Addition 

bool addBy0(BasicBlock::iterator Iter) {
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


/* Multiplication

a = nearestLog2(value) -> b = pow2(a) -> c = b*X +/- abs(b-value)*X, dove + con b<value e - con b>value
X<<a + X*abs(b-value)

X*A
B = 2^nearest - A
if (B!=0) // if log2(B)==0 -> x << nearest +/- x
  if (B>0)
    x << nearest - x <<log2(B)
  else if (B<0)
    X << nearest + X<<log2(|B|)
X<<nearest

*/


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
  unsigned log = diff.logBase2();

  Instruction *NewShlInst, *NewShlInst2, *NewInst;

  NewShlInst = BinaryOperator::Create(Instruction::Shl, Other, ConstantInt::get(ci -> getType(), near));
  NewShlInst -> insertAfter(binIter);
  Instruction::BinaryOps Operation;

  if(diff.slt(0))
    Operation = Instruction::Sub;
  else
    Operation = Instruction::Add;

  if (log == 0) {
    NewInst = BinaryOperator::Create(Operation, NewShlInst, Other);
    NewInst -> insertAfter(NewShlInst);
    binIter -> replaceAllUsesWith(NewInst);

    return true; 
  }
  
  NewShlInst2 = BinaryOperator::Create(Instruction::Shl, Other, ConstantInt::get(ci -> getType(), abs(log)));
  NewShlInst -> insertAfter(NewShlInst);
  NewInst = BinaryOperator::Create(Operation, NewShlInst, NewShlInst2);
  binIter -> replaceAllUsesWith(NewInst);
  return true;
}


bool mulBy1(BinaryOperator *binIter, Value *Other) {
  binIter -> replaceAllUsesWith(Other);
  return true;
}


bool mulOptimization(BasicBlock::iterator Iter){
  BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
  if (not binIter) return false;

  ConstantInt *ci = dyn_cast<ConstantInt>(binIter -> getOperand(0));
  Value *Other = binIter -> getOperand(1);

  if (not ci) {
    ci = dyn_cast<ConstantInt>(binIter -> getOperand(1));
    
    if (not ci) return false;
    
    Other = binIter -> getOperand(0);    
  }

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


bool divBy1(BinaryOperator *binIter, Value *Other) {
  binIter -> replaceAllUsesWith(Other);
  return true;
}


bool zeroDiv(BinaryOperator *binIter, ConstantInt *ci){
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


// Multi Instruction Optimizzation

bool multiInstructionOptimization(BasicBlock::iterator Iter, BasicBlock &B){
  BinaryOperator *binIter = dyn_cast<BinaryOperator>(Iter);
  if (not binIter) return false;

  ConstantInt *ci = dyn_cast<ConstantInt>(binIter -> getOperand(1));

  if (not ci) return false;
  
  Value *Other = binIter -> getOperand(0);

  if (inverseOperations.find(static_cast<Instruction::BinaryOps>(Iter -> getOpcode())) == inverseOperations.end()) return false;

  Instruction::BinaryOps inverse = inverseOperations.at(static_cast<Instruction::BinaryOps>(Iter -> getOpcode()));

  BinaryOperator *binIter2;
  ConstantInt *ci2;
  Value *Other2;
  
  bool modified = false;

  for (BasicBlock::iterator ni = ++Iter; ni != B.end(); ++ni){
    if (ni -> getOpcode() == inverse) {
      
      binIter2 = dyn_cast<BinaryOperator>(ni);
      if (not binIter2) continue;

      ci2 = dyn_cast<ConstantInt>(binIter2 -> getOperand(1));
      Other2 = binIter2 -> getOperand(0);

      if (not ci2 or ci != ci2) continue;
      if (Other2 != binIter) continue;
      
      ni -> replaceAllUsesWith(Other);

      modified = true;
    }
  }

 return modified;
}


bool runOnBasicBlock(BasicBlock &B) {
  bool modified = false;

  for (BasicBlock::iterator Iter = B.begin(); Iter != B.end(); ++Iter) {
    multiInstructionOptimization(Iter,B);
    switch (Iter -> getOpcode()) {

      case (Instruction::Add):
        if (addBy0(Iter)) {
          modified = true;
          continue;
        } 

        break;

      case (Instruction::Mul):
        if (mulOptimization(Iter)) {
          modified = true;
          continue;
        }

        break;

      case (Instruction::UDiv):
        if (divOptimization(Iter)) {
          modified = true;
          continue;
        }

        break;
    }
  }

  return modified;
}


bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (Function::iterator Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}


PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  for (Module::iterator Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();
  
  return PreservedAnalyses::all();
}