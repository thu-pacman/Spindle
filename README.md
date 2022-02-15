# Spindle

Spindle is an efficient memory access monitoring framework. Unlike methods delaying all checks to runtime or performing task-specific optimization at compile time, Spindle performs common static analysis to identify predictable memory access patterns into a compact program structure summary. Custom memory monitoring tools can then be developed on top of Spindle, leveraging the structural information extracted to dramatically reduce the amount of instrumentation that incurs heavy runtime memory address examination or recording.

## Requirements:
LLVM 15.0.0

## Installation as an out-of-tree pass:
```shell
cd Spindle/
LLVM_DIR="path/to/llvm/lib/cmake/llvm" cmake .
# LLVM_DIR should contains the file named “LLVMConfig.cmake” or “llvm-config.cmake”
```

<!-- ## Compile your own code: -->
<!-- ```shell -->
<!-- # clang -Xclang -load -Xclang mypass.so main.c -c -O2 -->
<!-- ``` -->

## Run test:
<!-- ```shell
cd test/
clang -Xclang -load -Xclang ../Spindle/Spindle/libSpindlePass.so -O2 -c testloop.c -o testloop.o
clang -O2 testloop.o ../lib/sdetector_lib.c -o testloop
./testloop
``` -->
```shell
clang -emit-llvm -O2 -c test_loop_out_of_bound.c
opt -load-pass-plugin=../build/Spindle/libSpindlePass.so -passes="spindle" test_loop_out_of_bound.bc -o opt_test.bc
clang opt_test.bc ../lib/sdetector_lib.c -O2 -o testloop
```

Load Spindle pass directly using clang is currently not supported since this version do not support LLVM's new PassManager.
Supporting for new PassManager will be updated in the future version.

Now Spindle is still under updating: divide the Spindle common static to different modules so that the logic can be more clear; re-implement S-Tracer with the latest version of LLVM (previous version of S-Tracer is based on LLVM 3.5); optimize S-Detector for more efficient memory bug checking, including using better algorithm for static analysis and implementing a faster runtime library.
