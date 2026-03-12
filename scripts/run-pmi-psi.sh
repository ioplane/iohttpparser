#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BASE_DIR="${BASE_DIR:-$ROOT_DIR/tests/artifacts/pmi-psi}"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-$(git -C "$ROOT_DIR" rev-parse --short HEAD)}"
OUT_DIR="${OUT_DIR:-$BASE_DIR/runs/$RUN_ID}"
DEBUG_PRESET="${DEBUG_PRESET:-clang-debug}"
RELEASE_PRESET="${RELEASE_PRESET:-clang-release}"
ITERATIONS="${ITERATIONS:-200000}"
RUNS="${RUNS:-5}"

mkdir -p "$OUT_DIR"
mkdir -p "$BASE_DIR/runs"

if [[ ! -f "$BASE_DIR/README.md" ]]; then
    cat >"$BASE_DIR/README.md" <<'EOF'
# PMI/PSI Artifact Contract

Repository-published functional and performance test artifacts live here.

## Layout

```text
tests/artifacts/pmi-psi/
  README.md
  index.tsv
  latest.txt
  runs/<run_id>/
```

## Required run files

- `manifest.json`
- `run.txt`
- `host.txt`
- `toolchain.txt`
- `ctest.txt`
- `throughput.tsv`
- `throughput-connect.tsv`
- `throughput-median.tsv`
- `throughput-connect-median.tsv`
- `throughput-extended.tsv`
- `throughput-extended-median.tsv`
- `charts/common-three-way.svg`
- `charts/connect-three-way.svg`
- `charts/extended-parser-state.svg`
- `charts/extended-semantics-body.svg`
- `charts/extended-consumer-iohttp.svg`
- `charts/extended-upgrade-ioguard.svg`
- `summary.md`
- `summary-extended.md`
EOF
fi

if [[ ! -f "$BASE_DIR/index.tsv" ]]; then
    printf 'run_id\tdate_utc\tgit_head_short\tstatus\n' >"$BASE_DIR/index.tsv"
fi

{
    echo "date_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "root=$ROOT_DIR"
    echo "run_id=$RUN_ID"
    echo "debug_preset=$DEBUG_PRESET"
    echo "release_preset=$RELEASE_PRESET"
    echo "iterations=$ITERATIONS"
    echo "median_runs=$RUNS"
} >"$OUT_DIR/run.txt"

{
    echo "kernel=$(uname -srvmo)"
    echo "machine=$(uname -m)"
    if command -v lscpu >/dev/null 2>&1; then
        echo "--- lscpu ---"
        lscpu | sed -n '1,80p'
    fi
    if [[ -r /proc/cpuinfo ]]; then
        echo "--- cpu flags ---"
        grep -m1 -i '^flags' /proc/cpuinfo || true
    fi
} >"$OUT_DIR/host.txt"

{
    echo "git_head=$(git -C "$ROOT_DIR" rev-parse HEAD)"
    echo "git_head_short=$(git -C "$ROOT_DIR" rev-parse --short HEAD)"
    echo "clang=$(clang --version | head -n 1)"
    echo "cmake=$(cmake --version | head -n 1)"
    echo "ctest=$(ctest --version | head -n 1)"
    echo "valgrind=$(valgrind --version 2>/dev/null || true)"
    echo "uftrace=$(uftrace --version 2>/dev/null | head -n 1 || true)"
    echo "gdb=$(gdb --version 2>/dev/null | head -n 1 || true)"
} >"$OUT_DIR/toolchain.txt"

cmake --preset "$DEBUG_PRESET"
cmake --build --preset "$DEBUG_PRESET"
ctest --preset "$DEBUG_PRESET" --output-on-failure | tee "$OUT_DIR/ctest.txt"

cmake --preset "$RELEASE_PRESET"
cmake --build --preset "$RELEASE_PRESET" --target bench_throughput_compare bench_extended_contract

FORMAT=tsv ITERATIONS="$ITERATIONS" bash "$ROOT_DIR/scripts/run-throughput-compare.sh" \
    >"$OUT_DIR/throughput.tsv"
FORMAT=tsv CONNECT_ONLY=1 ITERATIONS="$ITERATIONS" bash "$ROOT_DIR/scripts/run-throughput-compare.sh" \
    >"$OUT_DIR/throughput-connect.tsv"
