#ifndef __SPINDLE_MAS_BASIC_BLOCK_HPP__
#define __SPINDLE_MAS_BASIC_BLOCK_HPP__

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>

#include "./Value.hpp"

namespace spindle::mas
{

class BasicBlock final
{
	llvm::BasicBlock *_BB;

	std::vector<Value<llvm::Instruction>> _Is;
public:
	BasicBlock(llvm::BasicBlock& BB) : _BB(&BB), _Is(BB.begin(), BB.end()) {}

	BasicBlock(const BasicBlock&) = delete;
	BasicBlock(BasicBlock&&) noexcept = default;

	BasicBlock& operator=(const BasicBlock&) = delete;
	BasicBlock& operator=(BasicBlock&&) noexcept = default;
};

}

#endif
