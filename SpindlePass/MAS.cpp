#include "MAS.h"

#include <llvm/IR/Dominators.h>
#include <llvm/Passes/PassBuilder.h>

#include <iostream>

#include "utils.h"

auto MASLoop::isLoopInvariant(Value *v) const
    -> bool {  // check a Value whether is invariable in the loop
    if (Constant::classof(v) || Argument::classof(v)) {  // why `Argument` ???
        return true;
    }
    if (auto def = dyn_cast<Instruction>(
            v)) {  // all subloops cannot contain the `Instruction`
        return std::all_of(loops.begin(), loops.end(), [&](Loop *L) {
            return !L->contains(def);
        });
    }
    return false;
}

auto MASLoop::analyze() -> bool {  // whether the loop is analyzable
    auto header = loop.getHeader();
    auto preheader =
        loop.getLoopPreheader();  // the preheader must have only one exit !!!
    auto latch = loop.getLoopLatch();
    auto exitBB = loop.getExitBlock();
    if (!preheader || !latch || !exitBB) {  // not a canonical form
        is_canonical_loop = false;
        return false;
    } else {
        is_canonical_loop = true;
    }
    bool ret = false;
    for (auto instr = header->begin(); auto phi = dyn_cast<PHINode>(&(*instr));
         ++instr) {
        if (phi->getNumOperands() !=
            2) {  // a canonical form loop's `header` only has two input edges
            continue;  // one from `preheader` and the other `latch`
        }
        LoopIndVar curIndVar;
        // calculate init value
        bool idForLatch = (phi->getIncomingBlock(1) ==
                           latch);  // (`for`, `do_while`) or `while`
        curIndVar.initValue = phi->getOperand(!idForLatch);
        // calculate delta
        curIndVar.delta =
            ASTVisitor([&](Value *v) {  // leafChecker(), why is it ???
                return (v == dyn_cast<Value>(instr) || isLoopInvariant(v));
            })
                .visitValue(phi->getOperand(idForLatch));
        if (curIndVar.delta->computable) {
            // check and calculate final value
            if (auto brI = cast<BranchInst>(latch->getTerminator());
                brI->isConditional()) {
                if (auto icmpI = dyn_cast<ICmpInst>(
                        brI->getCondition())) {  // what if the contition is not
                                                 // `ICmpInst` ???
                    bool idForIndVar =
                        (icmpI->getOperand(1) ==
                         phi->getOperand(idForLatch));  // loopVsar
                    if (ASTVisitor([&](Value *v) { return isLoopInvariant(v); })
                            .visitValue(
                                icmpI->getOperand(!idForIndVar))  // finalValue
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
    LI.analyze(
        FAM.getResult<DominatorTreeAnalysis>(func));  // how does it works ???
    // traverse all rawLoops
    vector<Loop *> rawLoops;
    for (auto loop : LI) {
        rawLoops.push_back(loop);
    }
    for (unsigned i = 0; i < rawLoops.size(); ++i) {  // get all subloops
        for (auto subLoop : rawLoops[i]->getSubLoops()) {
            rawLoops.push_back(subLoop);
        }
    }
    num_loops = rawLoops.size();
    for (auto loop : rawLoops) {
        auto masLoop = new MASLoop(*loop, this);
        if (masLoop->analyze()) {
            instrMeta[&loop->getHeader()->front()].loop = masLoop;
            for (auto BB : loop->blocks()) {
                bbMeta[BB].inMASLoop = true;
                for (auto &I : *BB) {
                    instrMeta[&I].loop = masLoop;  // WARNING: a instr might be
                                                   // labeled by many loops !!!
                }
            }
        } else {
            delete masLoop;
        }
        if (masLoop->is_canonical_loop) {
            ++num_canonical_form_loops;
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
    num_loops = 0;
    num_canonical_form_loops = 0;
    for (auto func : functions) {
        num_loops += func->num_loops;
        num_canonical_form_loops += func->num_canonical_form_loops;
    }
}