#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Render deterministic iohttpparser release notes."""

from __future__ import annotations

import os
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    tag = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("GITHUB_REF_NAME", "dev")
    rc_run_id = (
        (root / "tests/artifacts/release-candidate/latest.txt")
        .read_text(encoding="utf-8")
        .strip()
    )
    psi_run_id = (
        (root / "tests/artifacts/pmi-psi/latest.txt")
        .read_text(encoding="utf-8")
        .strip()
    )
    (root / "dist").mkdir(parents=True, exist_ok=True)
    (root / "dist/RELEASE_NOTES.md").write_text(
        f"""# iohttpparser {tag}

## Scope

- HTTP/1.1 wire-level parser for C23
- strict parser, semantics, and body-decoder contracts
- consumer-ready contract for iohttp and ioguard

## Verification

- release gate: `scripts/release_gate.py`
- release-candidate evidence: `{rc_run_id}`
- PMI/PSI evidence: `{psi_run_id}`

## Published Assets

- source tarball
- source zip archive
- generated API reference archive
- verification artifact archive
- checksums

## References

- common PSI: `docs/en/09-test-results.md`
- extended contract: `docs/en/11-extended-contract-results.md`
- release-candidate checklist: `docs/en/12-release-candidate-checklist.md`
""",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
