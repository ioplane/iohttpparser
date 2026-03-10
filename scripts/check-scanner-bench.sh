#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ITERATIONS="${ITERATIONS:-3000}"
OUTPUT_FILE="$(mktemp /tmp/iohttpparser-scanner-bench.XXXXXX)"
trap 'rm -f "$OUTPUT_FILE"' EXIT

FORMAT=tsv ITERATIONS="$ITERATIONS" bash "$ROOT_DIR/scripts/run-scanner-bench.sh" >"$OUTPUT_FILE"

grep -q '^format[[:space:]]tsv[[:space:]]v1$' "$OUTPUT_FILE"
grep -q '^meta[[:space:]]iterations[[:space:]]'"$ITERATIONS"'$' "$OUTPUT_FILE"
simd_level="$(awk -F '\t' '$1 == "meta" && $2 == "simd_level" { print $3 }' "$OUTPUT_FILE")"
active_backend="$(awk -F '\t' '$1 == "meta" && $2 == "active_backend" { print $3 }' "$OUTPUT_FILE")"

case "$simd_level" in
    0x2|0x3) expected_backend="avx2" ;;
    0x1) expected_backend="sse42" ;;
    0x0) expected_backend="scalar" ;;
    *)
        echo "unexpected simd level: $simd_level" >&2
        exit 1
        ;;
esac

if [[ "$active_backend" != "$expected_backend" ]]; then
    echo "expected active backend $expected_backend for simd level $simd_level, got $active_backend" >&2
    exit 1
fi

grep -q '^columns[[:space:]]backend[[:space:]]operation[[:space:]]case[[:space:]]len[[:space:]]elapsed_ns[[:space:]]ns_per_op$' "$OUTPUT_FILE"
grep -q '^dispatch[[:space:]]find[[:space:]]request-space[[:space:]]' "$OUTPUT_FILE"
grep -q '^scalar[[:space:]]token[[:space:]]method[[:space:]]' "$OUTPUT_FILE"
grep -q '^meta[[:space:]]sinks[[:space:]]' "$OUTPUT_FILE"

case_lines="$(grep -cE '^(dispatch|scalar|sse42|avx2)[[:space:]]' "$OUTPUT_FILE" || true)"
if [[ "$case_lines" -lt 32 ]]; then
    echo "expected at least 32 benchmark rows, got $case_lines" >&2
    exit 1
fi

echo "scanner bench output verified ($case_lines rows)"
