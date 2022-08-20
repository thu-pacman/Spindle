# Spindle

Spindle is an efficient memory access monitoring framework. Unlike methods delaying all checks to runtime or performing task-specific optimization at compile time, Spindle performs common static analysis to identify predictable memory access patterns into a compact program structure summary. Custom memory monitoring tools can then be developed on top of Spindle, leveraging the structural information extracted to dramatically reduce the amount of instrumentation that incurs heavy runtime memory address examination or recording.

## Requirements:
LLVM 14

## Installation as an out-of-tree pass:

Use `./build.py -h" to see the building options. Notes that the script requires at least Python 3.6 to run.

Common use cases:

```shell
# For building the old version of SpindlePass with -O0
./build.py -O 0 -e loop_unsafe.c nested_gep.c -P stracer --inject stracer lib/stracer.cpp

# For building the old version of SpindlePass with -O2
./build.py -O 2 -e loop_unsafe.c nested_gep.c -P stracer --inject stracer lib/stracer.cpp --clang-f-extra no-unroll-loops no-vectorize

# Have a glance at the experimental frameworks (still WIP)
./build.py -E -O 0 -e loop_unsafe.c nested_gep.c -P "trace<mas>" "detect<mas>"
```

Load Spindle pass directly using clang is currently not supported since this version do not support LLVM's new PassManager.
Supporting for new PassManager will be updated in the future version.

Now Spindle is still under updating: divide the Spindle common static to different modules so that the logic can be more clear; re-implement S-Tracer with the latest version of LLVM (previous version of S-Tracer is based on LLVM 3.5); optimize S-Detector for more efficient memory bug checking, including using better algorithm for static analysis and implementing a faster runtime library.
