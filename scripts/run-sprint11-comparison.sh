#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/docs/tmp/comparison/results}"
BUILD_PRESET="${BUILD_PRESET:-clang-debug}"
ITERATIONS="${ITERATIONS:-200000}"

mkdir -p "$OUT_DIR"

{
    echo "date_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "kernel=$(uname -srvmo)"
    echo "machine=$(uname -m)"
    if command -v lscpu >/dev/null 2>&1; then
        lscpu | sed -n '1,40p'
    fi
    if [[ -r /proc/cpuinfo ]]; then
        echo "--- cpu flags ---"
        grep -m1 -i '^flags' /proc/cpuinfo || true
    fi
} >"$OUT_DIR/host.txt"

{
    echo "vendored_pico=$(git -C "$ROOT_DIR" ls-tree HEAD tests/third_party/picohttpparser 2>/dev/null | awk '{ print substr($3, 1, 7) }' || echo unknown)"
    echo "vendored_llhttp=$(git -C "$ROOT_DIR" ls-tree HEAD tests/third_party/llhttp 2>/dev/null | awk '{ print substr($3, 1, 7) }' || echo unknown)"

    if [[ -d "$ROOT_DIR/docs/tmp/comparison/picohttpparser/.git" ]]; then
        echo "upstream_pico=$(git -C "$ROOT_DIR/docs/tmp/comparison/picohttpparser" rev-parse --short HEAD)"
    fi
    if [[ -d "$ROOT_DIR/docs/tmp/comparison/llhttp/.git" ]]; then
        echo "upstream_llhttp=$(git -C "$ROOT_DIR/docs/tmp/comparison/llhttp" rev-parse --short HEAD)"
    fi
} >"$OUT_DIR/references.txt"

cmake --preset "$BUILD_PRESET"
cmake --build --preset "$BUILD_PRESET"

ctest --preset "$BUILD_PRESET" --output-on-failure -R \
    "test_differential_corpus|test_semantics_differential|test_scanner_backends|test_scanner_corpus|test_iohttp_integration" \
    | tee "$OUT_DIR/ctest.txt"

bench_raw="$OUT_DIR/scanner-bench.raw.txt"

FORMAT=tsv ITERATIONS="$ITERATIONS" bash "$ROOT_DIR/scripts/run-scanner-bench.sh" >"$bench_raw"
awk 'found || /^format\t/ { found = 1; print }' "$bench_raw" >"$OUT_DIR/scanner-bench.tsv"

printf 'comparison_results=%s\n' "$OUT_DIR"
