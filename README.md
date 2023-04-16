# Spindle

Spindle is an efficient memory access monitoring framework. Unlike methods delaying all checks to runtime or performing task-specific optimization at compile time, Spindle performs common static analysis to identify predictable memory access patterns into a compact program structure summary. Custom memory monitoring tools can then be developed on top of Spindle, leveraging the structural information extracted to dramatically reduce the amount of instrumentation that incurs heavy runtime memory address examination or recording.

## Requirements
LLVM (tested on 14.0.0)

## Installation
```shell
git clone https://github.com/thu-pacman/Spindle
cd Spindle
cmake .
make -j
```

## Run Test
To compile a single file `1.c` with spindle analysis:

```shell
clang 1.c -lstracer -L/path/to/Spindle/folder -fpass-plugin=/path/to/SpindlePass.so -o 1
```

For example, in the following directory structure,

```
├── 1.c
└── Spindle
    ├── libstracer.so
    └── SpindlePass
        └── SpindlePass.so
```

the compilation command will be

```shell
clang 1.c -lstracer -L`realpath Spindle` -fpass-plugin=`realpath Spindle/SpindelPass.so` -o 1
```

If compiled with optimization level `-O2` and above, it is highly recommended to add `-fno-unroll-loops -fno-vectorize` to avoid loop analysis failure.

### Spindle  Arguments

Currently Spindle supports the following arguments:

+ `-full_mem`, perform a full memory access instrumentation
+ `-full_br`, perform a full branch result instrumentation

To add `-arg` during compilation of a program, users should first add `-Xclang -load -Xclang /path/to/SpindlePass.so` to the clang compilation flag, and then add `-mllvm -arg` for each argument. For example, compiling `1.c` in the upper directory structure with both `-full_mem` and `-full_br`:

```shell
clang 1.c -lstracer -L`realpath Spindle` -fpass-plugin=Spindle/SpindelPass.so -o 1 -Xclang -load -Xclang SpindlePass/SpindlePass.so -mllvm -full_br -mllvm -full_mem
```

