#!/usr/bin/env bash
set -eu

. ./build.sh "${1}" "${2}"

cmake_folder="cmake-build-${1}"
example_name="examples/${4}"
example_bc="${cmake_folder}/${example_name}.bc"
example_opt_bc="${cmake_folder}/${example_name}_opt.bc"

mkdir -p "${cmake_folder}/examples"

clang -emit-llvm "-O${5}" -c -o "${example_bc}" "${example_name}.c" ${@:6}
opt -load-pass-plugin="${cmake_folder}/${2}/${2}.so" -passes="${3}" -o "${example_opt_bc}" "${example_bc}"
clang "-O${5}" -o "${cmake_folder}/${example_name}" "${example_opt_bc}" "lib/${3}.cpp"
