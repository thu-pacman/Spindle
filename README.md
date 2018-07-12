# Spindle

Spindle is a efficient memory access monitoring framework. Unlike methods delaying all checks to runtime or performing task-specific optimization at compile time, Spindle performs common static analysis to identify predictable memory access patterns into a compact program structure summary. Custom memory monitoring tools can then be developed on top of Spindle, leveraging the structural information extracted to dramatically reduce the amount of instrumentation that incurs heavy runtime memory address examination or recording.

Installation as an out-of-tree pass:

Out-of-tree pass
```
| -- spindle
|  | -- CMakeLists.txt
|  | -- spindle
|    | -- CMakeLists.txt
|    | -- Spindle.cpp
|    | -- OtherSpindleFile.cpp
```

./spindle/CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.4)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_subdirectory(spindle)  # Use your pass name here.
```

./spindle/spindle/CMakeLists.txt
```
add_library(SpindlePass MODULE
    # List your source files here.
    Spindle.cpp
    OtherSpindleFile.cpp
)

# Use C++11 to compile your pass (i.e., supply -std=c++11).
target_compile_features(SpindlePass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(SpindlePass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(SpindlePass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
```

```
LLVM_DIR="../clean-build/lib/cmake/llvm" cmake .
```

LLVM_DIR should contains the file named “LLVMConfig.cmake” or “llvm-config.cmake”

Compile your own code:
```
clang -Xclang -load -Xclang mypass.so main.c
```


Now Spindle is still under updating: devide the Spindle common static to different modules so that the logic can be more clear; re-implement S-Tracer with the latest version of LLVM (previous version of S-Tracer is based on LLVM 3.5); optimize S-Detector for more efficient memory bug checking, including using better algorithm for static analysis and implementing a faster runtime library.
