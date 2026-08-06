#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build and run the iohttpparser scanner benchmark."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    preset = os.environ.get("BUILD_PRESET", "clang-release")
    build_dir = Path(os.environ.get("BUILD_DIR", str(root / "build" / preset)))
    iterations = os.environ.get("ITERATIONS", "200000")
    output_format = os.environ.get("FORMAT", "human")
    if not iterations.isdigit() or int(iterations) < 1:
        raise SystemExit("ITERATIONS must be a positive integer")
    if output_format not in {"human", "tsv"}:
        raise SystemExit("FORMAT must be human or tsv")

    cmake_configure = [
        "cmake",
        "--preset",
        preset,
        "-DIOHTTPPARSER_BUILD_BENCH=ON",
        "-DIOHTTPPARSER_BUILD_TESTS=OFF",
    ]
    cmake_build = ["cmake", "--build", str(build_dir), "--target", "bench_parser"]
    if output_format == "tsv":
        subprocess.run(cmake_configure, cwd=root, check=True, stdout=sys.stderr)
        subprocess.run(cmake_build, cwd=root, check=True, stdout=sys.stderr)
    else:
        subprocess.run(cmake_configure, cwd=root, check=True)
        subprocess.run(cmake_build, cwd=root, check=True)

    benchmark = [str(build_dir / "bench_parser"), iterations]
    if output_format == "tsv":
        benchmark.append("--tsv")
    subprocess.run(benchmark, cwd=root, check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
