#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run the iohttpparser release quality and compatibility gate."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    commands = (
        [sys.executable, str(root / "scripts" / "quality.py")],
        [
            "ctest",
            "--preset",
            "clang-debug",
            "--output-on-failure",
            "-R",
            "test_differential_corpus|test_semantics_differential|test_iohttp_integration",
        ],
        ["cmake", "--build", "--preset", "clang-debug", "--target", "docs"],
        [sys.executable, str(root / "scripts" / "lint-docs.py")],
        [sys.executable, str(root / "scripts" / "check-pmi-psi-artifacts.py")],
    )
    for command in commands:
        subprocess.run(command, cwd=root, check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
