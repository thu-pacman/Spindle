#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__all__ = ()

import argparse
import os.path
import subprocess
import sys
from pathlib import Path
from typing import Any, List


def command(*args: Any) -> Any:
    return subprocess.run(args, stdout=sys.stdout, stderr=sys.stderr, check=True)


def cmake_command(*args: Any) -> Any:
    return command("cmake", *args)


def main() -> int:
    parser = argparse.ArgumentParser(description="Build the Spindle compiler analysis tool.")

    parser.add_argument(
        "-T", "--toolchain",
        default="ubuntu",
        help="The toolchain file in ./toolchains for CMake."
    )
    parser.add_argument(
        "-G", "--generator",
        default="Ninja",
        help="The generator to use for CMake."
    )
    parser.add_argument(
        "-E", "--experimental",
        action="store_true",
        default=False,
        help="Build the experimental version of the tool (WIP)."
    )
    parser.add_argument(
        "-P", "--passes",
        action="extend",
        nargs="+",
        required=True,
        help="The passes to use (must be defined in Spindle)."
    )
    parser.add_argument(
        "--inject",
        action="append",
        nargs=2,
        help="Inject external source files for the pass."
    )
    parser.add_argument(
        "-e", "--examples",
        action="extend",
        nargs="+",
        help="The examples to build."
    )
    parser.add_argument(
        "-O", "--clang-opt-level",
        required=True,
        help="The optimisation level for the clang compiler."
    )
    parser.add_argument(
        "--clang-f-extra",
        action="extend",
        nargs="+",
        help="Extra arguments for the clang compiler."
    )

    parsed = parser.parse_args()

    toolchain_path = Path("toolchains") / f"{parsed.toolchain}.cmake"
    if not toolchain_path.exists():
        parser.error(f"Toolchain {toolchain_path} does not exist.")

    cmake_folder = Path(f"cmake-build-{parsed.toolchain}")

    cmake_command("--toolchain", toolchain_path, "-G", parsed.generator, "-B", cmake_folder, "-S", ".")

    if parsed.experimental:
        target = "Spindle"
        target_path = cmake_folder / "Spindle.so"
    else:
        target = "SpindlePass"
        target_path = cmake_folder / "SpindlePass/SpindlePass.so"

    cmake_command("--build", cmake_folder, "--target", target)

    examples_path = Path("examples")
    example_output = cmake_folder / "examples"
    example_output.mkdir(0o755, False, True)

    language_options = {
        ".cpp": ("-x", "c++", "-std=c++20"),
        ".cxx": ("-x", "c++", "-std=c++20"),
        ".cc": ("-x", "c++", "-std=c++20"),
        ".c": ("-x", "c", "-std=c17")
    }

    optimisation = f"-O{parsed.clang_opt_level}"
    options = tuple(
        f"-f{option}"
        for option in (parsed.clang_f_extra or ())
    )
    passes = ",".join(parsed.passes)
    injections_map = {
        k: Path(v)
        for k, v in (parsed.inject or ())
    }
    for k, v in injections_map.items():
        if not v.exists():
            parser.error(f"Injection file {v} for pass {k} does not exist.")
    injections = []
    for p in parsed.passes:
        i = injections_map.get(p)
        if i is not None:
            injections.append(i)
    injections = tuple(injections)

    if parsed.examples:
        for example in parsed.examples:
            example_path = examples_path / example
            if not example_path.exists():
                parser.error(f"Example {example_path} does not exist.")

            example_base, example_ext = os.path.splitext(example)

            language = language_options.get(example_ext)
            if language is None:
                parser.error(f"Unrecognised suffix of {example_path}.")

            example_bc = example_output / f"{example_base}.bc"
            example_opt_bc = example_output / f"{example_base}-opt.bc"

            command("clang", *language, optimisation, *options, "-emit-llvm", "-c", "-o", example_bc, example_path)
            command("opt", "--load-pass-plugin", target_path, f"--passes={passes}", "-o", example_opt_bc, example_bc)
            command("clang++", optimisation, "-o", example_output / example_base, example_opt_bc, *injections)

    return 0


if __name__ == "__main__":
    sys.exit(main())
