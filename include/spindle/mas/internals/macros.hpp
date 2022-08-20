#ifndef __SPINDLE_MAS_INTERNALS_MACROS_HPP__
#define __SPINDLE_MAS_INTERNALS_MACROS_HPP__

#include <fmt/compile.h>
#include <fmt/format.h>
#include <llvm/Support/raw_ostream.h>

#define SPINDLE_DECORATE_FORMAT(__FS, ...) fmt::format(FMT_COMPILE("(Spindle) " __FS "\n") __VA_OPT__(,) __VA_ARGS__)
#define SPINDLE_FORMAT_STDOUT(...) llvm::outs() << SPINDLE_DECORATE_FORMAT(__VA_ARGS__)
#define SPINDLE_FORMAT_STDERR(...) llvm::errs() << SPINDLE_DECORATE_FORMAT(__VA_ARGS__)

#endif
