#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Configure and build the iohttpparser uftrace benchmark binary."""

from __future__ import annotations

import os
import subprocess
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    build_dir = Path(
        os.environ.get("BUILD_DIR", str(root / "build" / "uftrace-clang-pg"))
    )
    subprocess.run(
        [
            "cmake",
            "-S",
            str(root),
            "-B",
            str(build_dir),
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++",
            "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
            "-DIOHTTPPARSER_BUILD_BENCH=ON",
            "-DIOHTTPPARSER_BUILD_TESTS=ON",
            "-DIOHTTPPARSER_BUILD_FUZZ=OFF",
            "-DCMAKE_C_FLAGS_RELWITHDEBINFO=-O2 -g -pg",
            "-DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO=-pg",
        ],
        cwd=root,
        check=True,
    )
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--target", "bench_throughput_compare"],
        cwd=root,
        check=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
