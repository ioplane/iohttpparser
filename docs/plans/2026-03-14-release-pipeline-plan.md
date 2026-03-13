[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![GitHub Actions](https://img.shields.io/badge/GitHub%20Actions-Release%20Pipeline-2088FF?style=for-the-badge&logo=githubactions)](https://docs.github.com/actions)
[![Codecov](https://img.shields.io/badge/Codecov-Coverage-F01F7A?style=for-the-badge&logo=codecov)](https://docs.codecov.com/docs/quick-start)
[![Scorecard](https://img.shields.io/badge/OpenSSF-Scorecard-4285F4?style=for-the-badge)](https://github.com/ossf/scorecard-action)
[![SonarCloud](https://img.shields.io/badge/SonarQube%20Cloud-Quality-4E9BCD?style=for-the-badge&logo=sonarqubecloud)](https://docs.sonarsource.com/sonarqube-cloud/advanced-setup/ci-based-analysis/github-actions-for-sonarcloud/)
[![Trivy](https://img.shields.io/badge/Trivy-Security-1904DA?style=for-the-badge)](https://github.com/aquasecurity/trivy-action)

# Release Pipeline Plan

## Scope

This document defines the target release and repository-quality pipeline for `iohttpparser`.

## Release Model

Release model:
- source-first
- no mandatory prebuilt binaries
- release assets generated from a clean tagged commit

Published release assets:
- source tarball
- source zip archive
- checksums
- generated API reference archive
- published verification artifact archive

Rationale:
- the library is portable C23 code
- consumers build against local compiler and target ABI
- source release avoids unsupported binary-compatibility claims

## Workflow Set

| Workflow | Purpose | Trigger |
|---|---|---|
| `ci.yml` | release gate on repository changes | push, pull request |
| `coverage.yml` | collect coverage and upload to Codecov | push, pull request |
| `scorecard.yml` | OpenSSF Scorecard supply-chain analysis | branch protection, weekly schedule |
| `security-scans.yml` | CodeQL and Trivy scans | push, pull request, weekly schedule |
| `sonarcloud.yml` | SonarQube Cloud analysis | push, pull request |
| `release.yml` | publish a tagged source release | tag push `v*` |

## External Services

| Service | Required setup | Repository secret or variable |
|---|---|---|
| Codecov | repository enabled in Codecov GitHub App | optional `CODECOV_TOKEN` for private mode |
| SonarQube Cloud | organization and project created | `SONAR_TOKEN`, `SONARQUBE_PROJECT_KEY`, `SONARQUBE_ORGANIZATION` |
| OpenSSF Scorecard | none for public repositories | none |
| Trivy | none | none |
| CodeQL | none | none |

## Delivery Rules

Release workflow requirements:
1. run release gate
2. build and execute public examples
3. build Doxygen output
4. generate source archives
5. bundle published verification artifacts
6. generate checksums
7. create GitHub release with attached assets

## Documentation Rules

README requirements:
- show workflow and service badges
- link to official service pages
- include a short functional summary
- include a short performance summary with references to published PSI documents

## Validation Rules

Before merge:
- `python3 scripts/lint-docs.py`
- `bash -n scripts/run-coverage.sh`
- `bash -n scripts/build-release-assets.sh`
- `bash -n scripts/run-release-gate.sh`

Before release:
- `bash scripts/run-release-gate.sh`
- `bash scripts/run-pmi-psi.sh`
- `bash scripts/run-release-candidate.sh`

