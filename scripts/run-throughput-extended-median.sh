#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUNS="${RUNS:-5}"
ITERATIONS="${ITERATIONS:-200000}"
SCENARIO="${SCENARIO:-}"
TMP_DIR="${TMP_DIR:-$(mktemp -d)}"
KEEP_TMP="${KEEP_TMP:-0}"

cleanup() {
    if [[ "$KEEP_TMP" != "1" ]]; then
        rm -rf "$TMP_DIR"
    fi
}
trap cleanup EXIT

for ((run = 1; run <= RUNS; run++)); do
    raw="$TMP_DIR/run-$run.raw"
    out="$TMP_DIR/run-$run.tsv"
    FORMAT=tsv ITERATIONS="$ITERATIONS" SCENARIO="$SCENARIO" \
        bash "$ROOT_DIR/scripts/run-throughput-extended.sh" >"$raw"
    awk 'found || /^scenario_group\t/ { found = 1; print }' "$raw" >"$out"
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
                "scenario_group",
                "scenario",
                "baseline",
                "wire_len",
                "work_len",
                "elapsed_ns",
                "req_per_s",
                "mib_per_s",
                "ns_per_op",
            ],
        )
        for row in reader:
            if row["scenario_group"] == "scenario_group":
                continue
            key = (row["scenario_group"], row["scenario"])
            item = rows.setdefault(
                key,
                {
                    "scenario_group": row["scenario_group"],
                    "scenario": row["scenario"],
                    "baseline": row["baseline"],
                    "wire_len": int(row["wire_len"]),
                    "work_len": int(row["work_len"]),
                    "req_per_s": [],
                    "mib_per_s": [],
                    "ns_per_op": [],
                },
            )
            item["req_per_s"].append(float(row["req_per_s"]))  # type: ignore[index]
            item["mib_per_s"].append(float(row["mib_per_s"]))  # type: ignore[index]
            item["ns_per_op"].append(float(row["ns_per_op"]))  # type: ignore[index]

print("scenario_group\tscenario\tbaseline\twire_len\twork_len\treq_per_s_median\tmib_per_s_median\tns_per_op_median")
for key in sorted(rows):
    item = rows[key]
    print(
        "{group}\t{scenario}\t{baseline}\t{wire_len}\t{work_len}\t{req:.2f}\t{mib:.2f}\t{ns:.2f}".format(
            group=item["scenario_group"],
            scenario=item["scenario"],
            baseline=item["baseline"],
            wire_len=item["wire_len"],
            work_len=item["work_len"],
            req=statistics.median(item["req_per_s"]),  # type: ignore[arg-type]
            mib=statistics.median(item["mib_per_s"]),  # type: ignore[arg-type]
            ns=statistics.median(item["ns_per_op"]),  # type: ignore[arg-type]
        )
    )
PY
