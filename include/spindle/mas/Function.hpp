#ifndef __SPINDLE_MAS_FUNCTION_HPP__
#define __SPINDLE_MAS_FUNCTION_HPP__

#include <llvm/IR/Argument.h>
#include <llvm/IR/Function.h>

#include "./BasicBlock.hpp"
#include "./Value.hpp"

namespace spindle::mas
{

class Function final
{
	llvm::Function *_F;

	std::vector<Value<llvm::Argument>> _arguments;
	std::vector<BasicBlock> _basic_blocks;
public:
	Function(llvm::Function& F) : _F(&F), _arguments(F.arg_begin(), F.arg_end()), _basic_blocks(F.begin(), F.end()) {}

	Function(const Function&) = delete;
	Function(Function&&) noexcept = default;

	Function& operator=(const Function&) = delete;
	Function& operator=(Function&&) noexcept = default;
};

}

#endif
