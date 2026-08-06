#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Validate the machine-readable scanner benchmark contract."""

from __future__ import annotations

import os
import subprocess
import tempfile
from pathlib import Path


def parse_rows(output: str) -> list[list[str]]:
    return [line.split("\t") for line in output.splitlines() if line]


def find_meta(rows: list[list[str]], name: str) -> str:
    for row in rows:
        if len(row) == 3 and row[0] == "meta" and row[1] == name:
            return row[2]
    raise SystemExit(f"missing metadata field: {name}")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    iterations = os.environ.get("ITERATIONS", "3000")
    if not iterations.isdigit() or int(iterations) < 1:
        raise SystemExit("ITERATIONS must be a positive integer")
    output = tempfile.NamedTemporaryFile(
        mode="w+",
        encoding="utf-8",
        prefix="iohttpparser-scanner-bench-",
        delete=False,
    )
    output_path = Path(output.name)
    output.close()
    try:
        environment = os.environ.copy()
        environment["FORMAT"] = "tsv"
        environment["ITERATIONS"] = iterations
        with output_path.open("w", encoding="utf-8") as stream:
            subprocess.run(
                [
                    "uv",
                    "run",
                    "--script",
                    str(root / "scripts" / "run_scanner_bench.py"),
                ],
                cwd=root,
                check=True,
                env=environment,
                stdout=stream,
            )
        rows = parse_rows(output_path.read_text(encoding="utf-8"))
    finally:
        output_path.unlink(missing_ok=True)

    if [row for row in rows if row[:3] == ["format", "tsv", "v1"]] == []:
        raise SystemExit("missing TSV format marker")
    if find_meta(rows, "iterations") != iterations:
        raise SystemExit("benchmark iteration metadata does not match ITERATIONS")

    simd_level = find_meta(rows, "simd_level")
    expected_backend = {
        "0x0": "scalar",
        "0x1": "sse42",
        "0x2": "avx2",
        "0x3": "avx2",
    }.get(simd_level)
    if expected_backend is None:
        raise SystemExit(f"unexpected simd level: {simd_level}")
    active_backend = find_meta(rows, "active_backend")
    if active_backend != expected_backend:
        raise SystemExit(
            f"expected active backend {expected_backend} for simd level {simd_level}, "
            f"got {active_backend}"
        )

    required_columns = [
        "columns",
        "backend",
        "operation",
        "case",
        "len",
        "elapsed_ns",
        "ns_per_op",
    ]
    if required_columns not in rows:
        raise SystemExit("missing benchmark columns marker")
    for marker in (
        ("dispatch", "find", "request-space"),
        ("scalar", "token", "method"),
    ):
        if not any(row[:3] == list(marker) for row in rows):
            raise SystemExit(f"missing benchmark row: {' '.join(marker)}")
    find_meta(rows, "sinks")

    case_lines = sum(
        row and row[0] in {"dispatch", "scalar", "sse42", "avx2"} for row in rows
    )
    if case_lines < 32:
        raise SystemExit(f"expected at least 32 benchmark rows, got {case_lines}")
    print(f"scanner bench output verified ({case_lines} rows)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
