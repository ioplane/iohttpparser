#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
from pathlib import Path


REQUIRED_FILES = [
    "manifest.json",
    "run.txt",
    "host.txt",
    "toolchain.txt",
    "ctest.txt",
    "throughput.tsv",
    "throughput-connect.tsv",
    "throughput-median.tsv",
    "throughput-connect-median.tsv",
    "throughput-extended.tsv",
    "throughput-extended-median.tsv",
    "scanner-bench.tsv",
    "charts/common-three-way.svg",
    "charts/connect-three-way.svg",
    "charts/extended-parser-state.svg",
    "charts/extended-semantics-body.svg",
    "charts/extended-consumer-iohttp.svg",
    "charts/extended-upgrade-ioguard.svg",
    "charts/scanner-backends.svg",
    "summary.md",
    "summary-extended.md",
]


def fail(msg: str) -> int:
    print(f"FAIL: {msg}", file=sys.stderr)
    return 1


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    base = root / "tests" / "artifacts" / "pmi-psi"
    latest_file = base / "latest.txt"
    if not latest_file.exists():
        return fail("latest.txt is missing")

    run_id = latest_file.read_text(encoding="utf-8").strip()
    if not run_id:
        return fail("latest.txt is empty")

    run_dir = base / "runs" / run_id
    if not run_dir.exists():
        return fail(f"run directory does not exist: {run_dir}")

    missing = [rel for rel in REQUIRED_FILES if not (run_dir / rel).exists()]
    if missing:
        return fail("missing artifact files: " + ", ".join(missing))

    manifest_path = run_dir / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    files = manifest.get("files", {})
    manifest_missing = [rel for rel in REQUIRED_FILES if rel != "manifest.json" and rel not in files.values()]
    if manifest_missing:
        return fail("manifest does not reference: " + ", ".join(manifest_missing))

    print(f"PASS: PMI/PSI artifact set is complete for run {run_id}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
