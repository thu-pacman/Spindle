#include "visitor.h"

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
        ret->v = v, ret->computable = res;
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
    if (isa<CastInst>(UI)) {
        return visitValue(UI.getOperand(0));
    } else {
        auto ret = new ASTLeafNode;
        ret->v = &UI, ret->computable = leafChecker(cast<Value>(&UI));
        if (debug) {
            errs() << ret->computable << '\n';
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

void MemDependenceVisitor::visitLoadInst(LoadInst &LI) {
    if (auto pointer = dyn_cast<Instruction>(LI.getPointerOperand())) {
        visit(pointer);
    }
}

void MemDependenceVisitor::visitStoreInst(StoreInst &SI) {
    if (auto pointer = dyn_cast<Instruction>(SI.getPointerOperand())) {
        visit(pointer);
    }
}

void MemDependenceVisitor::visitGetElementPtrInst(GetElementPtrInst &GEPI) {
    meta[&GEPI].isSTraceDependence = true;
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

void MemDependenceVisitor::visitInstruction(Instruction &I) {
    if (meta[&I].isSTraceDependence) {
        return;
    }
    meta[&I].isSTraceDependence = true;
    for (auto &use : I.operands()) {
        if (auto def = dyn_cast<Instruction>(use);
            def && indVars.find(cast<Value>(use)) == indVars.end()) {
            visit(def);
        }
    }
}
