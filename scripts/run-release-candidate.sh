#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
RUNSTAMP="$(date -u +%Y%m%dT%H%M%SZ)"
HEAD_SHORT="$(git -C "${ROOT_DIR}" rev-parse --short HEAD)"
RUN_ID="${RUNSTAMP}-${HEAD_SHORT}"
OUT_DIR="${ROOT_DIR}/tests/artifacts/release-candidate/runs/${RUN_ID}"
LATEST_FILE="${ROOT_DIR}/tests/artifacts/release-candidate/latest.txt"
INDEX_FILE="${ROOT_DIR}/tests/artifacts/release-candidate/index.tsv"
BUILD_DIR="${ROOT_DIR}/build/clang-debug"

mkdir -p "${OUT_DIR}"

{
    echo "run_id	${RUN_ID}"
    echo "git_head	$(git -C "${ROOT_DIR}" rev-parse HEAD)"
    echo "git_head_short	${HEAD_SHORT}"
    echo "utc_started	$(date -u +%Y-%m-%dT%H:%M:%SZ)"
} >"${OUT_DIR}/run.txt"

{
    uname -a
    echo "---"
    lscpu | sed -n '1,80p'
} >"${OUT_DIR}/host.txt"

{
    clang --version | sed -n '1,2p'
    echo "---"
    cmake --version | sed -n '1,1p'
    echo "---"
    doxygen --version
    echo "---"
    gdb --version | sed -n '1,1p'
    echo "---"
    valgrind --version
    echo "---"
    uftrace --version | sed -n '1,1p'
    echo "---"
    if command -v frun >/dev/null 2>&1; then
        command -v frun
    else
        echo "frun not installed"
    fi
} >"${OUT_DIR}/toolchain.txt"

bash "${ROOT_DIR}/scripts/run-release-gate.sh" >"${OUT_DIR}/release-gate.txt" 2>&1

cmake --preset clang-debug -DIOHTTPPARSER_BUILD_EXAMPLES=ON >/dev/null
cmake --build --preset clang-debug \
    --target example_basic example_connect example_expect_trailers example_response_upgrade \
    >"${OUT_DIR}/examples-build.txt" 2>&1

"${BUILD_DIR}/example_basic" >"${OUT_DIR}/example_basic.txt" 2>&1
"${BUILD_DIR}/example_connect" >"${OUT_DIR}/example_connect.txt" 2>&1
"${BUILD_DIR}/example_expect_trailers" >"${OUT_DIR}/example_expect_trailers.txt" 2>&1
"${BUILD_DIR}/example_response_upgrade" >"${OUT_DIR}/example_response_upgrade.txt" 2>&1

cat >"${OUT_DIR}/summary.md" <<EOF
# Release Candidate Verification Summary

Run id: \`${RUN_ID}\`

## Verification Scope

- release gate
- public documentation build
- public examples build
- public examples execution
- toolchain version capture

## Published Files

- \`run.txt\`
- \`host.txt\`
- \`toolchain.txt\`
- \`release-gate.txt\`
- \`examples-build.txt\`
- \`example_basic.txt\`
- \`example_connect.txt\`
- \`example_expect_trailers.txt\`
- \`example_response_upgrade.txt\`

## Result

All release-candidate checks completed successfully for this run.
EOF

if [[ ! -f "${INDEX_FILE}" ]]; then
    printf "run_id\tgit_head_short\tstatus\n" >"${INDEX_FILE}"
fi
printf "%s\t%s\tPASS\n" "${RUN_ID}" "${HEAD_SHORT}" >>"${INDEX_FILE}"
printf "%s\n" "${RUN_ID}" >"${LATEST_FILE}"
