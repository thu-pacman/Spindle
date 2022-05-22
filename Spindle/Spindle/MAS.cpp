#include "MAS.h"

#include <llvm/Passes/PassBuilder.h>

void MASLoop::analyze(set<Value *> &parentIndVars) {
    auto header = loop.getHeader();
    for (auto instr = header->begin(); auto phi = dyn_cast<PHINode>(&(*instr));
         ++instr) {
        if (phi->getNumOperands() != 2) {
            continue;
        }
        LoopIndVar curIndVar;
        // calculate init value
        int idForUpdate = (phi->getIncomingBlock(1) == header);
        curIndVar.initValue = phi->getOperand(!idForUpdate);
        // calculate delta
        curIndVar.delta =
            ASTVisitor([&](Value *v) {
                return Constant::classof(v) ||
                       v == dyn_cast<Value>(instr);  // TODO: add loop invariant
            }).visitValue(phi->getOperand(idForUpdate));
        if (curIndVar.delta->computable) {
            // TODO: calculate final value
            indVars.push_back(curIndVar);
            parentIndVars.insert(cast<Value>(phi));
        }
    }
}

void MASFunction::analyzeLoop() {
    PassBuilder PB;
    FunctionAnalysisManager FAM;
    PB.registerFunctionAnalyses(FAM);
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(func);
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
        loops.push_back(new MASLoop(*loop));
        (*loops.rbegin())->analyze(indVars);
        instrMeta[&*loop->getHeader()->begin()].loop = *loops.rbegin();
        for (auto &BB : loop->blocks()) {
            bbMeta[BB].inLoop = true;
        }
    }
}

void MASModule::analyze(Module &m) {
    functions.clear();
    for (auto &F : m) {
        if (!F.isDeclaration()) {
            functions.push_back(new MASFunction(F));
        }
    }
}
