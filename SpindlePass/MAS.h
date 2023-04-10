#pragma once

#include <llvm/Analysis/LoopInfo.h>

#include <map>
#include <set>
#include <vector>

#include "AST.h"
#include "visitor.h"

using namespace llvm;

using std::map;
using std::set;
using std::vector;

class MASFunction;

class MASLoop {
    struct LoopIndVar {
        Value *initValue, *finalValue;
        ASTAbstractNode *delta;
    };

    Loop &loop;
    MASFunction *parent;
    SmallVector<Loop *> loops;  // the loop's all subloops

public:
    SmallVector<LoopIndVar> indVars;  // computable loops' loop variables
    bool is_canonical_loop;  // whether it satisfies loop simplify form

    MASLoop(Loop &loop, MASFunction *func) : loop(loop), parent(func) {
        // find all sub loops
        loops = {&loop};
        for (unsigned i = 0; i < loops.size(); ++i) {
            for (auto subLoop : loops[i]->getSubLoops()) {
                loops.push_back(subLoop);
            }
        }
    }
    auto isLoopInvariant(Value *v) const -> bool;
    [[nodiscard]] auto getEndPosition() const -> Instruction *;
    auto analyze() -> bool;
};

struct InstrMetaInfo {
    bool isSTraceDependence = false;
    MASLoop *loop = nullptr;
};

struct BBMetaInfo {
    bool needRecord = false, inMASLoop = false;
};

class MASFunction {
    LoopInfo LI;
    void analyzeLoop();

public:
    Function &func;
    set<Value *> indVars;  // loopVars
    map<Instruction *, InstrMetaInfo> instrMeta;
    map<BasicBlock *, BBMetaInfo> bbMeta;
    int num_loops;                 // number of loops in this function
    int num_canonical_form_loops;  // number of loops which satisfy loop
                                   // simplify form

    explicit MASFunction(Function &func) : func(func) {
        num_canonical_form_loops = 0;
        analyzeLoop();  // now only find all loops
    }
};

class MASModule {
public:
    vector<MASFunction *> functions;
    int num_loops;                 // number of loops in this module
    int num_canonical_form_loops;  // number of loops which satisfy loop
                                   // simplify form

    void analyze(Module &m);
};