#ifndef __SPINDLE_TRACER_PASS_HPP__
#define __SPINDLE_TRACER_PASS_HPP__

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

#include "spindle/mas/ModuleAnalysis.hpp"

namespace spindle::mas
{

struct TracerPass : public llvm::PassInfoMixin<TracerPass>
{
	TracerPass() noexcept = default;

	llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager& MAM)
	{
		llvm::outs() << "tracer run\n";
		MAM.getResult<mas::ModuleAnalysis>(M);
		return llvm::PreservedAnalyses::all();
	}
};

}

#endif
