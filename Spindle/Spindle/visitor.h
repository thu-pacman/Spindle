#pragma once
#include "AST.h"
#include "llvm/IR/InstVisitor.h"

using namespace llvm;

class ASTVisitor: public InstVisitor<ASTVisitor, ASTAbstractNode *> {
    function_ref<bool (Value *)> leafChecker;
    bool debug;
  public:
    explicit ASTVisitor(function_ref<bool (Value *)> leafChecker, bool debug = false)
        : leafChecker(leafChecker), debug(debug) {}
    ASTAbstractNode *visitValue(Value *v);
    ASTAbstractNode *visitInstruction(Instruction &I);
    ASTAbstractNode *visitUnaryInstruction(UnaryInstruction &UI);
    ASTAbstractNode *visitBinaryOperator(BinaryOperator &BOI);
    ASTAbstractNode *visitGetElementPtrInst(GetElementPtrInst &GEPI);
};
