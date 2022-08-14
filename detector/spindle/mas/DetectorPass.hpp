#ifndef __SPINDLE_DETECTOR_PASS_HPP__
#define __SPINDLE_DETECTOR_PASS_HPP__

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

#include "spindle/mas/ModuleAnalysis.hpp"

namespace spindle::mas
{

struct DetectorPass : public llvm::PassInfoMixin<DetectorPass>
{
	DetectorPass() noexcept = default;

	llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager& MAM)
	{
		llvm::outs() << "detector run\n";
		MAM.getResult<mas::ModuleAnalysis>(M);
		return llvm::PreservedAnalyses::all();
	}
};

}

#endif
