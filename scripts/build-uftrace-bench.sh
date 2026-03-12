#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/uftrace-clang-pg}"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DIOHTTPPARSER_BUILD_BENCH=ON \
    -DIOHTTPPARSER_BUILD_TESTS=ON \
    -DIOHTTPPARSER_BUILD_FUZZ=OFF \
    -DCMAKE_C_FLAGS_RELWITHDEBINFO="-O2 -g -pg" \
    -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="-pg"

cmake --build "$BUILD_DIR" --target bench_throughput_compare
