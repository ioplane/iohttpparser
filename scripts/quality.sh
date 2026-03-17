#!/usr/bin/env bash
# shellcheck shell=bash
# Repository quality pipeline for iohttpparser.
# Run inside the dev container: cd /workspace && ./scripts/quality.sh
# Or from host: podman run --rm \
#   --env-file .env \
#   -v /opt/projects/repositories/iohttpparser:/workspace:Z \
#   localhost/iohttpparser-dev:latest bash -c "cd /workspace && ./scripts/quality.sh"
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

# shellcheck disable=SC1091
if [[ -f /usr/local/lib/ioplane/common.sh ]]; then
    source /usr/local/lib/ioplane/common.sh
else
    source "${SCRIPT_DIR}/lib/common.sh"
fi

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd -P)"
readonly ROOT_DIR
readonly BUILD_DIR="${BUILD_DIR:-build/clang-debug}"
readonly PRESET="${PRESET:-clang-debug}"
readonly THIRD_PARTY_DIR="${ROOT_DIR}/tests/third_party"
readonly TOTAL_STEPS=10

cd "${ROOT_DIR}"

# ═══════════════════════════════════════════════════════
# Step 1: Repository baseline
# ═══════════════════════════════════════════════════════

ioj_step 1 "${TOTAL_STEPS}" "Repository baseline"
ioj_check_repo_baseline

# ═══════════════════════════════════════════════════════
# Step 2: Stale identifier scan
# ═══════════════════════════════════════════════════════

ioj_step 2 "${TOTAL_STEPS}" "Stale identifier scan"
STALE_HITS=$(grep -rn 'iojournal\|ij_\|IJ_' docs/ scripts/ \
    --include='*.md' --include='*.sh' --include='*.py' \
    --exclude='quality.sh' \
    2>/dev/null || true)
if [[ -z "${STALE_HITS}" ]]; then
    ioj_record_pass "No stale iojournal/ij_ prefix remnants in docs/scripts"
else
    printf '%s\n' "${STALE_HITS}"
    ioj_record_fail "Stale iojournal/ij_ prefix found in docs/scripts"
fi

# ═══════════════════════════════════════════════════════
# Step 3: Documentation lint
# ═══════════════════════════════════════════════════════

ioj_step 3 "${TOTAL_STEPS}" "Documentation lint"
ioj_check_docs_lint

# ═══════════════════════════════════════════════════════
# Step 4: Configure and build
# ═══════════════════════════════════════════════════════

ioj_step 4 "${TOTAL_STEPS}" "Configure and build"
ioj_check_build_and_test "${PRESET}" "${BUILD_DIR}"

# ═══════════════════════════════════════════════════════
# Step 5: Format check
# ═══════════════════════════════════════════════════════

ioj_step 5 "${TOTAL_STEPS}" "Format check"
ioj_check_format "${PRESET}"

# ═══════════════════════════════════════════════════════
# Step 6: cppcheck
# ═══════════════════════════════════════════════════════

ioj_step 6 "${TOTAL_STEPS}" "cppcheck"
if ioj_has_cmake_surface; then
    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        ioj_record_fail "cppcheck: compile database missing"
    elif ! command -v cppcheck >/dev/null 2>&1; then
        ioj_record_skip "cppcheck not installed"
    else
        if cppcheck --enable=warning,performance,portability \
            --error-exitcode=1 --inline-suppr \
            --project="${BUILD_DIR}/compile_commands.json" \
            --suppress='*:/usr/local/src/unity/*' \
            --suppress="*:${THIRD_PARTY_DIR}/*" \
            -q 2>&1; then
            ioj_record_pass "cppcheck clean"
        else
            ioj_record_fail "cppcheck found issues"
        fi
    fi
else
    ioj_record_skip "Compile database unavailable before CMake bootstrap"
fi

# ═══════════════════════════════════════════════════════
# Step 7: PVS-Studio
# ═══════════════════════════════════════════════════════

ioj_step 7 "${TOTAL_STEPS}" "PVS-Studio"
ioj_check_pvs_studio "${BUILD_DIR}"

# ═══════════════════════════════════════════════════════
# Step 8: CodeChecker
# ═══════════════════════════════════════════════════════

ioj_step 8 "${TOTAL_STEPS}" "CodeChecker"
ioj_check_codechecker "${BUILD_DIR}" "${THIRD_PARTY_DIR}"

# ═══════════════════════════════════════════════════════
# Step 9: GCC analyzer
# ═══════════════════════════════════════════════════════

ioj_step 9 "${TOTAL_STEPS}" "GCC analyzer"
ioj_check_gcc_analyzer

# ═══════════════════════════════════════════════════════
# Step 10: shellcheck
# ═══════════════════════════════════════════════════════

ioj_step 10 "${TOTAL_STEPS}" "Shellcheck"
ioj_check_shellcheck

# ═══════════════════════════════════════════════════════
# Summary
# ═══════════════════════════════════════════════════════

ioj_print_summary