ITERATIONS="$ITERATIONS" RUNS="$RUNS" bash "$ROOT_DIR/scripts/run-throughput-median.sh" \
    >"$OUT_DIR/throughput-median.tsv"
CONNECT_ONLY=1 ITERATIONS="$ITERATIONS" RUNS="$RUNS" bash "$ROOT_DIR/scripts/run-throughput-median.sh" \
    >"$OUT_DIR/throughput-connect-median.tsv"
FORMAT=tsv ITERATIONS="$ITERATIONS" bash "$ROOT_DIR/scripts/run-throughput-extended.sh" \
    >"$OUT_DIR/throughput-extended.tsv"
ITERATIONS="$ITERATIONS" RUNS="$RUNS" bash "$ROOT_DIR/scripts/run-throughput-extended-median.sh" \
    >"$OUT_DIR/throughput-extended-median.tsv"

python3 "$ROOT_DIR/scripts/render-performance-charts.py" "$OUT_DIR"

python3 - "$OUT_DIR" "$RUN_ID" <<'PY'
from __future__ import annotations

import json
import sys
from pathlib import Path

out_dir = Path(sys.argv[1])
run_id = sys.argv[2]

run_meta = {}
for line in (out_dir / "run.txt").read_text(encoding="utf-8").splitlines():
    if "=" in line:
        k, v = line.split("=", 1)
        run_meta[k] = v

def parse_tsv(path: Path):
    rows = []
    header = None
    with path.open("r", encoding="utf-8") as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
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

median_rows = parse_tsv(out_dir / "throughput-median.tsv")
connect_rows = parse_tsv(out_dir / "throughput-connect-median.tsv")
extended_rows = parse_tsv(out_dir / "throughput-extended-median.tsv")

files = {
    "run": "run.txt",
    "host": "host.txt",
    "toolchain": "toolchain.txt",
    "ctest": "ctest.txt",
    "throughput": "throughput.tsv",
    "throughput_connect": "throughput-connect.tsv",
    "throughput_median": "throughput-median.tsv",
    "throughput_connect_median": "throughput-connect-median.tsv",
    "throughput_extended": "throughput-extended.tsv",
    "throughput_extended_median": "throughput-extended-median.tsv",
    "chart_common_three_way": "charts/common-three-way.svg",
    "chart_connect_three_way": "charts/connect-three-way.svg",
    "chart_extended_parser_state": "charts/extended-parser-state.svg",
    "chart_extended_semantics_body": "charts/extended-semantics-body.svg",
    "chart_extended_consumer_iohttp": "charts/extended-consumer-iohttp.svg",
    "chart_extended_upgrade_ioguard": "charts/extended-upgrade-ioguard.svg",
    "summary": "summary.md",
    "summary_extended": "summary-extended.md",
}

(out_dir / "manifest.json").write_text(
    json.dumps(
        {
            "schema_version": 1,
            "run_id": run_id,
            "metadata": run_meta,
            "files": files,
        },
        indent=2,
    )
    + "\n",
    encoding="utf-8",
)

parser_order = [
    "picohttpparser",
    "llhttp",
    "iohttpparser-stateful-strict",
    "iohttpparser-strict",
    "iohttpparser-stateful-lenient",
    "iohttpparser-lenient",
]

scenario_order = [
    "req-small",
    "req-headers",
    "resp-small",
    "resp-headers",
    "resp-upgrade",
    "req-line-only",
    "req-line-hot",
    "req-line-long-target",
    "req-line-connect",
    "req-line-options",
    "req-pico-bench",
    "hdr-common-heavy",
    "hdr-name-heavy",
    "hdr-uncommon-valid",
    "hdr-value-ascii-clean",
    "hdr-value-heavy",
    "hdr-value-obs-text",
    "hdr-value-trim-heavy",
    "hdr-count-04-minimal",
    "hdr-count-16-minimal",
    "hdr-count-32-minimal",
]


