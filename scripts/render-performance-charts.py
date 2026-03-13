#!/usr/bin/env python3
from __future__ import annotations

import json
import math
import sys
from pathlib import Path


PICO = "picohttpparser"
LLHTTP = "llhttp"
IHTP = "iohttpparser-stateful-strict"

VENDOR_COLORS = {
    PICO: "#2563eb",
    LLHTTP: "#dc2626",
    IHTP: "#059669",
}

TEXT = "#334155"
MUTED = "#64748b"
GRID = "#cbd5e1"
BG = "transparent"


def parse_tsv(path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    header: list[str] | None = None
    with path.open("r", encoding="utf-8") as fh:
        for raw in fh:
            parts = raw.rstrip("\n").split("\t")
            if not parts or parts[0] in {"", "format", "meta"}:
                continue
            if parts[0] == "columns":
                header = parts[1:]
                continue
            if header is None:
                header = parts
                continue
            rows.append(dict(zip(header, parts, strict=False)))
    return rows


def parse_scanner_tsv(path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    header: list[str] | None = None
    with path.open("r", encoding="utf-8") as fh:
        for raw in fh:
            parts = raw.rstrip("\n").split("\t")
            if not parts or parts[0] in {"", "format", "meta"}:
                continue
            if parts[0] == "columns":
                header = parts[1:]
                continue
            if header is None:
                continue
            rows.append(dict(zip(header, parts, strict=False)))
    return rows


def ensure_manifest_has_charts(out_dir: Path, chart_files: list[str]) -> None:
    manifest = out_dir / "manifest.json"
    if not manifest.exists():
        return
    data = json.loads(manifest.read_text(encoding="utf-8"))
    files = data.setdefault("files", {})
    for rel in chart_files:
        key = rel.replace("/", "_").replace("-", "_").replace(".svg", "")
        files[key] = rel
    manifest.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def fmt_reqs(value: float) -> str:
    return f"{value/1_000_000:.2f}M"


def fmt_ns(value: float) -> str:
    return f"{value:.2f} ns"


def esc(text: str) -> str:
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def render_grouped_vertical_svg(
    title: str,
    categories: list[str],
    series: list[tuple[str, str, list[float]]],
    out_path: Path,
) -> None:
    width = 1240
    height = 720
    left = 80
    right = 40
    top = 90
    bottom = 150
    chart_w = width - left - right
    chart_h = height - top - bottom

    max_v = max(max(values) for _, _, values in series)
    max_v *= 1.1
    ticks = 5
    plot = []

    plot.append(
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" role="img" aria-label="{esc(title)}">'
    )
    plot.append(f'<rect width="{width}" height="{height}" fill="{BG}"/>')
    plot.append(
        f'<text x="{left}" y="36" font-size="26" font-weight="700" fill="{TEXT}">{esc(title)}</text>'
    )

    legend_x = left
    legend_y = 60
    for name, color, _ in series:
        plot.append(
            f'<rect x="{legend_x}" y="{legend_y-12}" width="18" height="18" rx="3" fill="{color}"/>'
        )
        plot.append(
            f'<text x="{legend_x+28}" y="{legend_y+2}" font-size="16" fill="{TEXT}">{esc(name)}</text>'
        )
        legend_x += 250

    for i in range(ticks + 1):
        frac = i / ticks
        y = top + chart_h - chart_h * frac
        value = max_v * frac
        plot.append(
            f'<line x1="{left}" y1="{y:.1f}" x2="{left+chart_w}" y2="{y:.1f}" stroke="{GRID}" stroke-width="1"/>'
        )
        plot.append(
            f'<text x="{left-10}" y="{y+5:.1f}" text-anchor="end" font-size="13" fill="{MUTED}">{fmt_reqs(value)}</text>'
        )

    plot.append(
        f'<line x1="{left}" y1="{top}" x2="{left}" y2="{top+chart_h}" stroke="{MUTED}" stroke-width="1.5"/>'
    )
    plot.append(
        f'<line x1="{left}" y1="{top+chart_h}" x2="{left+chart_w}" y2="{top+chart_h}" stroke="{MUTED}" stroke-width="1.5"/>'
    )

    category_w = chart_w / max(len(categories), 1)
    group_w = category_w * 0.72
    bar_gap = group_w * 0.08
    bar_w = (group_w - bar_gap * (len(series) - 1)) / len(series)

    for idx, category in enumerate(categories):
        gx = left + idx * category_w + (category_w - group_w) / 2
        plot.append(
            f'<text x="{gx + group_w/2:.1f}" y="{top+chart_h+28}" text-anchor="end" transform="rotate(-28 {gx + group_w/2:.1f},{top+chart_h+28})" font-size="13" fill="{TEXT}">{esc(category)}</text>'
        )
        for sidx, (name, color, values) in enumerate(series):
            v = values[idx]
            bar_h = 0 if max_v == 0 else chart_h * (v / max_v)
            x = gx + sidx * (bar_w + bar_gap)
            y = top + chart_h - bar_h
            plot.append(
                f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_w:.1f}" height="{bar_h:.1f}" rx="4" fill="{color}" />'
            )
            plot.append(
                f'<text x="{x + bar_w/2:.1f}" y="{max(y-8, top+12):.1f}" text-anchor="middle" font-size="11" fill="{TEXT}">{fmt_reqs(v)}</text>'
            )

    plot.append("</svg>")
    out_path.write_text("\n".join(plot) + "\n", encoding="utf-8")


def render_single_series_horizontal_svg(
    title: str,
    rows: list[tuple[str, float]],
    color: str,
    out_path: Path,
    formatter=fmt_reqs,
    legend_label: str = "iohttpparser",
) -> None:
    width = 1240
    row_h = 52
    top = 80
    bottom = 40
    left = 320
    right = 120
    chart_h = row_h * len(rows)
    height = top + chart_h + bottom
    chart_w = width - left - right
    max_v = max(v for _, v in rows) * 1.1

    plot = []
    plot.append(
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" role="img" aria-label="{esc(title)}">'
    )
    plot.append(f'<rect width="{width}" height="{height}" fill="{BG}"/>')
    plot.append(
        f'<text x="{left}" y="34" font-size="26" font-weight="700" fill="{TEXT}">{esc(title)}</text>'
    )
    plot.append(
        f'<rect x="{left}" y="48" width="18" height="18" rx="3" fill="{color}"/>'
    )
    plot.append(
        f'<text x="{left+28}" y="62" font-size="16" fill="{TEXT}">{esc(legend_label)}</text>'
    )

    for idx, (label, value) in enumerate(rows):
        y = top + idx * row_h
        plot.append(
            f'<text x="{left-16}" y="{y+24}" text-anchor="end" font-size="14" fill="{TEXT}">{esc(label)}</text>'
        )
        plot.append(
            f'<rect x="{left}" y="{y+8}" width="{chart_w}" height="22" rx="4" fill="#e2e8f0"/>'
        )
        bar_w = 0 if max_v == 0 else chart_w * (value / max_v)
        plot.append(
            f'<rect x="{left}" y="{y+8}" width="{bar_w:.1f}" height="22" rx="4" fill="{color}"/>'
        )
        plot.append(
            f'<text x="{left+bar_w+8:.1f}" y="{y+24}" font-size="13" fill="{TEXT}">{formatter(value)}</text>'
        )

    plot.append("</svg>")
    out_path.write_text("\n".join(plot) + "\n", encoding="utf-8")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: render-performance-charts.py <run-dir>", file=sys.stderr)
        return 2

    out_dir = Path(sys.argv[1]).resolve()
    charts_dir = out_dir / "charts"
    charts_dir.mkdir(parents=True, exist_ok=True)

    throughput = parse_tsv(out_dir / "throughput-median.tsv")
    connect = parse_tsv(out_dir / "throughput-connect-median.tsv")
    extended = parse_tsv(out_dir / "throughput-extended-median.tsv")
    scanner = parse_scanner_tsv(out_dir / "scanner-bench.tsv")

    common_scenarios = ["req-small", "req-headers", "resp-small", "resp-headers", "resp-upgrade"]
    common_map = {(r["parser"], r["scenario"]): float(r["req_per_s_median"]) for r in throughput}
    common_series = [
        (PICO, VENDOR_COLORS[PICO], [common_map[(PICO, s)] for s in common_scenarios]),
        (LLHTTP, VENDOR_COLORS[LLHTTP], [common_map[(LLHTTP, s)] for s in common_scenarios]),
        (IHTP, VENDOR_COLORS[IHTP], [common_map[(IHTP, s)] for s in common_scenarios]),
    ]
    render_grouped_vertical_svg(
        "Common consumer scenarios req/s median",
        common_scenarios,
        common_series,
        charts_dir / "common-three-way.svg",
    )

    connect_map = {(r["parser"], r["scenario"]): float(r["req_per_s_median"]) for r in connect}
    connect_series = [
        (PICO, VENDOR_COLORS[PICO], [connect_map[(PICO, "req-connect")]]),
        (LLHTTP, VENDOR_COLORS[LLHTTP], [connect_map[(LLHTTP, "req-connect")]]),
        (IHTP, VENDOR_COLORS[IHTP], [connect_map[(IHTP, "req-connect")]]),
    ]
    render_grouped_vertical_svg(
        "CONNECT req/s median",
        ["req-connect"],
        connect_series,
        charts_dir / "connect-three-way.svg",
    )

    ext_map = {r["scenario"]: float(r["req_per_s_median"]) for r in extended}

    render_single_series_horizontal_svg(
        "Parser state reuse req/s median",
        [("stateful-reuse-request", ext_map["stateful-reuse-request"])],
        VENDOR_COLORS[IHTP],
        charts_dir / "extended-parser-state.svg",
    )
    render_single_series_horizontal_svg(
        "Semantics and body handoff req/s median",
        [
            ("request-chunked-parse", ext_map["request-chunked-parse"]),
            ("request-chunked-parse-semantics", ext_map["request-chunked-parse-semantics"]),
            ("request-chunked-parse-semantics-body", ext_map["request-chunked-parse-semantics-body"]),
            ("response-fixed-parse-semantics-body", ext_map["response-fixed-parse-semantics-body"]),
        ],
        "#0f766e",
        charts_dir / "extended-semantics-body.svg",
    )
    render_single_series_horizontal_svg(
        "iohttp-style consumer flows req/s median",
        [
            ("consumer-iohttp-expect-trailers", ext_map["consumer-iohttp-expect-trailers"]),
            ("consumer-iohttp-fixed-response", ext_map["consumer-iohttp-fixed-response"]),
            ("consumer-iohttp-pipeline", ext_map["consumer-iohttp-pipeline"]),
        ],
        "#7c3aed",
        charts_dir / "extended-consumer-iohttp.svg",
    )
    render_single_series_horizontal_svg(
        "Upgrade and ioguard-style flows req/s median",
        [
            ("response-upgrade-parse", ext_map["response-upgrade-parse"]),
            ("response-upgrade-parse-semantics", ext_map["response-upgrade-parse-semantics"]),
            ("consumer-ioguard-connect", ext_map["consumer-ioguard-connect"]),
            ("consumer-ioguard-reject-te-cl", ext_map["consumer-ioguard-reject-te-cl"]),
        ],
        "#d97706",
        charts_dir / "extended-upgrade-ioguard.svg",
    )

    scanner_rows_by_backend: dict[str, list[float]] = {}
    for row in scanner:
        backend = row["backend"]
        scanner_rows_by_backend.setdefault(backend, []).append(float(row["ns_per_op"]))

    scanner_avg_rows = []
    for backend in ["dispatch", "scalar", "sse42", "avx2"]:
        values = scanner_rows_by_backend.get(backend)
        if not values:
            continue
        scanner_avg_rows.append((backend, sum(values) / float(len(values))))

    render_single_series_horizontal_svg(
        "Scanner backend average ns/op",
        scanner_avg_rows,
        "#7c3aed",
        charts_dir / "scanner-backends.svg",
        formatter=fmt_ns,
        legend_label="lower is better",
    )

    ensure_manifest_has_charts(
        out_dir,
        [
            "charts/common-three-way.svg",
            "charts/connect-three-way.svg",
            "charts/extended-parser-state.svg",
            "charts/extended-semantics-body.svg",
            "charts/extended-consumer-iohttp.svg",
            "charts/extended-upgrade-ioguard.svg",
            "charts/scanner-backends.svg",
        ],
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
