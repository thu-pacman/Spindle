# Spindle

Spindle is an efficient memory access monitoring framework. Unlike methods delaying all checks to runtime or performing task-specific optimization at compile time, Spindle performs common static analysis to identify predictable memory access patterns into a compact program structure summary. Custom memory monitoring tools can then be developed on top of Spindle, leveraging the structural information extracted to dramatically reduce the amount of instrumentation that incurs heavy runtime memory address examination or recording.

## Requirements:
LLVM 15.0.0

## Installation as an out-of-tree pass:
```shell
./build.sh $<desired toolchain name> SpindlePass
```

for example

```shell
./build.sh ubuntu SpindlePass
```

## Run test:
```shell
./build_example.sh $<desired toolchain name> $<pass name> $<example name in ./examples> $<clang optimisation level>
```

for example

```shell
./build_example.sh ubuntu SpindlePass stracer loop_unsafe 0
```

or more commonly

```shell
./build_example.sh ubuntu SpindlePass stracer loop_unsafe 2 -fno-unroll-loops -fno-vectorize
```

Notes that to simplify the work in the current status, the source files under `./examples` are all considered written in C (due to the fixed suffix, `.c`, is used in the script). It can be extended to support arbitrary language (which, however, must be supported by LLVM) in the future in case of necessity.

Load Spindle pass directly using clang is currently not supported since this version do not support LLVM's new PassManager.
Supporting for new PassManager will be updated in the future version.

Now Spindle is still under updating: divide the Spindle common static to different modules so that the logic can be more clear; re-implement S-Tracer with the latest version of LLVM (previous version of S-Tracer is based on LLVM 3.5); optimize S-Detector for more efficient memory bug checking, including using better algorithm for static analysis and implementing a faster runtime library.
