#ifndef __SPINDLE_MAS_FUNCTION_HPP__
#define __SPINDLE_MAS_FUNCTION_HPP__

#include <llvm/IR/Argument.h>
#include <llvm/IR/Function.h>

#include "./internals/macros.hpp"
#include "./BasicBlock.hpp"
#include "./Sequence.hpp"
#include "./Value.hpp"

namespace spindle::mas
{

class Function final
{
	llvm::Function *_F;

	std::vector<Value<llvm::Argument>> _arguments;
	std::vector<Sequence<llvm::Argument, llvm::Argument>> _arguments_sequences;
	std::vector<BasicBlock> _basic_blocks;

	[[using clang : always_inline]]
	inline void _analyse_general(llvm::Function& F) &
	{
		_arguments.reserve(F.arg_size());
		for (auto& arg : F.args())
			_arguments.emplace_back(arg);
	}

	[[using clang : always_inline]]
	inline void _analyse_main(llvm::Function& F) &
	{
		switch (F.arg_size())
		{
			// int main()
			case 0:
				return;
			// int main(int argc, char *argv[])
			case 2:
				_arguments_sequences.reserve(1);
				_arguments_sequences.emplace_back(*F.getArg(1), *F.getArg(0));
				return;
			// invalid signature (including the potentially supported syntax with `envp`)
			default:
				SPINDLE_FORMAT_STDERR("`main` function signature unknown and will be treated as normal function");
				_analyse_general(F);
				return;
		}
	}
public:
	Function(llvm::Function& F) : _F(&F), _arguments(), _arguments_sequences(), _basic_blocks(F.begin(), F.end())
	{
		if (F.getName() == "main")
			_analyse_main(F);
		else
			_analyse_general(F);
	}

	Function(const Function&) = delete;
	Function(Function&&) noexcept = default;

	Function& operator=(const Function&) = delete;
	Function& operator=(Function&&) noexcept = default;
};

}

#endif
