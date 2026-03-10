#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build/clang-release"
ITERATIONS="${ITERATIONS:-200000}"

cmake --preset clang-release -DIOHTTPPARSER_BUILD_BENCH=ON -DIOHTTPPARSER_BUILD_TESTS=OFF
cmake --build "$BUILD_DIR" --target bench_parser
"$BUILD_DIR/bench_parser" "$ITERATIONS"
