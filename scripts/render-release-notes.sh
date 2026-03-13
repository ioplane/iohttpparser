#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${ROOT_DIR}/dist"
TAG_NAME="${1:-${GITHUB_REF_NAME:-dev}}"
RC_RUN_ID="$(cat "${ROOT_DIR}/tests/artifacts/release-candidate/latest.txt")"
PSI_RUN_ID="$(cat "${ROOT_DIR}/tests/artifacts/pmi-psi/latest.txt")"

mkdir -p "${DIST_DIR}"

cat > "${DIST_DIR}/RELEASE_NOTES.md" <<EOF
# iohttpparser ${TAG_NAME}

## Scope

- HTTP/1.1 wire-level parser for C23
- strict parser, semantics, and body-decoder contracts
- consumer-ready contract for iohttp and ioguard

## Verification

- release gate: \`scripts/run-release-gate.sh\`
- release-candidate evidence: \`${RC_RUN_ID}\`
- PMI/PSI evidence: \`${PSI_RUN_ID}\`

## Published Assets

- source tarball
- source zip archive
- generated API reference archive
- verification artifact archive
- checksums

## References

- common PSI: \`docs/en/09-test-results.md\`
- extended contract: \`docs/en/11-extended-contract-results.md\`
- release-candidate checklist: \`docs/en/12-release-candidate-checklist.md\`
EOF

