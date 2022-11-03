#include "MAS.h"

#include <llvm/IR/Dominators.h>
#include <llvm/Passes/PassBuilder.h>

#include <iostream>
#include "utils.h"

auto MASLoop::isLoopInvariant(Value *v) const -> bool {         // check a Value whether is invariable in the loop
    if (Constant::classof(v) || Argument::classof(v)) {         // why `Argument` ???
        return true;
    }
    if (auto def = dyn_cast<Instruction>(v)) {                  // all subloops cannot contain the `Instruction`
        return std::all_of(loops.begin(), loops.end(), [&](Loop *L) {
            return !L->contains(def);
        });
    }
    return false;
}

auto MASLoop::analyze() -> bool {                               // whether the loop is analyzable
    std::cout << "\nfind a loop !!!" << std::endl;
    std::cout << "Loop: " << Print(&(this->loop)) << std::endl;

    auto header = loop.getHeader();
    auto preheader = loop.getLoopPreheader();           // the preheader must have only one exit !!!
    auto latch = loop.getLoopLatch();
    auto exitBB = loop.getExitBlock();
    if (!preheader || !latch || !exitBB) {      // not a canonical form
        std::cout << "not a canonical form" << std::endl;
        // if (preheader) {
        //     std::cout << "preheader: " << Print(preheader) << std::endl;
        // }
        // if (latch) {
        //     std::cout << "latch: " << Print(latch) << std::endl;
        // }
        // if (exitBB) {
        //     std::cout << "exitBB: " << Print(exitBB) << std::endl;
        // }
        return false;
    }
    bool ret = false;
    for (auto instr = header->begin(); auto phi = dyn_cast<PHINode>(&(*instr));
         ++instr) {
        if (phi->getNumOperands() != 2) {       // a canonical form loop's `header` only has two input edges
            continue;                           // one from `preheader` and the other `latch`
        }
        LoopIndVar curIndVar;
        // calculate init value
        bool idForLatch = (phi->getIncomingBlock(1) == latch);      // (`for`, `do_while`) or `while`
        curIndVar.initValue = phi->getOperand(!idForLatch);
        // calculate delta
        std::cout << "cur_instr: " << Print(&*instr) << std::endl;
        std::cout << "PHINode->latch: " << Print(phi->getOperand(idForLatch)) << std::endl;     // *ssa* trait

        curIndVar.delta =
            ASTVisitor([&](Value *v) {                              // leafChecker(), why is it ???
                return (v == dyn_cast<Value>(instr) || isLoopInvariant(v));
            }, true).visitValue(phi->getOperand(idForLatch));
        std::cout << "assign delta end" << std::endl;
        if (curIndVar.delta->computable) {
            // check and calculate final value
            if (auto brI = cast<BranchInst>(latch->getTerminator());
                brI->isConditional()) {
                std::cout << "icmpI: " << Print(dyn_cast<ICmpInst>(brI->getCondition())) << std::endl;
                if (auto icmpI = dyn_cast<ICmpInst>(brI->getCondition())) { // what if the contition is not `ICmpInst` ???
                    bool idForIndVar =
                        (icmpI->getOperand(1) == phi->getOperand(idForLatch));      // loopVsar
                    if (ASTVisitor([&](Value *v) { return isLoopInvariant(v); })
                            .visitValue(icmpI->getOperand(!idForIndVar))            // finalValue
                            ->computable) {
                        curIndVar.finalValue = icmpI->getOperand(!idForIndVar);
                        indVars.push_back(curIndVar);
                        std::cout << "indVar: " << Print(cast<Value>(phi)) << std::endl;
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
    LI.analyze(FAM.getResult<DominatorTreeAnalysis>(func));     // how does it works ???
    // traverse all rawLoops
    vector<Loop *> rawLoops;
    for (auto loop : LI) {
        rawLoops.push_back(loop);
    }
    for (unsigned i = 0; i < rawLoops.size(); ++i) {        // get all subloops
        for (auto subLoop : rawLoops[i]->getSubLoops()) {
            rawLoops.push_back(subLoop);
        }
    }
    for (auto loop : rawLoops) {
        auto masLoop = new MASLoop(*loop, this);
        if (masLoop->analyze()) {
            std::cout << "Find a analyzable loop: " << Print(&loop->getHeader()->front()) << std::endl;
            instrMeta[&loop->getHeader()->front()].loop = masLoop;
            for (auto BB : loop->blocks()) {
                // std::cout << "BB: " << Print(BB) << std::endl;
                bbMeta[BB].inMASLoop = true;
                for (auto &I : *BB) {
                    instrMeta[&I].loop = masLoop;       // WARNING: a instr might be labeled by many loops !!!
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