#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build, run, and summarize iohttpparser throughput benchmarks."""

from __future__ import annotations

import argparse
import csv
import os
import shlex
import shutil
import statistics
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def positive_env(name: str, default: str) -> str:
    value = os.environ.get(name, default)
    if not value.isdigit() or int(value) < 1:
        raise SystemExit(f"{name} must be a positive integer")
    return value


def run_compare(extra_args: list[str]) -> int:
    root = root_dir()
    preset = os.environ.get("BUILD_PRESET", "clang-release")
    build_dir = Path(os.environ.get("BUILD_DIR", str(root / "build" / preset)))
    iterations = positive_env("ITERATIONS", "200000")
    command = [
        "cmake",
        "--preset",
        preset,
        "-DIOHTTPPARSER_BUILD_BENCH=ON",
        "-DIOHTTPPARSER_BUILD_TESTS=OFF",
    ]
    subprocess.run(command, cwd=root, check=True)
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--target", "bench_throughput_compare"],
        cwd=root,
        check=True,
    )
    arguments = [iterations]
    if os.environ.get("FORMAT", "human") == "tsv":
        arguments.append("--tsv")
    if os.environ.get("CONNECT_ONLY", "0") == "1":
        arguments.append("--connect-only")
    arguments.extend(extra_args)
    subprocess.run(
        [str(build_dir / "bench_throughput_compare"), *arguments], cwd=root, check=True
    )
    return 0


def run_extended(_: list[str]) -> int:
    root = root_dir()
    build_dir = Path(os.environ.get("BUILD_DIR", str(root / "build" / "clang-release")))
    iterations = positive_env("ITERATIONS", "200000")
    arguments = [f"--iterations={iterations}"]
    if os.environ.get("FORMAT", "human") == "tsv":
        arguments.append("--format=tsv")
    scenario = os.environ.get("SCENARIO", "")
    if scenario:
        arguments.append(f"--scenario={scenario}")
    subprocess.run(
        [str(build_dir / "bench_extended_contract"), *arguments], cwd=root, check=True
    )
    return 0


def execute_to_text(arguments: list[str], environment: dict[str, str]) -> str:
    result = subprocess.run(
        ["uv", "run", "--script", str(Path(__file__).resolve()), *arguments],
        cwd=root_dir(),
        check=True,
        env=environment,
        text=True,
        stdout=subprocess.PIPE,
    )
    return result.stdout


def keep_tsv(text: str, marker: str) -> str:
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if line.startswith(marker):
            return "\n".join(lines[index:]) + "\n"
    raise SystemExit(f"benchmark output marker not found: {marker}")


@dataclass
class CompareRow:
    parser: str
    scenario: str
    kind: str
    length: int
    req_per_s: list[float] = field(default_factory=list)
    mib_per_s: list[float] = field(default_factory=list)
    ns_per_req: list[float] = field(default_factory=list)


@dataclass
class ExtendedRow:
    group: str
    scenario: str
    baseline: str
    wire_len: int
    work_len: int
    req_per_s: list[float] = field(default_factory=list)
    mib_per_s: list[float] = field(default_factory=list)
    ns_per_op: list[float] = field(default_factory=list)


def median_compare(path: Path, rows: dict[tuple[str, str], CompareRow]) -> None:
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream, delimiter="\t")
        for row in reader:
            if len(row) < 8 or row[0] in {"format", "meta", "columns"}:
                continue
            key = (row[0], row[1])
            item = rows.setdefault(key, CompareRow(row[0], row[1], row[2], int(row[3])))
            item.req_per_s.append(float(row[5]))
            item.mib_per_s.append(float(row[6]))
            item.ns_per_req.append(float(row[7]))


def median_extended(path: Path, rows: dict[tuple[str, str], ExtendedRow]) -> None:
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream, delimiter="\t")
        for row in reader:
            if len(row) < 9 or row[0] == "scenario_group":
                continue
            key = (row[0], row[1])
            item = rows.setdefault(
                key,
                ExtendedRow(row[0], row[1], row[2], int(row[3]), int(row[4])),
            )
            item.req_per_s.append(float(row[6]))
            item.mib_per_s.append(float(row[7]))
            item.ns_per_op.append(float(row[8]))


def run_median(extended: bool) -> int:
    runs = int(positive_env("RUNS", "5"))
    temporary = (
        Path(os.environ["TMP_DIR"])
        if "TMP_DIR" in os.environ
        else Path(tempfile.mkdtemp())
    )
    temporary.mkdir(parents=True, exist_ok=True)
    keep_tmp = os.environ.get("KEEP_TMP", "0") == "1"
    try:
        for run in range(1, runs + 1):
            environment = os.environ.copy()
            environment["FORMAT"] = "tsv"
            arguments = (
                ["extended"]
                if extended
                else ["compare", *shlex.split(os.environ.get("EXTRA_ARGS", ""))]
            )
            text = execute_to_text(arguments, environment)
            marker = "scenario_group\t" if extended else "format\t"
            (temporary / f"run-{run}.tsv").write_text(
                keep_tsv(text, marker), encoding="utf-8"
            )

        if extended:
            rows: dict[tuple[str, str], ExtendedRow] = {}
            for run in range(1, runs + 1):
                median_extended(temporary / f"run-{run}.tsv", rows)
            print(
                "scenario_group\tscenario\tbaseline\twire_len\twork_len\treq_per_s_median\tmib_per_s_median\tns_per_op_median"
            )
            for key in sorted(rows):
                item = rows[key]
                print(
                    f"{item.group}\t{item.scenario}\t{item.baseline}\t{item.wire_len}\t"
                    f"{item.work_len}\t{statistics.median(item.req_per_s):.2f}\t"
                    f"{statistics.median(item.mib_per_s):.2f}\t{statistics.median(item.ns_per_op):.2f}"
                )
        else:
            rows_compare: dict[tuple[str, str], CompareRow] = {}
            for run in range(1, runs + 1):
                median_compare(temporary / f"run-{run}.tsv", rows_compare)
            print(
                "parser\tscenario\tkind\tlen\treq_per_s_median\tmib_per_s_median\tns_per_req_median"
            )
            for key in sorted(rows_compare):
                item = rows_compare[key]
                print(
                    f"{item.parser}\t{item.scenario}\t{item.kind}\t{item.length}\t"
                    f"{statistics.median(item.req_per_s):.2f}\t"
                    f"{statistics.median(item.mib_per_s):.2f}\t"
                    f"{statistics.median(item.ns_per_req):.2f}"
                )
    finally:
        if not keep_tmp:
            if "TMP_DIR" in os.environ:
                shutil.rmtree(temporary)
            else:
                for path in temporary.glob("run-*.tsv"):
                    path.unlink(missing_ok=True)
                temporary.rmdir()
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "command", choices=("compare", "extended", "median", "extended-median")
    )
    parser.add_argument("extra", nargs=argparse.REMAINDER)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.command == "compare":
        return run_compare(args.extra)
    if args.command == "extended":
        return run_extended(args.extra)
    return run_median(args.command == "extended-median")


if __name__ == "__main__":
    raise SystemExit(main())
