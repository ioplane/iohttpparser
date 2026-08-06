#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build reproducible iohttpparser release archives and checksums."""

from __future__ import annotations

import hashlib
import os
import subprocess
import sys
from pathlib import Path


def run(command: list[str], *, root: Path) -> None:
    subprocess.run(command, cwd=root, check=True)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    dist = root / "dist"
    tag = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("GITHUB_REF_NAME", "dev")
    prefix = f"iohttpparser-{tag}"
    dist.mkdir(parents=True, exist_ok=True)
    for name in (
        f"{prefix}.tar.gz",
        f"{prefix}.zip",
        f"{prefix}-docs.tar.gz",
        f"{prefix}-verification.tar.gz",
        f"{prefix}.sha256",
        "RELEASE_NOTES.md",
    ):
        (dist / name).unlink(missing_ok=True)
    run(
        [
            "git",
            "archive",
            "--format=tar.gz",
            f"--prefix={prefix}/",
            "-o",
            str(dist / f"{prefix}.tar.gz"),
            "HEAD",
        ],
        root=root,
    )
    run(
        [
            "git",
            "archive",
            "--format=zip",
            f"--prefix={prefix}/",
            "-o",
            str(dist / f"{prefix}.zip"),
            "HEAD",
        ],
        root=root,
    )
    run(
        ["cmake", "--preset", "clang-debug", "-DIOHTTPPARSER_BUILD_EXAMPLES=ON"],
        root=root,
    )
    run(["cmake", "--build", "--preset", "clang-debug", "--target", "docs"], root=root)
    docs = root / "docs" / "api" / "html"
    if docs.is_dir():
        run(
            ["tar", "-C", str(docs), "-czf", str(dist / f"{prefix}-docs.tar.gz"), "."],
            root=root,
        )
    artifacts = root / "tests" / "artifacts"
    if (artifacts / "pmi-psi").is_dir() and (artifacts / "release-candidate").is_dir():
        run(
            [
                "tar",
                "-C",
                str(artifacts),
                "-czf",
                str(dist / f"{prefix}-verification.tar.gz"),
                "pmi-psi",
                "release-candidate",
            ],
            root=root,
        )
    run(
        ["uv", "run", "--script", str(root / "scripts" / "release_notes.py"), tag],
        root=root,
    )
    archives = sorted(
        path
        for path in dist.iterdir()
        if path.is_file()
        and path.name.startswith(prefix)
        and not path.name.endswith(".sha256")
    )
    (dist / f"{prefix}.sha256").write_text(
        "\n".join(
            f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}"
            for path in archives
        )
        + "\n",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
