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

class MASLoop {
    struct LoopIndVar {
        Value *initValue, *finalValue;
        ASTAbstractNode *delta;
    };

    Loop &loop;

public:
    vector<LoopIndVar> indVars;

    explicit MASLoop(Loop &loop) : loop(loop) {
    }

    void analyze(set<Value *> &parentIndVars);
};

struct InstrMetaInfo {
    bool isSTraceDependence = false;
    MASLoop *loop = nullptr;
};

struct BBMetaInfo {
    bool needRecord = false, inLoop = false;
};

class MASFunction {
    vector<MASLoop *> loops;

    void analyzeLoop();

public:
    Function &func;
    set<Value *> indVars;
    map<Instruction *, InstrMetaInfo> instrMeta;
    map<BasicBlock *, BBMetaInfo> bbMeta;

    explicit MASFunction(Function &func) : func(func) {
        analyzeLoop();  // now only find all loops
    }
};

class MASModule {
    vector<MASFunction *> functions;

public:
    void analyze(Module &m);

    vector<MASFunction *> &getFunctions() {
        return functions;
    }
};
