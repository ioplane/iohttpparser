#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run the focused Sprint 11 parser comparison evidence workflow."""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
from datetime import UTC, datetime
from pathlib import Path


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def positive_env(name: str, default: str) -> str:
    value = os.environ.get(name, default)
    if not value.isdigit() or int(value) < 1:
        raise SystemExit(f"{name} must be a positive integer")
    return value


def capture(command: list[str], root: Path) -> str:
    result = subprocess.run(
        command,
        cwd=root,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return result.stdout


def run_logged(command: list[str], root: Path, output: Path) -> None:
    text = capture(command, root)
    output.write_text(text, encoding="utf-8")
    print(text, end="")


def git_tree_sha(root: Path, path: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(root), "ls-tree", "HEAD", path],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
    )
    fields = result.stdout.split()
    return fields[2][:7] if len(fields) > 2 else "unknown"


def write_host(output: Path) -> None:
    lines = [
        f"date_utc={datetime.now(UTC).isoformat(timespec='seconds')}",
        f"kernel={platform.release()}",
        f"machine={platform.machine()}",
    ]
    if shutil.which("lscpu"):
        lines.extend(
            [
                "--- lscpu ---",
                *subprocess.run(
                    ["lscpu"], check=True, text=True, stdout=subprocess.PIPE
                ).stdout.splitlines()[:40],
            ]
        )
    cpuinfo = Path("/proc/cpuinfo")
    if cpuinfo.is_file():
        flags = next(
            (
                line
                for line in cpuinfo.read_text(encoding="utf-8").splitlines()
                if line.lower().startswith("flags")
            ),
            None,
        )
        if flags:
            lines.extend(["--- cpu flags ---", flags])
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    root = root_dir()
    output_dir = Path(
        os.environ.get("OUT_DIR", str(root / "docs" / "tmp" / "comparison" / "results"))
    )
    preset = os.environ.get("BUILD_PRESET", "clang-debug")
    iterations = positive_env("ITERATIONS", "200000")
    output_dir.mkdir(parents=True, exist_ok=True)
    write_host(output_dir / "host.txt")

    references = [
        f"vendored_pico={git_tree_sha(root, 'tests/third_party/picohttpparser')}",
        f"vendored_llhttp={git_tree_sha(root, 'tests/third_party/llhttp')}",
    ]
    for name, path in (
        ("upstream_pico", root / "docs" / "tmp" / "comparison" / "picohttpparser"),
        ("upstream_llhttp", root / "docs" / "tmp" / "comparison" / "llhttp"),
    ):
        if (path / ".git").is_dir():
            references.append(
                f"{name}="
                + capture(
                    ["git", "-C", str(path), "rev-parse", "--short", "HEAD"], root
                ).strip()
            )
    (output_dir / "references.txt").write_text(
        "\n".join(references) + "\n", encoding="utf-8"
    )

    subprocess.run(["cmake", "--preset", preset], cwd=root, check=True)
    subprocess.run(["cmake", "--build", "--preset", preset], cwd=root, check=True)
    run_logged(
        [
            "ctest",
            "--preset",
            preset,
            "--output-on-failure",
            "-R",
            "test_differential_corpus|test_semantics_differential|test_scanner_backends|test_scanner_corpus|test_iohttp_integration",
        ],
        root,
        output_dir / "ctest.txt",
    )

    environment = os.environ.copy()
    environment["FORMAT"] = "tsv"
    environment["ITERATIONS"] = iterations
    result = subprocess.run(
        ["uv", "run", "--script", str(root / "scripts" / "run_scanner_bench.py")],
        cwd=root,
        check=True,
        env=environment,
        text=True,
        stdout=subprocess.PIPE,
    )
    raw = output_dir / "scanner-bench.raw.txt"
    raw.write_text(result.stdout, encoding="utf-8")
    lines = result.stdout.splitlines()
    marker = next(
        (index for index, line in enumerate(lines) if line.startswith("format\t")), None
    )
    if marker is None:
        raise SystemExit("scanner benchmark TSV marker not found")
    (output_dir / "scanner-bench.tsv").write_text(
        "\n".join(lines[marker:]) + "\n", encoding="utf-8"
    )
    print(f"comparison_results={output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
