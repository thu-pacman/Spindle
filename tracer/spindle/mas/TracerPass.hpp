#ifndef __SPINDLE_TRACER_PASS_HPP__
#define __SPINDLE_TRACER_PASS_HPP__

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

#include "spindle/mas/internals/macros.hpp"
#include "spindle/mas/ModuleAnalysis.hpp"

namespace spindle::mas
{

struct TracerPass : public llvm::PassInfoMixin<TracerPass>
{
	TracerPass() noexcept = default;

	llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager& MAM)
	{
		SPINDLE_FORMAT_STDOUT("tracer run");
		MAM.getResult<mas::ModuleAnalysis>(M);
		return llvm::PreservedAnalyses::all();
	}
};

}

#endif
