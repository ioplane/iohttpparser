#!/usr/bin/env python3
"""Curated RFC downloader for iohttpparser.

This script is derived from the broader iohttp RFC scraper, but is trimmed down
for parser-centric needs:
- no datatracker API search
- no external Python dependencies
- curated RFC profiles for parser development and integration context
- local README generation for docs/rfc

Usage:
    python3 deploy/podman/scripts/rfc-scraper.py --download docs/rfc
    python3 deploy/podman/scripts/rfc-scraper.py --download docs/rfc --profile all
    python3 deploy/podman/scripts/rfc-scraper.py --readme docs/rfc/README.md
"""

from __future__ import annotations

import argparse
import logging
import sys
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

log = logging.getLogger("rfc-scraper")

RFC_EDITOR_BASE = "https://www.rfc-editor.org"
USER_AGENT = "iohttpparser-rfc-scraper/1.0"
REQUEST_TIMEOUT = 30
RETRY_DELAY = 1.0
MAX_RETRIES = 3


@dataclass(frozen=True)
class RfcSpec:
    number: int
    title: str
    group: str
    rationale: str
    priority: str

    @property
    def url(self) -> str:
        return f"{RFC_EDITOR_BASE}/rfc/rfc{self.number}"

    @property
    def txt_url(self) -> str:
        return f"{RFC_EDITOR_BASE}/rfc/rfc{self.number}.txt"

    @property
    def filename(self) -> str:
        return f"rfc{self.number}.txt"


RFC_SPECS: tuple[RfcSpec, ...] = (
    RfcSpec(9110, "HTTP Semantics", "Core", "Field semantics and method rules.", "P0"),
    RfcSpec(9111, "HTTP Caching", "Core", "Useful boundary for cache-related headers.", "P2"),
    RfcSpec(9112, "HTTP/1.1", "Core", "Primary wire-format and framing reference.", "P0"),
    RfcSpec(3986, "Uniform Resource Identifier (URI): Generic Syntax", "Core", "Request-target structure and decomposition.", "P0"),
    RfcSpec(5234, "Augmented BNF for Syntax Specifications: ABNF", "Core", "Grammar notation used by HTTP RFCs.", "P1"),
    RfcSpec(7405, "Case-Sensitive String Support in ABNF", "Core", "ABNF extensions relevant to parser interpretation.", "P1"),
    RfcSpec(6265, "HTTP State Management Mechanism", "Adjacent", "Cookie semantics to keep outside parser core.", "P2"),
    RfcSpec(6455, "The WebSocket Protocol", "Adjacent", "Handshake-related behavior after HTTP upgrade.", "P2"),
    RfcSpec(7239, "Forwarded HTTP Extension", "Adjacent", "Proxy metadata semantics relevant to consumers.", "P2"),
    RfcSpec(7578, "multipart/form-data", "Adjacent", "Body-level parsing intentionally outside core.", "P2"),
    RfcSpec(9113, "HTTP/2", "Integration", "Version-neutral semantic layer context for iohttp.", "P3"),
    RfcSpec(9114, "HTTP/3", "Integration", "Version-neutral semantic layer context for iohttp.", "P3"),
)

PROFILES: dict[str, tuple[int, ...]] = {
    "core": (9110, 9112, 3986, 5234, 7405),
    "adjacent": (6265, 6455, 7239, 7578),
    "integration": (9113, 9114, 9111),
    "all": tuple(spec.number for spec in RFC_SPECS),
}


def _spec_map() -> dict[int, RfcSpec]:
    return {spec.number: spec for spec in RFC_SPECS}


def specs_for_profile(profile: str) -> list[RfcSpec]:
    spec_map = _spec_map()
    return [spec_map[number] for number in PROFILES[profile]]


def fetch_bytes(url: str) -> bytes:
    req = Request(url, headers={"User-Agent": USER_AGENT})
    last_error: Exception | None = None

    for attempt in range(1, MAX_RETRIES + 1):
        try:
            with urlopen(req, timeout=REQUEST_TIMEOUT) as resp:
                return resp.read()
        except (HTTPError, URLError) as exc:
            last_error = exc
            log.warning("Fetch failed (%d/%d) for %s: %s", attempt, MAX_RETRIES, url, exc)
            if attempt < MAX_RETRIES:
                time.sleep(RETRY_DELAY * attempt)

    assert last_error is not None
    raise last_error


def download_rfc(spec: RfcSpec, dest_dir: Path, *, force: bool) -> Path:
    dest = dest_dir / spec.filename
    if dest.exists() and not force:
        log.info("Keep existing %s", dest.name)
        return dest

    content = fetch_bytes(spec.txt_url)
    dest.write_bytes(content)
    log.info("Downloaded RFC %d -> %s", spec.number, dest)
    return dest


def generate_readme(specs: Iterable[RfcSpec], download_dir: Path) -> str:
    generated = datetime.now(tz=timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    lines: list[str] = []
    w = lines.append

    w("# RFC Index\n")
    w(f"Generated: {generated}")
    w("")
    w("This directory stores curated RFC references for `iohttpparser`.")
    w("The set is intentionally parser-centric, not a full HTTP ecosystem mirror.")
    w("")
    w("## Priority Guide\n")
    w("- `P0`: required for parser core and default policy behavior")
    w("- `P1`: important supporting references")
    w("- `P2`: adjacent semantics kept outside parser core")
    w("- `P3`: integration context for consumers such as `iohttp`")
    w("")
    w("## RFCs\n")
    w("| RFC | Priority | Group | Local file | Purpose |")
    w("|-----|----------|-------|------------|---------|")

    for spec in sorted(specs, key=lambda item: (item.priority, item.number)):
        local = download_dir / spec.filename
        local_label = spec.filename if local.exists() else "(not downloaded)"
        w(
            f"| [{spec.number}]({spec.url}) | {spec.priority} | {spec.group} | "
            f"{local_label} | {spec.rationale} |"
        )

    w("")
    w("## Profiles\n")
    for profile, numbers in PROFILES.items():
        refs = ", ".join(f"RFC {number}" for number in numbers)
        w(f"- `{profile}`: {refs}")

    return "\n".join(lines)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Curated RFC downloader for iohttpparser",
    )
    parser.add_argument(
        "--profile",
        choices=sorted(PROFILES),
        default="all",
        help="RFC profile to use (default: all)",
    )
    parser.add_argument(
        "--download",
        type=Path,
        default=None,
        metavar="DIR",
        help="Download RFC .txt files into DIR",
    )
    parser.add_argument(
        "--readme",
        type=Path,
        default=None,
        metavar="FILE",
        help="Write Markdown index to FILE",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-download existing RFC files",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase verbosity (-v info, -vv debug)",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)

    level = logging.WARNING
    if args.verbose >= 2:
        level = logging.DEBUG
    elif args.verbose >= 1:
        level = logging.INFO

    logging.basicConfig(
        level=level,
        format="%(levelname)-5s %(message)s",
        stream=sys.stderr,
    )

    specs = specs_for_profile(args.profile)

    if args.download is not None:
        args.download.mkdir(parents=True, exist_ok=True)
        for spec in specs:
            download_rfc(spec, args.download, force=args.force)

    if args.readme is not None:
        readme_dir = args.download if args.download is not None else args.readme.parent
        args.readme.parent.mkdir(parents=True, exist_ok=True)
        args.readme.write_text(generate_readme(specs, readme_dir), encoding="utf-8")
        log.info("Wrote %s", args.readme)

    if args.download is None and args.readme is None:
        sys.stdout.write(generate_readme(specs, Path(".")))
        sys.stdout.write("\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
