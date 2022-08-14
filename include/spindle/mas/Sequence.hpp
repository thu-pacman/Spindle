#ifndef __SPINDLE_MAS_SEQUENCE_HPP__
#define __SPINDLE_MAS_SEQUENCE_HPP__

#include "./internals/concepts.hpp"

namespace spindle::mas
{

template<llvm::ValueConcept VP, llvm::ValueConcept VB = VP>
class Sequence final
{
	VP *_pointer_value;
	VB *_bound_value;
public:
	Sequence(VP& pointer_value, VB& bound_value) : _pointer_value(&pointer_value), _bound_value(&bound_value) {}

	Sequence(const Sequence&) = delete;
	Sequence(Sequence&&) noexcept = default;

	Sequence& operator=(const Sequence&) = delete;
	Sequence& operator=(Sequence&&) noexcept = default;
};

}

#endif
