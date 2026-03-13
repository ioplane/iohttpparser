[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![CMake](https://img.shields.io/badge/CMake-clang--debug-064F8C?style=for-the-badge)](https://cmake.org/)
[![Doxygen](https://img.shields.io/badge/Doxygen-API-2C4AA8?style=for-the-badge)](https://www.doxygen.nl/)

# Release Candidate Artifacts

## Scope

This directory stores published verification evidence for release-candidate runs.

## Entry Points

- `latest.txt`: last published release-candidate run id
- `index.tsv`: published run index
- `runs/<run-id>/summary.md`: release-candidate summary
- `runs/<run-id>/toolchain.txt`: captured toolchain versions
- `runs/<run-id>/release-gate.txt`: release-gate output
- `runs/<run-id>/example_*.txt`: example outputs

## Producer

Artifacts are produced by:

- `scripts/run-release-candidate.sh`

## Required Files Per Run

- `run.txt`
- `host.txt`
- `toolchain.txt`
- `release-gate.txt`
- `examples-build.txt`
- `example_basic.txt`
- `example_connect.txt`
- `example_expect_trailers.txt`
- `example_response_upgrade.txt`
- `summary.md`
