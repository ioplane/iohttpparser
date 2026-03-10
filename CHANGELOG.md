# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial project structure
- 4-layer parser architecture (Scanner / Parser / Semantics / Body decoder)
- Scalar scanner baseline
- SSE4.2 scanner with PCMPESTRI
- AVX2 scanner (placeholder)
- Pull-based incremental request/response parser
- Chunked transfer encoding decoder (in-place)
- Fixed-length body decoder
- Strict/lenient policy profiles (RFC 9112)
- Unity unit tests for all layers
- LibFuzzer targets for parser and chunked decoder
- CMake build system with 11 presets
- Quality pipeline (clang-format, cppcheck, PVS-Studio, CodeChecker)
- Dev container (Containerfile)
