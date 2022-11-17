#include "visitor.h"

#include <iostream>

#include "utils.h"

auto ASTVisitor::visitValue(Value *v) -> ASTAbstractNode * {
    bool res = leafChecker(v);
    auto I = dyn_cast<Instruction>(v);
    if (!res && I) {
        return visit(I);
    } else {
        if (debug) {
            errs() << "Visiting leaf value " << *v << ' ' << leafChecker(v)
                   << '\n';
        }
        auto ret = new ASTLeafNode;
        ret->v = v,
        ret->computable = res ? res : leafChecker(v);
        return ret;
    }
}

auto ASTVisitor::visitInstruction(Instruction &I) -> ASTAbstractNode * {
    if (debug) {
        errs() << "Visiting other instruction" << I << ' '
               << leafChecker(cast<Value>(&I)) << '\n';
    }
    auto ret = new ASTLeafNode;
    ret->v = &I, ret->computable = leafChecker(cast<Value>(&I));
    return ret;
}

auto ASTVisitor::visitUnaryInstruction(UnaryInstruction &UI)
    -> ASTAbstractNode * {
    if (debug) {
        errs() << "Visiting unary instruction" << UI << '\n';
    }
    switch (UI.getOpcode()) {
    // ignore trivial cast
    case Instruction::SExt:
    case Instruction::ZExt:
    case Instruction::Trunc:
    case Instruction::FPToSI:
    case Instruction::Load:
        // FIXME: Actually `load` should not be a unary operator. Here `load` is
        // for conveniently checking loop invariants. Consider using
        // Loop::isLoopInvariant instead.
        return visitValue(UI.getOperand(0));
        // TODO: add case for UnaryOperator
    default:
        auto ret = new ASTLeafNode;
        ret->v = &UI, ret->computable = false;
        if (debug) {
            errs() << "0\n";
        }
        return ret;
    }
}

auto ASTVisitor::visitBinaryOperator(BinaryOperator &BOI) -> ASTAbstractNode * {
    if (debug) {
        errs() << "Visiting binary operator instruction" << BOI << '\n';
    }
    auto ret = new ASTOpNode;
    ret->opCode = BOI.getOpcode();
    ret->lc = visitValue(BOI.getOperand(0));
    ret->rc = visitValue(BOI.getOperand(1));
    ret->computable = ret->lc->computable && ret->rc->computable;
    return ret;
}

auto ASTVisitor::visitGetElementPtrInst(GetElementPtrInst &GEPI)
    -> ASTAbstractNode * {
    if (debug) {
        errs() << "Visiting GEP" << GEPI << '\n';
    }
    auto ptr = GEPI.getPointerOperand();
    ASTAbstractNode *ret;
    ret = visitValue(ptr);
    for (auto use = GEPI.operands().begin() + 1; use != GEPI.operands().end();
         ++use) {
        auto next = new ASTOpNode;
        next->lc = ret;
        next->rc = visitValue(use->get());
        next->opCode = Instruction::Add;
        next->computable = next->lc->computable && next->rc->computable;
        ret = next;
    }
    return ret;
}

void GEPDependenceVisitor::visitGetElementPtrInst(GetElementPtrInst &GEPI) {
    auto ptr = GEPI.getPointerOperand();
    if (indVars.find(ptr) == indVars.end()) {  // not loopVars
        if (auto instr = dyn_cast<Instruction>(ptr)) {
            visit(instr);
        }
    }
    for (auto use = GEPI.operands().begin() + 1;
         use !=
         GEPI.operands().end();  // operands: op_range(op_begin(), op_end());
         ++use) {  // using op_range = iterator_range<op_iterator>;
                   // using op_iterator = Use*;
        if (auto def = dyn_cast<Instruction>(use);
            def &&
            indVars.find(cast<Value>(use)) == indVars.end()) {  // not loopVars
            visit(def);
        }
    }
}

void GEPDependenceVisitor::visitInstruction(Instruction &I) {
    if (meta[&I].isSTraceDependence) {
        return;
    }
    meta[&I].isSTraceDependence = true;  // in MDT(Memory Dependency Tree)
    for (auto &use : I.operands()) {
        if (auto def = dyn_cast<Instruction>(use);
            def && indVars.find(cast<Value>(use)) == indVars.end()) {
            visit(def);
        }
    }
}
