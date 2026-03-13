#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${ROOT_DIR}/dist"
TAG_NAME="${1:-${GITHUB_REF_NAME:-dev}}"
ARCHIVE_PREFIX="iohttpparser-${TAG_NAME}"

mkdir -p "${DIST_DIR}"
rm -f \
  "${DIST_DIR}/${ARCHIVE_PREFIX}.tar.gz" \
  "${DIST_DIR}/${ARCHIVE_PREFIX}.zip" \
  "${DIST_DIR}/${ARCHIVE_PREFIX}-docs.tar.gz" \
  "${DIST_DIR}/${ARCHIVE_PREFIX}-verification.tar.gz" \
  "${DIST_DIR}/${ARCHIVE_PREFIX}.sha256" \
  "${DIST_DIR}/RELEASE_NOTES.md"

git -C "${ROOT_DIR}" archive \
  --format=tar.gz \
  --prefix="${ARCHIVE_PREFIX}/" \
  -o "${DIST_DIR}/${ARCHIVE_PREFIX}.tar.gz" \
  HEAD

git -C "${ROOT_DIR}" archive \
  --format=zip \
  --prefix="${ARCHIVE_PREFIX}/" \
  -o "${DIST_DIR}/${ARCHIVE_PREFIX}.zip" \
  HEAD

cmake --preset clang-debug -DIOHTTPPARSER_BUILD_EXAMPLES=ON >/dev/null
cmake --build --preset clang-debug --target docs >/dev/null

if [[ -d "${ROOT_DIR}/docs/api/html" ]]; then
  tar -C "${ROOT_DIR}/docs/api/html" \
    -czf "${DIST_DIR}/${ARCHIVE_PREFIX}-docs.tar.gz" .
fi

if [[ -d "${ROOT_DIR}/tests/artifacts/pmi-psi" && -d "${ROOT_DIR}/tests/artifacts/release-candidate" ]]; then
  tar -C "${ROOT_DIR}/tests/artifacts" \
    -czf "${DIST_DIR}/${ARCHIVE_PREFIX}-verification.tar.gz" pmi-psi release-candidate
fi

bash "${ROOT_DIR}/scripts/render-release-notes.sh" "${TAG_NAME}"

(cd "${DIST_DIR}" && sha256sum \
  "${ARCHIVE_PREFIX}.tar.gz" \
  "${ARCHIVE_PREFIX}.zip" \
  "${ARCHIVE_PREFIX}-docs.tar.gz" \
  "${ARCHIVE_PREFIX}-verification.tar.gz" \
  > "${ARCHIVE_PREFIX}.sha256")
