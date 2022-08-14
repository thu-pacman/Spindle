#ifndef __SPINDLE_MAS_MODULE_ANALYSIS_HPP__
#define __SPINDLE_MAS_MODULE_ANALYSIS_HPP__

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

#include "./Module.hpp"

namespace spindle::mas
{

struct ModuleAnalysis : llvm::AnalysisInfoMixin<ModuleAnalysis>
{
	struct Result
	{
		Module result;

		Result(llvm::Module& M) : result(M) {}
	};

	Result run(llvm::Module& M, llvm::ModuleAnalysisManager& MAM)
	{
		llvm::outs() << "MAS analysis run\n";
		return M;
	}
private:
	static llvm::AnalysisKey Key;
	friend struct llvm::AnalysisInfoMixin<ModuleAnalysis>;
};

llvm::AnalysisKey ModuleAnalysis::Key;

}

#endif
