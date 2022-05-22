#include "visitor.h"

ASTAbstractNode *ASTVisitor::visitValue(Value *v) {
    if (auto I = dyn_cast<Instruction>(v)) {
        return visit(&(*I));
    } else {
        if (debug) {
            errs() << "Visiting leaf value" << *v << '\n';
        }
        auto *ret = new ASTLeafNode;
        ret->v = v, ret->computable = leafChecker(v);
        return ret;
    }
}

ASTAbstractNode *ASTVisitor::visitInstruction(Instruction &I) {
    if (debug) {
        errs() << "Visiting other instruction" << I << '\n';
    }
    auto *ret = new ASTLeafNode;
    ret->v = &I, ret->computable = leafChecker(cast<Value>(&I));
    return ret;
}

ASTAbstractNode *ASTVisitor::visitUnaryInstruction(UnaryInstruction &UI) {
    if (debug) {
        errs() << "Visiting unary instruction" << UI << '\n';
    }
    switch (UI.getOpcode()) {
    // ignore trivial cast
    case Instruction::SExt:
    case Instruction::ZExt:
        return visitValue(UI.getOperand(0));
        // TODO: add case for UnaryOperator
    default:
        auto *ret = new ASTLeafNode;
        ret->v = &UI, ret->computable = false;
        return ret;
    }
}

ASTAbstractNode *ASTVisitor::visitBinaryOperator(BinaryOperator &BOI) {
    if (debug) {
        errs() << "Visiting binary operator instruction" << BOI << '\n';
    }
    auto *ret = new ASTOpNode;
    ret->opCode = BOI.getOpcode();
    ret->lc = visitValue(BOI.getOperand(0));
    ret->rc = visitValue(BOI.getOperand(1));
    ret->computable = ret->lc->computable && ret->rc->computable;
    return ret;
}

ASTAbstractNode *ASTVisitor::visitGetElementPtrInst(GetElementPtrInst &GEPI) {
    if (debug) {
        errs() << "Visiting GEP" << GEPI << '\n';
    }
    auto ptr = GEPI.getPointerOperand();
    ASTAbstractNode *ret;
    ret = visitValue(ptr);
    /*
    auto *base = new ASTLeafNode;
    base->v = ptr, base->computable = true;
    ret = base; */
    for (auto use = GEPI.operands().begin() + 1; use != GEPI.operands().end();
         ++use) {
        auto *next = new ASTOpNode;
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
    if (indVars.find(ptr) == indVars.end()) {
        if (auto instr = dyn_cast<Instruction>(ptr)) {
            visit(instr);
        }
    }
    for (auto use = GEPI.operands().begin() + 1; use != GEPI.operands().end();
         ++use) {
        if (auto def = dyn_cast<Instruction>(use);
            def && indVars.find(cast<Value>(use)) == indVars.end()) {
            visit(def);
        }
    }
}

void GEPDependenceVisitor::visitInstruction(Instruction &I) {
    if (visited.find(&I) != visited.end()) {
        return;
    }
    visited.insert(&I);
    meta[&I].isGEPDependence = true;
    for (auto &use : I.operands()) {
        if (auto def = dyn_cast<Instruction>(use);
            def && indVars.find(cast<Value>(use)) == indVars.end()) {
            visit(def);
        }
    }
}