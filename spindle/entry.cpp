#include <cstdint>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

#include "entry.h"
#include "spindle/mas/DetectorPass.hpp"
#include "spindle/mas/ModuleAnalysis.hpp"
#include "spindle/mas/TracerPass.hpp"

namespace spindle::mas
{

enum class Pass : uint8_t
{
	Detector,
	Tracer
};

static const llvm::StringMap<Pass> PASS_MAP {
	{ "detect<mas>", Pass::Detector },
	{ "trace<mas>", Pass::Tracer }
};

}

llvm::PassPluginLibraryInfo getSpindlePluginInfo()
{
	return {
		.APIVersion = LLVM_PLUGIN_API_VERSION,
		.PluginName = "Spindle",
		.PluginVersion = LLVM_VERSION_STRING,
		.RegisterPassBuilderCallbacks = [](llvm::PassBuilder& PB) {
			PB.registerAnalysisRegistrationCallback([](llvm::ModuleAnalysisManager& MAM) -> void {
				MAM.registerPass([]() -> spindle::mas::ModuleAnalysis { return {}; });
			});

			PB.registerPipelineParsingCallback([](
				llvm::StringRef Name,
				llvm::ModulePassManager& MPM,
				llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
			) -> bool {
				using namespace spindle::mas;

				if (auto PassIter = PASS_MAP.find(Name); PassIter != PASS_MAP.end())
					switch (PassIter->second)
					{
						case Pass::Detector:
							MPM.addPass(DetectorPass());
							return true;
						case Pass::Tracer:
							MPM.addPass(TracerPass());
							return true;
						default:
							break;
					}
				return false;
			});
		}
	};
}

extern "C" LLVM_ATTRIBUTE_WEAK
::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
	return getSpindlePluginInfo();
}
