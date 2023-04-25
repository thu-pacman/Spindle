#include "STracer.h"

#include <queue>

#include "llvm/Support/FileSystem.h"
#include "utils.h"

using namespace llvm;

namespace llvm {

void STracer::run(Instrumentation &instrument,
                  bool fullMem,
                  bool fullBr) {  // Static Trace
    std::error_code ec;
    raw_fd_ostream strace("strace.log", ec, sys::fs::OF_Append);
    strace << "File: " << instrument.getName() << "\n";
    int tot = 0, cnt = 0;
    for (auto F : MAS.functions) {
        strace << "Function: " << F->func.getName() << "\n";
        // step 1: find memory access dependencies
        auto depVisitor = MemDependenceVisitor(
            F->instrMeta,
            F->indVars);  // mark all dependent instructions in `instrMeta`
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                GetElementPtrInst *GEPI = nullptr;
                if (auto LI = dyn_cast<LoadInst>(&I)) {
                    depVisitor.visit(LI);
                } else if (auto SI = dyn_cast<StoreInst>(&I)) {
                    depVisitor.visit(SI);
                }
            }
        }
        // step 2: for each BB find whether the branch should be recorded
        queue<BasicBlock *> q;
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                if (dyn_cast<GetElementPtrInst>(&I) ||
                    F->instrMeta[&I].isSTraceDependence) {
                    F->bbMeta[&BB].needRecord = true;
                    q.push(&BB);
                    break;
                }
            }
        }
        for (; !q.empty(); q.pop()) {  // all the needed-record BB's succeeding
                                       // BBs need to be recorded
            for (auto BB : successors(q.front())) {
                if (!F->bbMeta[BB].needRecord) {
                    F->bbMeta[BB].needRecord = true;
                    q.push(BB);
                }
            }
        }
        for (auto &BB : F->func) {
            if (!F->bbMeta[&BB].inMASLoop && F->bbMeta[&BB].needRecord ||
                fullBr) {
                if (auto BrI = dyn_cast<BranchInst>(
                        BB.getTerminator());        // branches are in MDT
                    BrI && BrI->isConditional()) {  // instrument for br
                    F->instrMeta[BrI].isSTraceDependence = true;
                    instrument.record_br(BrI);
                }
            }
        }
        // step 3: print static trace
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                auto loop = F->instrMeta[&I].loop;
                if (loop && isa<PHINode>(I)) {
                    strace << "  For loop starts at " << I << '\n';
                    for (auto &indVar : loop->indVars) {
                        if (auto def =
                                dyn_cast<Instruction>(indVar.initValue)) {
                            instrument.record_value(def);
                        }
                        if (auto def =
                                dyn_cast<Instruction>(indVar.finalValue)) {
                            instrument.record_value(def);
                        }
                        strace << "\tLoop IndVar starts from "
                               << *indVar.initValue << ", ends at "
                               << *indVar.finalValue << ", steps by ";
                        indVar.delta->print(strace);
                        strace << '\n';
                    }
                    if (auto endPosition = loop->getEndPosition()) {
                        strace << "  For loop ends at " << *endPosition << '\n';
                    }
                } else if (F->instrMeta[&I].isSTraceDependence) {
                    strace << I << '\n';
                } else {
                    Instruction *ptr = nullptr;
                    auto LI = dyn_cast<LoadInst>(&I);
                    if (LI) {
                        ptr = dyn_cast<Instruction>(LI->getPointerOperand());
                    }
                    auto SI = dyn_cast<StoreInst>(&I);
                    if (SI) {
                        ptr = dyn_cast<Instruction>(SI->getPointerOperand());
                    }
                    if (ptr) {
                        if (LI) {
                            strace << *LI;
                        } else {
                            strace << *SI;
                        }
                        strace << "\n\tFormula: ";
                        auto visitor = loop ? ASTVisitor([&](Value *v) {
                            return loop->isLoopInvariant(v) ||
                                   F->indVars.count(v);
                        })
                                            : ASTVisitor([](Value *v) {
                                                  return Constant::classof(v);
                                              });
                        auto formula = visitor.visit(ptr);
                        formula->print(strace);
                        strace << '\n';

                        /*
                        // analyze `GEP` layer by layer
                        // TODO: below code is for struct access, merge to
                        // MemDependenceVisitor or move to a new function
                        auto typeOfFirstElement =
                            GEPI->getOperand(0)->getType();
                        // Ignore the first value
                        for (int i = 2; i < GEPI->getNumOperands(); i++) {
                            auto ptrType =
                                dyn_cast<PointerType>(typeOfFirstElement);
                            auto curOperand = GEPI->getOperand(i);
                            if (ptrType) {
                                auto typeOfPtr =
                                    ptrType->getPointerElementType();
                                if (auto structType =
                                        dyn_cast<StructType>(typeOfPtr)) {
                                    strace << "       Getting (" << *curOperand
                                           << ") of Struct type " << *typeOfPtr
                                           << ":";
                                    if (auto CI =
                                            dyn_cast<ConstantInt>(curOperand)) {
                                        typeOfFirstElement =
                                            (structType->getTypeAtIndex(
                                                CI->getLimitedValue()));
                                        strace << *typeOfFirstElement << "\n";
                                    } else {
                                        strace
                                            << "Cannot calculate: non-constant"
                                               "operand\n";
                                        break;
                                    }
                                } else if (auto AType =
                                               dyn_cast<ArrayType>(typeOfPtr)) {
                                    strace << "       Getting (" << *curOperand
                                           << ") value of ArrayType "
                                           << *typeOfPtr << "\n";
                                    typeOfFirstElement =
                                        AType->getElementType();
                                } else {
                                    strace << "Cannot analyze the type\n";
                                }
                            } else {
                                strace << "Not a pointer halfway\n";
                                break;
                            }
                            typeOfFirstElement =
                                PointerType::getUnqual(typeOfFirstElement);
                        }
                         */
                        if (loop) {
                            ++tot;
                            cnt += formula->computable;
                        }
                        /*
                        if (!formula->computable && loop) {
                            errs() << "non-computable in loop:\n";
                            if (LI) {
                                errs() << '\t' << *LI << "\n\t";
                            } else {
                                errs() << '\t' << *SI << "\n\t";
                            }
                            formula->print(errs());
                            errs() << '\n';
                            ASTVisitor(
                                [&](Value *v) {
                                  return loop->isLoopInvariant(v) ||
                                         F->indVars.count(v);
                                },
                                true)
                                .visit(ptr);
                        }
                         */
                        if (!formula->computable || fullMem) {
                            instrument.record_value(ptr);
                        }  // TODO: instrument for loop invariants
                    }
                }
            }
        }
    }
    errs() << "Canonical loops: " << MAS.num_canonical_form_loops << '/'
           << MAS.num_loops << '\n';
    errs() << "Computable memory accesses in loops: " << cnt << '/' << tot
           << '\n';
    errs() << "Static trace has been dumped into strace.log.\n";
}

}  // namespace llvm
