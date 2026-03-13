#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$ROOT_DIR"

./scripts/quality.sh
ctest --preset clang-debug --output-on-failure -R "test_differential_corpus|test_semantics_differential|test_iohttp_integration"
cmake --build --preset clang-debug --target docs
python3 scripts/lint-docs.py
python3 scripts/check-pmi-psi-artifacts.py
