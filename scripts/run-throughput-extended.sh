#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/clang-release}"
FORMAT="${FORMAT:-human}"
ITERATIONS="${ITERATIONS:-200000}"
SCENARIO="${SCENARIO:-}"

args=(--iterations="$ITERATIONS")
if [[ "$FORMAT" == "tsv" ]]; then
    args+=(--format=tsv)
fi
if [[ -n "$SCENARIO" ]]; then
    args+=(--scenario="$SCENARIO")
fi

exec "$BUILD_DIR/bench_extended_contract" "${args[@]}"
