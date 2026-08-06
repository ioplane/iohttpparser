#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run the supported iohttpparser profiling workflows."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
from pathlib import Path


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def require_command(name: str) -> None:
    if shutil.which(name) is None:
        raise SystemExit(f"missing tool: {name}")


def require_executable(path: Path, message: str) -> None:
    if not path.is_file() or not path.stat().st_mode & 0o111:
        raise SystemExit(f"{message}: {path}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "tool", choices=("trace", "uftrace", "valgrind", "gdb", "ftracer")
    )
    parser.add_argument("scenario", nargs="?", default="req-pico-bench")
    parser.add_argument("iterations", nargs="?", default="50000")
    parser.add_argument("parser", nargs="?", default="iohttpparser-strict")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = root_dir()
    results_dir = root / "docs" / "tmp" / "profiling"
    results_dir.mkdir(parents=True, exist_ok=True)
    benchmark = root / "build" / "clang-debug" / "bench_throughput_compare"
    trace = root / "build" / "trace-clang" / "bench_throughput_compare"
    uftrace = (
        Path(
            os.environ.get(
                "UFTRACE_BUILD_DIR", str(root / "build" / "uftrace-clang-pg")
            )
        )
        / "bench_throughput_compare"
    )

    if args.tool == "trace":
        require_executable(
            trace, "trace build not found; build with IOHTTPPARSER_PERF_TRACE=ON first"
        )
        subprocess.run(
            [
                str(trace),
                args.iterations,
                "--trace-parser",
                args.parser,
                "--trace-scenario",
                args.scenario,
            ],
            cwd=root,
            check=True,
        )
        return 0

    if args.tool == "uftrace":
        require_command("uftrace")
        require_executable(
            uftrace,
            "uftrace benchmark binary not found; build it with scripts/build_uftrace_bench.py",
        )
        output_dir = results_dir / f"uftrace-{args.scenario}-{args.parser}"
        print(f"Running uftrace record into {output_dir}")
        subprocess.run(
            [
                "uftrace",
                "record",
                "-d",
                str(output_dir),
                "--",
                str(uftrace),
                args.iterations,
                "--tsv",
                "--scenario",
                args.scenario,
                "--parser",
                args.parser,
            ],
            cwd=root,
            check=True,
        )
        return 0

    if args.tool == "valgrind":
        require_command("valgrind")
        require_executable(benchmark, "benchmark binary not found")
        output_file = results_dir / f"valgrind-{args.scenario}-{args.parser}.log"
        print(f"Running Valgrind Memcheck into {output_file}")
        subprocess.run(
            [
                "valgrind",
                "--tool=memcheck",
                "--leak-check=full",
                "--track-origins=yes",
                f"--log-file={output_file}",
                str(benchmark),
                args.iterations,
                "--tsv",
                "--scenario",
                args.scenario,
                "--parser",
                args.parser,
            ],
            cwd=root,
            check=True,
        )
        return 0

    if args.tool == "gdb":
        require_command("gdb")
        require_executable(benchmark, "benchmark binary not found")
        subprocess.run(
            [
                "gdb",
                "--args",
                str(benchmark),
                args.iterations,
                "--tsv",
                "--scenario",
                args.scenario,
                "--parser",
                args.parser,
            ],
            cwd=root,
            check=True,
        )
        return 0

    print(
        "ftracer is not wired as a default runnable mode.\n\n"
        "Recommended usage for this repository:\n"
        "1. Build a dedicated GCC-only profiling binary with:\n"
        "   -pg -mfentry -rdynamic\n"
        "2. Link with the external ftracer object as described by upstream.\n"
        "3. Use it only for narrow control-flow tracing of hot scenarios, not for throughput numbers.\n\n"
        "This is intentionally not automated here because it requires a separate instrumented build and\n"
        "introduces large per-call overhead."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
