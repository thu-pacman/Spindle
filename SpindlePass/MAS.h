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

public:
    SmallVector<LoopIndVar> indVars;  // computable loops' loop variables

    MASLoop(Loop &loop, MASFunction *func) : loop(loop), parent(func) {
    }
    auto isLoopInvariant(Value *v) const -> bool;
    auto analyze() -> bool;
};

struct InstrMetaInfo {
    bool isSTraceDependence = false;
};

struct BBMetaInfo {
    bool needRecord = false, inMASLoop = false;
    MASLoop *loop = nullptr;
};

class MASFunction {
    LoopInfo LI;
    void analyzeLoop();

public:
    Function &func;
    set<Value *> indVars;
    map<Instruction *, InstrMetaInfo> instrMeta;
    map<BasicBlock *, BBMetaInfo> bbMeta;
    size_t num_loops;
    size_t num_computable_loops;

    explicit MASFunction(Function &func) : func(func), num_computable_loops(0) {
        analyzeLoop();  // now only find all loops
    }
};

class MASModule {
public:
    vector<MASFunction *> functions;
    size_t num_loops;
    size_t num_computable_loops;

    void analyze(Module &m);
};
