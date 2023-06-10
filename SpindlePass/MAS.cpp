#include "MAS.h"

#include <llvm/IR/Dominators.h>
#include <llvm/Passes/PassBuilder.h>

#include "utils.h"

MASLoop::MASLoop(Loop &loop, MASFunction *func) : loop(loop), parent(func) {
}

auto MASLoop::isLoopInvariant(Value *v) const -> bool {
    return loop.isLoopInvariant(v);
}

auto MASLoop::analyze() -> bool {  // whether the loop is analyzable
    auto header = loop.getHeader(), preheader = loop.getLoopPreheader(),
         latch = loop.getLoopLatch(), exitBB = loop.getExitBlock();
    if (!preheader || !latch || !exitBB) {  // not a canonical form
        return false;
    }
    bool ret = false;
    for (auto instr = header->begin(); auto phi = dyn_cast<PHINode>(&(*instr));
         ++instr) {
        // a simplified loop's header only has two input edges, one from
        // preheader and the other from latch
        if (phi->getNumOperands() != 2) {
            continue;
        }
        LoopIndVar curIndVar;
        // calculate init value
        bool idForLatch = (phi->getIncomingBlock(1) == latch);
        curIndVar.initValue = phi->getOperand(!idForLatch);
        // calculate delta
        curIndVar.delta =
            FormulaVisitor(
                [&](Value *v) {
                    return (v == dyn_cast<Value>(instr) || isLoopInvariant(v));
                },
                parent->parent)
                .visitValue(phi->getOperand(idForLatch));
        if (curIndVar.delta->computable) {
            // check and calculate final value
            if (auto brI = cast<BranchInst>(latch->getTerminator());
                brI->isConditional()) {
                if (auto icmpI = dyn_cast<ICmpInst>(brI->getCondition())) {
                    bool idForIndVar =
                        (icmpI->getOperand(1) == phi->getOperand(idForLatch));
                    curIndVar.finalValue = icmpI->getOperand(!idForIndVar);
                    if (isLoopInvariant(curIndVar.finalValue)) {
                        // more fine-grained check may be applicable with
                        // following visitor pattern
                        // if (FormulaVisitor([&](Value *v) { return
                        // isLoopInvariant(v); })
                        //.visitValue(curIndVar.finalValue)
                        //->computable) {
                        indVars.push_back(curIndVar);
                        parent->indVars.insert(cast<Value>(phi));
                        ret = true;
                        parent->instrMeta[phi].indVar = &*indVars.rbegin();
                    }
                }
            }
        }
    }
    return ret;
}

MASFunction::MASFunction(Function &func, MASModule *module)
    : func(func), parent(module), num_computable_loops(0) {
    analyzeLoop();  // now only find all loops
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
    for (unsigned i = 0; i < rawLoops.size(); ++i) {  // get all subloops
        for (auto subLoop : rawLoops[i]->getSubLoops()) {
            rawLoops.push_back(subLoop);
        }
    }
    num_loops = rawLoops.size();
    for (auto loop : rawLoops) {
        auto masLoop = new MASLoop(*loop, this);
        if (masLoop->analyze()) {
            // loop can be identified by its header
            bbMeta[loop->getHeader()].loop = masLoop;
            for (auto BB : loop->blocks()) {
                bbMeta[BB].inMASLoop = true;
            }
            ++num_computable_loops;
        } else {
            delete masLoop;
        }
    }
}

MASModule::MASModule(Module &M) : module(&M) {
    context = new LLVMContext;
}

void MASModule::analyze() {
    functions.clear();
    for (auto &F : *module) {
        if (!F.isDeclaration()) {
            functions.push_back(new MASFunction(F, this));
        }
    }
    num_loops = 0;
    num_computable_loops = 0;
    for (auto func : functions) {
        num_loops += func->num_loops;
        num_computable_loops += func->num_computable_loops;
    }
}
