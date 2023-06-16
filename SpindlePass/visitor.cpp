#include "visitor.h"

#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "utils.h"

FormulaVisitor::FormulaVisitor(decltype(leafChecker) leafComputableChecker,
                               MASModule *M,
                               bool debug)
    : leafChecker(std::move(leafComputableChecker)),
      DL(M->module),
      debug(debug),
      context(M->context) {
}

auto FormulaVisitor::visitValue(Value *v) -> ASTAbstractNode * {
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

auto FormulaVisitor::visitInstruction(Instruction &I) -> ASTAbstractNode * {
    if (debug) {
        errs() << "Visiting other instruction" << I << ' '
               << leafChecker(cast<Value>(&I)) << '\n';
    }
    auto ret = new ASTLeafNode;
    ret->v = &I, ret->computable = leafChecker(cast<Value>(&I));
    return ret;
}

auto FormulaVisitor::visitUnaryInstruction(UnaryInstruction &UI)
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

auto FormulaVisitor::visitBinaryOperator(BinaryOperator &BOI)
    -> ASTAbstractNode * {
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

auto FormulaVisitor::visitGetElementPtrInst(GetElementPtrInst &GEPI)
    -> ASTAbstractNode * {
    if (debug) {
        errs() << "Visiting GEP" << GEPI << '\n';
    }
    auto ptr = GEPI.getPointerOperand();
    ASTAbstractNode *ret;
    ret = visitValue(ptr);
    auto GTI = gep_type_begin(GEPI);
    for (auto use = GEPI.operands().begin() + 1; use != GEPI.operands().end();
         ++use, ++GTI) {
        auto next = new ASTOpNode, cur = new ASTOpNode;
        auto offset = new ASTLeafNode;
        offset->v = IRBuilder<>(*context).getInt32(
            DL.getTypeSizeInBits(GTI.getIndexedType()) / 8);
        offset->computable = true;
        cur->lc = offset, cur->rc = visitValue(use->get());
        cur->opCode = Instruction::Mul;
        cur->computable = cur->rc->computable;
        next->lc = ret, next->rc = cur;
        next->opCode = Instruction::Add;
        next->computable = next->lc->computable && next->rc->computable;
        ret = next;
    }
    return ret;
}

MemDependenceVisitor::MemDependenceVisitor(decltype(meta) &meta,
                                           decltype(indVars) &indVars)
    : meta(meta), indVars(indVars) {
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

template <class T>
auto ASTVisitor<T>::dispatch(ASTAbstractNode *v) -> T {
    if (auto leaf = dynamic_cast<ASTLeafNode *>(v)) {
        return visit(leaf);
    } else {
        return visit(dynamic_cast<ASTOpNode *>(v));
    }
}

InstrumentationVisitor::InstrumentationVisitor(
    decltype(leafInstrumentation) leafFunc)
    : leafInstrumentation(std::move(leafFunc)) {
}

void InstrumentationVisitor::visit(ASTOpNode *v) {
    dispatch(v->lc), dispatch(v->rc);
}

void InstrumentationVisitor::visit(ASTLeafNode *v) {
    leafInstrumentation(v);
}

CalculationVisitor::CalculationVisitor(SymbolTable &table) : table(table) {
}

auto CalculationVisitor::visit(ASTOpNode *v) -> ValueType {
    auto lc = dispatch(v->lc), rc = dispatch(v->rc);
    switch (v->opCode) {
    case Instruction::Add:
        return lc + rc;
    case Instruction::Sub:
        return lc - rc;
    case Instruction::Mul:
        return lc * rc;
    case Instruction::UDiv:
    case Instruction::SDiv:
        return lc / rc;
    case Instruction::URem:
    case Instruction::SRem:
        return lc % rc;
    case Instruction::Shl:
        return lc << rc;
    case Instruction::LShr:
    case Instruction::AShr:
        return lc >> rc;
    case Instruction::And:
        return lc & rc;
    case Instruction::Or:
        return lc | rc;
    case Instruction::Xor:
        return lc ^ rc;
    default:
        errs() << "Unknown opcode for BinaryOperator: " << v->opCode << "\n";
        exit(1);
    }
}

auto CalculationVisitor::visit(ASTLeafNode *v) -> ValueType {
    if (auto constant = dyn_cast<ConstantInt>(v->v)) {
        return constant->getZExtValue();
    } else {
        assert(table.find(v->v) != table.end());
        return table[v->v];
    }
}
