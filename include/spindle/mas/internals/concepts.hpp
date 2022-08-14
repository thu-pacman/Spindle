#ifndef __SPINDLE_MAS_INTERNALS_CONCEPTS_HPP__
#define __SPINDLE_MAS_INTERNALS_CONCEPTS_HPP__

#include <llvm/IR/Value.h>

namespace llvm
{

template<typename V>
concept ValueConcept = std::is_base_of_v<Value, V>;

}

#endif
