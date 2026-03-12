#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUNS="${RUNS:-5}"
ITERATIONS="${ITERATIONS:-200000}"
CONNECT_ONLY="${CONNECT_ONLY:-0}"
TMP_DIR="${TMP_DIR:-$(mktemp -d)}"
KEEP_TMP="${KEEP_TMP:-0}"
EXTRA_ARGS="${EXTRA_ARGS:-}"

cleanup() {
    if [[ "$KEEP_TMP" != "1" ]]; then
        rm -rf "$TMP_DIR"
    fi
}
trap cleanup EXIT

for ((run = 1; run <= RUNS; run++)); do
    raw="$TMP_DIR/run-$run.raw"
    out="$TMP_DIR/run-$run.tsv"
    if [[ "$CONNECT_ONLY" == "1" ]]; then
        FORMAT=tsv CONNECT_ONLY=1 ITERATIONS="$ITERATIONS" \
            bash "$ROOT_DIR/scripts/run-throughput-compare.sh" $EXTRA_ARGS >"$raw"
    else
        FORMAT=tsv ITERATIONS="$ITERATIONS" \
            bash "$ROOT_DIR/scripts/run-throughput-compare.sh" $EXTRA_ARGS >"$raw"
    fi
    awk 'found || /^format\t/ { found = 1; print }' "$raw" >"$out"
done

python3 - "$TMP_DIR" "$RUNS" <<'PY'
from __future__ import annotations

import csv
import statistics
import sys
from pathlib import Path

tmp_dir = Path(sys.argv[1])
runs = int(sys.argv[2])

rows: dict[tuple[str, str], dict[str, list[float] | str | int]] = {}

for run in range(1, runs + 1):
    path = tmp_dir / f"run-{run}.tsv"
    with path.open("r", encoding="utf-8") as fh:
        reader = csv.DictReader(
            fh,
            delimiter="\t",
            fieldnames=[
                "parser",
                "scenario",
                "kind",
                "len",
                "elapsed_ns",
                "req_per_s",
                "mib_per_s",
                "ns_per_req",
            ],
        )
        for row in reader:
            if row["parser"] in {"format", "meta", "columns"}:
                continue
            key = (row["parser"], row["scenario"])
            item = rows.setdefault(
                key,
                {
                    "parser": row["parser"],
                    "scenario": row["scenario"],
                    "kind": row["kind"],
                    "length": int(row["len"]),
                    "req_per_s": [],
                    "mib_per_s": [],
                    "ns_per_req": [],
                },
            )
            item["req_per_s"].append(float(row["req_per_s"]))  # type: ignore[index]
            item["mib_per_s"].append(float(row["mib_per_s"]))  # type: ignore[index]
            item["ns_per_req"].append(float(row["ns_per_req"]))  # type: ignore[index]

print("parser\tscenario\tkind\tlen\treq_per_s_median\tmib_per_s_median\tns_per_req_median")
for key in sorted(rows):
    item = rows[key]
    print(
        "{parser}\t{scenario}\t{kind}\t{length}\t{req:.2f}\t{mib:.2f}\t{ns:.2f}".format(
            parser=item["parser"],
            scenario=item["scenario"],
            kind=item["kind"],
            length=item["length"],
            req=statistics.median(item["req_per_s"]),  # type: ignore[arg-type]
            mib=statistics.median(item["mib_per_s"]),  # type: ignore[arg-type]
            ns=statistics.median(item["ns_per_req"]),  # type: ignore[arg-type]
        )
    )
PY
