#include "MAS.h"

#include <llvm/IR/Dominators.h>
#include <llvm/Passes/PassBuilder.h>

auto MASLoop::isLoopInvariant(Value *v) const -> bool {
    if (Constant::classof(v) || Argument::classof(v)) {
        return true;
    }
    if (auto def = dyn_cast<Instruction>(v)) {
        return std::all_of(loops.begin(), loops.end(), [&](Loop *L) {
            return !L->contains(def);
        });
    }
    return false;
}

auto MASLoop::analyze() -> bool {
    auto header = loop.getHeader();
    auto preheader = loop.getLoopPreheader();
    auto latch = loop.getLoopLatch();
    auto exitBB = loop.getExitBlock();
    if (!preheader || !latch || !exitBB) {  // not a canonical form
        return false;
    }
    bool ret = false;
    for (auto instr = header->begin(); auto phi = dyn_cast<PHINode>(&(*instr));
         ++instr) {
        if (phi->getNumOperands() != 2) {
            continue;
        }
        LoopIndVar curIndVar;
        // calculate init value
        bool idForLatch = (phi->getIncomingBlock(1) == latch);
        curIndVar.initValue = phi->getOperand(!idForLatch);
        // calculate delta
        curIndVar.delta =
            ASTVisitor([&](Value *v) {
                return (v == dyn_cast<Value>(instr) || isLoopInvariant(v));
            }).visitValue(phi->getOperand(idForLatch));
        if (curIndVar.delta->computable) {
            // check and calculate final value
            if (auto brI = cast<BranchInst>(latch->getTerminator());
                brI->isConditional()) {
                if (auto icmpI = dyn_cast<ICmpInst>(brI->getCondition())) {
                    bool idForIndVar =
                        (icmpI->getOperand(1) == phi->getOperand(idForLatch));
                    if (ASTVisitor([&](Value *v) { return isLoopInvariant(v); })
                            .visitValue(icmpI->getOperand(!idForIndVar))
                            ->computable) {
                        curIndVar.finalValue = icmpI->getOperand(!idForIndVar);
                        indVars.push_back(curIndVar);
                        parent->indVars.insert(cast<Value>(phi));
                        ret = true;
                    }
                }
            }
        }
    }
    return ret;
}

auto MASLoop::getEndPosition() const -> Instruction * {
    if (!loop.getExitBlock()) {  // multiple exit blocks
        return nullptr;
    }
    return &loop.getExitBlock()->front();
}

void MASFunction::analyzeLoop() {
    PassBuilder PB;
    FunctionAnalysisManager FAM;
    PB.registerFunctionAnalyses(FAM);
    LI.analyze(FAM.getResult<DominatorTreeAnalysis>(func));
    // traverse all rawLoops
    vector<Loop *> rawLoops;
    for (auto loop : LI) {
        rawLoops.push_back(loop);
    }
    for (unsigned i = 0; i < rawLoops.size(); ++i) {
        for (auto subLoop : rawLoops[i]->getSubLoops()) {
            rawLoops.push_back(subLoop);
        }
    }
    for (auto loop : rawLoops) {
        auto masLoop = new MASLoop(*loop, this);
        if (masLoop->analyze()) {
            instrMeta[&loop->getHeader()->front()].loop = masLoop;
            for (auto BB : loop->blocks()) {
                bbMeta[BB].inMASLoop = true;
                for (auto &I : *BB) {
                    instrMeta[&I].loop = masLoop;
                }
            }
        } else {
            delete masLoop;
        }
    }
}

void MASModule::analyze(Module &M) {
    functions.clear();
    for (auto &F : M) {
        if (!F.isDeclaration()) {
            functions.push_back(new MASFunction(F));
        }
    }
}