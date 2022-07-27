#!/usr/bin/env bash
set -eu

cmake_folder="cmake-build-${1}"

cmake --toolchain "toolchains/${1}.cmake" -G Ninja -B "${cmake_folder}" -S .

cmake --build "${cmake_folder}" --target ${@:2}
