#!/usr/bin/env bash
set -eu

cmake_folder="cmake-build-${1}"
example_name="examples/${3}"
example_bc="${cmake_folder}/${example_name}.bc"
example_opt_bc="${cmake_folder}/${example_name}_opt.bc"

mkdir -p "${cmake_folder}/examples"

clang -emit-llvm -O2 -c -o "${example_bc}" "${example_name}.c"
opt -load-pass-plugin="${cmake_folder}/SpindlePass/SpindlePass.so" -passes="${2}" -o "${example_opt_bc}" "${example_bc}"
clang -O2 -o "${cmake_folder}/${example_name}" "${example_opt_bc}" "lib/${4:-${2}}.cpp"
