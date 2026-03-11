#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TOOL="${1:-}"
SCENARIO="${2:-req-pico-bench}"
ITERATIONS="${3:-50000}"
PARSER="${4:-iohttpparser-strict}"

usage() {
    cat <<'EOF'
usage: scripts/run-profiler-stack.sh <tool> [scenario] [iterations] [parser]

tool:
  trace      built-in parser layer counters (requires IOHTTPPARSER_PERF_TRACE build)
  uftrace    function graph tracing (preferred external profiler)
  valgrind   memory/cost model profiling (memcheck/callgrind/cachegrind)
  gdb        debugger entrypoint for scenario-focused inspection
  ftracer    GCC-only function tracing workflow note

examples:
  scripts/run-profiler-stack.sh trace req-pico-bench 50000 iohttpparser-strict
  scripts/run-profiler-stack.sh uftrace req-line-only 10000 iohttpparser-strict
  scripts/run-profiler-stack.sh valgrind req-pico-bench 1000 iohttpparser-strict
EOF
}

need_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "missing tool: $1" >&2
        exit 2
    fi
}

bench_bin_default="$ROOT_DIR/build/clang-debug/bench_throughput_compare"
trace_bin="$ROOT_DIR/build/trace-clang/bench_throughput_compare"
results_dir="$ROOT_DIR/docs/tmp/profiling"

mkdir -p "$results_dir"

case "$TOOL" in
trace)
    if [[ ! -x "$trace_bin" ]]; then
        echo "trace build not found: $trace_bin" >&2
        echo "build with IOHTTPPARSER_PERF_TRACE=ON first" >&2
        exit 2
    fi
    exec "$trace_bin" "$ITERATIONS" --trace-parser "$PARSER" --trace-scenario "$SCENARIO"
    ;;
uftrace)
    need_cmd uftrace
    if [[ ! -x "$bench_bin_default" ]]; then
        echo "benchmark binary not found: $bench_bin_default" >&2
        exit 2
    fi
    out_dir="$results_dir/uftrace-${SCENARIO}-${PARSER}"
    echo "Running uftrace record into $out_dir"
    exec uftrace record -d "$out_dir" -- "$bench_bin_default" "$ITERATIONS" --tsv
    ;;
valgrind)
    need_cmd valgrind
    if [[ ! -x "$bench_bin_default" ]]; then
        echo "benchmark binary not found: $bench_bin_default" >&2
        exit 2
    fi
    out_file="$results_dir/valgrind-${SCENARIO}-${PARSER}.log"
    echo "Running Valgrind Memcheck into $out_file"
    exec valgrind --tool=memcheck --leak-check=full --track-origins=yes \
        --log-file="$out_file" "$bench_bin_default" "$ITERATIONS" --tsv
    ;;
gdb)
    need_cmd gdb
    if [[ ! -x "$bench_bin_default" ]]; then
        echo "benchmark binary not found: $bench_bin_default" >&2
        exit 2
    fi
    exec gdb --args "$bench_bin_default" "$ITERATIONS" --tsv
    ;;
ftracer)
    cat <<'EOF'
ftracer is not wired as a default runnable mode.

Recommended usage for this repository:
1. Build a dedicated GCC-only profiling binary with:
   -pg -mfentry -rdynamic
2. Link with the external ftracer object as described by upstream.
3. Use it only for narrow control-flow tracing of hot scenarios, not for throughput numbers.

This is intentionally not automated here because it requires a separate instrumented build and
introduces large per-call overhead.
EOF
    ;;
*)
    usage
    exit 2
    ;;
esac
