#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_PRESET="${BUILD_PRESET:-clang-release}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/$BUILD_PRESET}"
ITERATIONS="${ITERATIONS:-200000}"
CONNECT_ONLY="${CONNECT_ONLY:-0}"

cmake --preset "$BUILD_PRESET" -DIOHTTPPARSER_BUILD_BENCH=ON -DIOHTTPPARSER_BUILD_TESTS=OFF
cmake --build "$BUILD_DIR" --target bench_throughput_compare

args=("$ITERATIONS")
if [[ "${FORMAT:-human}" == "tsv" ]]; then
    args+=("--tsv")
fi
if [[ "$CONNECT_ONLY" == "1" ]]; then
    args+=("--connect-only")
fi

"$BUILD_DIR/bench_throughput_compare" "${args[@]}"
