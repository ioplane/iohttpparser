#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build and run an iohttpparser LibFuzzer target with its seed corpus."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

TARGETS = {
    "scanner": "scanner",
    "body": "chunked",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target", choices=sorted(TARGETS))
    parser.add_argument(
        "--runs",
        type=int,
        default=int(os.environ.get("RUNS", "256")),
        help="LibFuzzer run budget (default: RUNS or 256)",
    )
    parser.add_argument(
        "--preset",
        default=os.environ.get("PRESET", "clang-fuzz"),
        help="CMake preset (default: PRESET or clang-fuzz)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.runs < 1:
        raise SystemExit("--runs must be positive")
    root = Path(__file__).resolve().parents[1]
    target = TARGETS[args.target]
    build_dir = root / "build" / args.preset
    corpus_dir = root / "tests" / "fuzz" / "corpus" / target
    executable = build_dir / f"fuzz_{target}"
    subprocess.run(["cmake", "--preset", args.preset], cwd=root, check=True)
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--target", f"fuzz_{target}"],
        cwd=root,
        check=True,
    )
    if not executable.is_file():
        raise SystemExit(f"fuzz executable not found: {executable}")
    with tempfile.TemporaryDirectory(
        prefix=f"iohttpparser-{args.target}-fuzz-"
    ) as temporary:
        corpus = Path(temporary)
        if corpus_dir.is_dir():
            for source in corpus_dir.iterdir():
                if source.is_file():
                    shutil.copy2(source, corpus / source.name)
        subprocess.run(
            [str(executable), str(corpus), f"-runs={args.runs}"],
            cwd=root,
            check=True,
        )
    print(f"PASSED: {args.target} fuzz target ({args.runs} runs)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
