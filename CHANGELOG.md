# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0-rc.2] - 2026-03-14

### Fixed
- Release publication pipeline now configures Git safe-directory handling inside containerized verification and asset-build steps.
- Coverage workflow now uploads flat coverage artifacts that are readable to GitHub Actions and Codecov on hosted runners.
- SonarQube Cloud workflow uses a compatible CFamily configuration and the project no longer inherits unsupported custom target settings.

## [0.1.0-rc.1] - 2026-03-14

### Added
- Parser-core support for HTTP/1.1 request line, status line, and header fields.
- Public stateless and stateful parsing APIs.
- Public semantics APIs for framing, upgrade ownership, trailer ownership, and connection decisions.
- Fixed-length and chunked body decoders.
- Policy presets for `iohttp` and `ioguard`.
- Differential verification against `picohttpparser` and `llhttp`.
- Published PMI/PSI and extended-contract PSI artifact bundles.
- Release gate, coverage, CodeQL, Trivy, Scorecard, and SonarQube Cloud workflows.
- Source-first release packaging with source archives, generated API reference, verification bundles, and checksums.

### Changed
- Documentation set rewritten into numbered technical references.
- `README.md` updated with release, coverage, and static-analysis badges.
- Performance comparison methodology aligned to common parser-core scenarios and extended contract scenarios.

### Security
- Strict-by-default validation and framing rejection rules documented and verified for `iohttp` and `ioguard` consumer scenarios.

[Unreleased]: https://github.com/ioplane/iohttpparser/compare/v0.1.0-rc.2...HEAD
[0.1.0-rc.2]: https://github.com/ioplane/iohttpparser/compare/v0.1.0-rc.1...v0.1.0-rc.2
[0.1.0-rc.1]: https://github.com/ioplane/iohttpparser/releases/tag/v0.1.0-rc.1
