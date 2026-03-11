#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUNS="${RUNS:-5}"
ITERATIONS="${ITERATIONS:-100000}"
CONNECT_ONLY="${CONNECT_ONLY:-0}"
WORKDIR="${WORKDIR:-$ROOT_DIR/docs/tmp/throughput-median}"
EXTRA_ARGS="${EXTRA_ARGS:-}"

mkdir -p "$WORKDIR"

for run in $(seq 1 "$RUNS"); do
    out="$WORKDIR/run-$run.tsv"
    FORMAT=tsv CONNECT_ONLY="$CONNECT_ONLY" ITERATIONS="$ITERATIONS" \
        bash "$ROOT_DIR/scripts/run-throughput-compare.sh" $EXTRA_ARGS |
        awk '/^format\t/ {found=1} found {print}' >"$out"
done

python3 - "$WORKDIR" "$RUNS" <<'PY'
from pathlib import Path
import statistics
import sys

workdir = Path(sys.argv[1])
runs = int(sys.argv[2])
series = {}

for idx in range(1, runs + 1):
    path = workdir / f"run-{idx}.tsv"
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            if line.startswith(("format\t", "meta\t", "columns\t")):
                continue
            parts = line.rstrip("\n").split("\t")
            if len(parts) != 8:
                continue
            parser_name, scenario_name, _kind, _len, _elapsed, req_s, mib_s, ns_s = parts
            key = (parser_name, scenario_name)
            bucket = series.setdefault(key, {"req": [], "mib": [], "ns": []})
            bucket["req"].append(float(req_s))
            bucket["mib"].append(float(mib_s))
            bucket["ns"].append(float(ns_s))

print("parser\tscenario\tmedian_req_per_s\tmedian_mib_per_s\tmedian_ns_per_req")
for parser_name, scenario_name in sorted(series):
    bucket = series[(parser_name, scenario_name)]
    print(
        "{}\t{}\t{:.2f}\t{:.2f}\t{:.2f}".format(
            parser_name,
            scenario_name,
            statistics.median(bucket["req"]),
            statistics.median(bucket["mib"]),
            statistics.median(bucket["ns"]),
        )
    )
PY
