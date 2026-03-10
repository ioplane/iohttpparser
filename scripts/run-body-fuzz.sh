#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build/clang-fuzz"
SOURCE_CORPUS_DIR="$ROOT_DIR/tests/fuzz/corpus/chunked"
RUNS="${RUNS:-256}"

TMP_CORPUS_DIR="$(mktemp -d /tmp/iohttpparser-body-fuzz.XXXXXX)"
cleanup() {
    rm -rf "$TMP_CORPUS_DIR"
}
trap cleanup EXIT

cp -r "$SOURCE_CORPUS_DIR/." "$TMP_CORPUS_DIR/"

cmake --preset clang-fuzz
cmake --build "$BUILD_DIR" --target fuzz_chunked

"$BUILD_DIR/fuzz_chunked" "$TMP_CORPUS_DIR" -runs="$RUNS"
