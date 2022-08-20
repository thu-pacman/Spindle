#ifndef __SPINDLE_MAS_MODULE_HPP__
#define __SPINDLE_MAS_MODULE_HPP__

#include <vector>

#include <llvm/IR/Module.h>

#include "./Function.hpp"

namespace spindle::mas
{

class Module final
{
	llvm::Module *_M;

	std::vector<Function> _functions;
public:
	Module(llvm::Module& M) noexcept : _M(&M)
	{
		_functions.reserve(M.size());
		for (auto & F : M)
			_functions.emplace_back(F);
	}

	~Module() noexcept = default;

	Module(const Module&) = delete;
	Module(Module&&) noexcept = default;

	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) noexcept = default;
};

}

#endif