def render_reqs_matrix(rows):
    grouped = {}
    for row in rows:
        grouped.setdefault(row["scenario"], {})[row["parser"]] = row["req_per_s_median"]

    lines = [
        "| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]

    seen = set()
    for scenario in scenario_order:
        if scenario not in grouped:
            continue
        seen.add(scenario)
        data = grouped[scenario]
        lines.append(
            "| {scenario} | {pico} | {llhttp} | {ihtp_ss} | {ihtp_s} | {ihtp_sl} | {ihtp_l} |".format(
                scenario=scenario,
                pico=data.get("picohttpparser", "—"),
                llhttp=data.get("llhttp", "—"),
                ihtp_ss=data.get("iohttpparser-stateful-strict", "—"),
                ihtp_s=data.get("iohttpparser-strict", "—"),
                ihtp_sl=data.get("iohttpparser-stateful-lenient", "—"),
                ihtp_l=data.get("iohttpparser-lenient", "—"),
            )
        )

    for scenario in sorted(set(grouped) - seen):
        data = grouped[scenario]
        lines.append(
            "| {scenario} | {pico} | {llhttp} | {ihtp_ss} | {ihtp_s} | {ihtp_sl} | {ihtp_l} |".format(
                scenario=scenario,
                pico=data.get("picohttpparser", "—"),
                llhttp=data.get("llhttp", "—"),
                ihtp_ss=data.get("iohttpparser-stateful-strict", "—"),
                ihtp_s=data.get("iohttpparser-strict", "—"),
                ihtp_sl=data.get("iohttpparser-stateful-lenient", "—"),
                ihtp_l=data.get("iohttpparser-lenient", "—"),
            )
        )

    return "\n".join(lines)

summary = f"""# PSI Run Summary

Run id: `{run_id}`

## Source code used for the run

- `bench/bench_throughput_compare.c`
- `scripts/run-throughput-compare.sh`
- `scripts/run-throughput-median.sh`
- `scripts/run-pmi-psi.sh`
- `tests/unit/test_differential_corpus.c`
- `tests/unit/test_semantics_differential.c`
- `tests/unit/test_iohttp_integration.c`

## Functional result

`ctest --preset {run_meta["debug_preset"]} --output-on-failure` completed successfully.

## Common median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-median.tsv`.

{render_reqs_matrix(median_rows)}

## CONNECT median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-connect-median.tsv`.

{render_reqs_matrix(connect_rows)}
"""

(out_dir / "summary.md").write_text(summary, encoding="utf-8")

extended_group_order = [
    "parser-state",
    "semantics-body",
    "consumer-iohttp",
    "upgrade",
    "consumer-ioguard",
]

extended_group_titles = {
    "parser-state": "Parser State",
    "semantics-body": "Semantics And Body Handoff",
    "consumer-iohttp": "iohttp-style Consumer Flows",
    "upgrade": "Upgrade Handoff",
    "consumer-ioguard": "ioguard-style Consumer Flows",
}

extended_lines = [
    "# Extended PSI Run Summary",
    "",
    f"Run id: `{run_id}`",
    "",
    "## Scope",
    "",
    "This summary publishes the extended-contract performance layer.",
    "",
    "## Source code used for the run",
    "",
    "- `bench/bench_extended_contract.c`",
    "- `scripts/run-throughput-extended.sh`",
    "- `scripts/run-throughput-extended-median.sh`",
    "- `scripts/run-pmi-psi.sh`",
    "- `tests/unit/test_iohttp_integration.c`",
    "- `tests/unit/test_semantics.c`",
    "- `tests/unit/test_body_decoder.c`",
    "",
    "## Median matrix",
    "",
    "Full values are published in `throughput-extended-median.tsv`.",
]

grouped_ext = {}
for row in extended_rows:
    grouped_ext.setdefault(row["scenario_group"], []).append(row)

for group in extended_group_order:
    rows = grouped_ext.get(group)
    if not rows:
        continue
    extended_lines.extend([
        "",
        f"### {extended_group_titles.get(group, group)}",
        "",
        "| Scenario | Baseline | req/s median | MiB/s median | ns/op median |",
        "|---|---|---:|---:|---:|",
    ])
    for row in rows:
        extended_lines.append(
            "| {scenario} | {baseline} | {req} | {mib} | {ns} |".format(
                scenario=row["scenario"],
                baseline=row["baseline"],
                req=row["req_per_s_median"],
                mib=row["mib_per_s_median"],
                ns=row["ns_per_op_median"],
            )
        )

(out_dir / "summary-extended.md").write_text("\n".join(extended_lines) + "\n", encoding="utf-8")
PY

printf '%s\n' "$RUN_ID" >"$BASE_DIR/latest.txt"
printf '%s\t%s\t%s\tPASS\n' \
    "$RUN_ID" \
    "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    "$(git -C "$ROOT_DIR" rev-parse --short HEAD)" \
    >>"$BASE_DIR/index.tsv"
