#ifndef __SPINDLE_MAS_VALUE_HPP__
#define __SPINDLE_MAS_VALUE_HPP__

#include "./internals/concepts.hpp"

namespace spindle::mas
{

template<llvm::ValueConcept V>
class Value final
{
	V *_value;
public:
	Value(V& value) : _value(&value) {}

	Value(const Value&) = delete;
	Value(Value&&) noexcept = default;

	Value& operator=(const Value&) = delete;
	Value& operator=(Value&&) noexcept = default;
};

}

#endif
