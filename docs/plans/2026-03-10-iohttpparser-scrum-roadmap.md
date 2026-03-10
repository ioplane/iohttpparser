# iohttpparser Scrum Roadmap

**Goal:** Bring `iohttpparser` from parser prototype to a production-ready C23 HTTP/1.1 parser library with strict default semantics, SIMD scanner backends, validated quality tooling, and consumer-ready integration contracts for `iohttp` and `ringwall`.

**Workflow constraints:**
- Development and validation happen only inside the dev container.
- Use `git` and `.worktrees/` for sprint isolation after the repository has a first real commit.
- Use `gh api graphql` for GitHub organization checks, repo publication, and release automation.
- Use repository-local skills as mandatory guidance:
  - `.claude/skills/iohttpparser-architecture/`
  - `.claude/skills/iohttpparser-coding-standards/`
  - `.claude/skills/modern-c23/`
  - `.claude/skills/http-rfc-reference/`

**MCP note:** no MCP resources are currently configured for this repository, so planning relies on local docs, repo skills, and `gh api graphql`.

## Current Status (2026-03-10)

**Completed:**
- dev image `localhost/iohttpparser-dev:latest` builds successfully
- `.env` with local PVS credentials is available for container runs
- `docs/rfc/` curated mirror and index are in place
- `CLAUDE.md`, `CODEX.md`, base skills, RFC scraper, and plan docs were added
- docs layout was normalized to `docs/en`, `docs/ru`, `docs/rfc`, `docs/plans`, `docs/tmp`
- target GitHub organization `ioplane` was verified with `gh api graphql`
- container-only baseline is green for:
  - `cmake --preset clang-debug`
  - `cmake --build --preset clang-debug`
  - `ctest --preset clang-debug`
  - `./scripts/quality.sh`
- current Sprint 0 fixes include:
  - `clang-format` normalization for headers, sources, tests, and examples
  - `ihtp_body_decoder.c` fix for widening-cast analyzer findings
  - `ihtp_scanner_sse42.c` fallback to scalar token validation until a proven SIMD-equivalent implementation exists
  - `ihtp_parser.c` cleanup for real `PVS-Studio V769` findings
  - `.pvs-suppress.json` narrowed to current `V1042` license-noise only
  - public API and file namespace migrated from `hp_` / `HP_` to `ihtp_` / `IHTP_`
- first real commit exists on `main`: `c383d68` (`chore: initialize iohttpparser`)
- Sprint 1 worktree is active at `.worktrees/sprint-1` on `feature/sprint-1-scalar-correctness`
- current Sprint 1 parser tasks completed:
  - strict status-line separator enforcement
  - request-target validation against `SP` / `CTL` / `DEL`
  - `allow_spaces_in_uri` wired into request-line parsing without loosening `CTL` rejection
  - bare `LF` handling wired to `reject_bare_lf` with strict reject and lenient accept
  - `bare LF` policy coverage extended across request, response, and headers-only APIs
  - header value validation for `CTL` / `DEL` with `HTAB` preserved
  - `reject_obs_fold` now distinguishes strict reject from lenient raw folded-value acceptance
  - lenient `obs-fold` continuation still rejects invalid control bytes
  - `bytes_consumed` reset contract on incomplete/error paths
  - response status-code range validation (`100..599`)
  - response `reason-phrase` validation against control bytes
  - regression coverage for `ERROR_TOO_LONG` and `ERROR_TOO_MANY_HEADERS`
  - full container quality checkpoint is green again:
    - `cmake --preset clang-debug`
    - `cmake --build --preset clang-debug`
    - `ctest --preset clang-debug`
    - `./scripts/quality.sh`
- Sprint 2 worktree is active at `.worktrees/sprint-2` on `feature/sprint-2-semantics-hardening`
- current Sprint 2 semantics tasks completed:
  - `TE + CL` strict reject / lenient accept coverage for requests and responses
  - conflicting duplicate `Content-Length` rejected; identical duplicates accepted
  - explicit `Connection` header decisions preserved over version defaults
  - `Host` validation added for `HTTP/1.1` requests
  - `Transfer-Encoding` parsing hardened for comma-separated codings and case-insensitive `chunked`
  - requests with `Transfer-Encoding` not ending in `chunked` are reject-by-default
  - malformed `Transfer-Encoding` and `Connection` token lists are rejected
  - `Connection` token-list parsing is now case-insensitive and close-wins
  - duplicate `chunked` codings are rejected both within one header and across multiple headers
  - `chunked` transfer-coding parameters are rejected
  - empty `Connection` values are rejected
  - no-body response regressions are covered for `204` and `304`
  - no-body response precedence is now regression-covered for `1xx`, `204`, and `304`
  - semantics negative corpus has started under `tests/corpus/semantics/`
  - corpus-driven regression runner is wired into `ctest`
  - corpus assertions now cover `body_mode`, `content_length`, and `keep_alive`
  - semantics corpus now covers malformed and empty `Connection` values
  - semantics corpus now covers `chunked` transfer-coding parameters for requests and responses
  - semantics corpus now covers additional no-body response precedence cases for `204` and `304`
  - semantics corpus now covers fixed-length framing and identical/conflicting duplicate `Content-Length`
  - semantics corpus now covers request/response keep-alive defaults for HTTP/1.0 and HTTP/1.1
  - semantics corpus now covers request-side `Host` invariants for missing, duplicate, and empty `Host`
  - semantics corpus now covers lenient response handling for `TE + CL`
  - semantics corpus now covers positive `Transfer-Encoding` paths for request chains ending in `chunked` and case-insensitive `chunked`
  - semantics corpus now covers malformed `Transfer-Encoding` lists and request-side rejection when `Transfer-Encoding` does not end in `chunked`
  - semantics corpus now covers positive `Connection` paths for case-insensitive tokens and token-list `close wins`
  - semantics corpus now covers `101` no-body precedence for both `Content-Length` and `Transfer-Encoding`
  - semantics corpus now covers strict reject paths for `TE + CL` on `101`, `204`, and `304` responses
  - full container quality checkpoint is green in Sprint 2:
    - `cmake --preset clang-debug`
    - `cmake --build --preset clang-debug`
    - `ctest --preset clang-debug`
    - `./scripts/quality.sh`
- Sprint 3 worktree is active at `.worktrees/sprint-3` on `feature/sprint-3-body-decoder`
- current Sprint 3 body decoder tasks completed:
  - chunked trailer consumption now distinguishes empty trailers from non-empty trailer sections
  - non-empty trailers are consumed line-by-line until the terminating empty line
  - chunked body tests now cover empty trailers, non-empty trailers, and incomplete trailers
  - chunked body tests now cover incremental decoding across multiple buffers
  - chunked body tests now cover incremental trailer consumption across multiple buffers
  - chunk extensions now reject bare `LF` in strict decoder flow
  - chunked body tests now cover valid, incomplete, and malformed chunk extensions
  - chunked body tests now cover malformed chunk framing for invalid hex digits, missing `LF`, missing `CR`, and oversized chunk sizes
  - fixed-length body tests now cover zero-length bodies, zero-length no-op consumes, and overflow after partial progress
  - chunked body tests now cover trailer-line strictness for bare `LF` and missing `LF` after trailer `CR`
  - chunked body tests now cover incremental `CRLF` boundary splits across size and data transitions
  - chunked body tests now document trailing-byte completion contract both with and without trailer consumption
  - `fuzz_chunked` now exercises both `consume_trailer = false` and `consume_trailer = true`
  - starter chunked fuzz seeds now exist under `tests/fuzz/corpus/chunked/`
  - `clang-fuzz` builds both `fuzz_chunked` and `fuzz_parser` successfully in the container
  - starter parser fuzz seeds now exist under `tests/fuzz/corpus/parser/`
  - both `fuzz_chunked` and `fuzz_parser` have been executed successfully in the container against local seed corpora
  - body decoder corpus now exists under `tests/corpus/body/`
  - `test_body_decoder_corpus` is wired into `ctest` for representative valid and invalid chunked and fixed-length cases
  - decoder contracts are now documented in `include/iohttpparser/ihtp_body.h`, `README.md`, `docs/en/body-decoder.md`, and `docs/ru/body-decoder.md`
  - `scripts/run-body-fuzz.sh` now provides a reproducible in-container body-fuzz workflow using temporary corpus copies
  - Sprint 3 closeout verification is green on this branch:
    - `cmake --preset clang-debug`
    - `cmake --build --preset clang-debug`
    - `ctest --preset clang-debug`
    - `./scripts/quality.sh`
    - `RUNS=64 bash scripts/run-body-fuzz.sh`
  - full container quality checkpoint is green in Sprint 3:
    - `cmake --preset clang-debug`
    - `cmake --build --preset clang-debug`
    - `ctest --preset clang-debug`
    - `./scripts/quality.sh`

**Current Sprint 1 focus:**
- continue scalar edge-case coverage
- expand malformed request/response corpus
- keep full container quality baseline green as parser tasks land

**Current Sprint 2 focus:**
- continue semantics hardening around framing ambiguity
- expand negative corpus for smuggling-sensitive cases
- keep full container quality baseline green as semantics tasks land

**Current Sprint 3 focus:**
- closeout branch is ready for review and merge
- next implementation focus after merge is Sprint 4 SIMD scalar-equivalence and benchmarks

**Current Sprint 4 focus:**
- `feature/sprint-4-simd-equivalence` is active from the Sprint 3 closeout branch
- scanner backend equivalence is now covered in `test_scanner_backends`
- scanner corpus coverage now exists under `tests/corpus/scanner/` with `test_scanner_corpus`
- runtime dispatch invariants are now covered for active backend selection and public scanner delegation
- exhaustive single-byte scanner invariants now cover all 256 byte values for token classification and delimiter search
- internal backend selection is now testable directly via explicit SIMD-level selection
- internal backend naming is now exposed for deterministic diagnostics without changing public API
- SSE4.2 delimiter loading was hardened to avoid over-reading short delimiter strings
- `scripts/run-scanner-bench.sh` now provides a reproducible container benchmark smoke-run
- `scripts/check-scanner-bench.sh` now verifies machine-readable benchmark output shape for reproducible smoke checks
- `fuzz_scanner` and `scripts/run-scanner-fuzz.sh` now provide differential scanner fuzzing against scalar truth
- benchmark slices now include longer parser-like request and header inputs
- full Sprint 4 checkpoint baseline is green:
  - `cmake --preset clang-debug`
  - `cmake --build --preset clang-debug`
  - `ctest --preset clang-debug`
  - `ITERATIONS=3000 bash scripts/run-scanner-bench.sh`
  - `./scripts/quality.sh`
- next implementation focus is verifying fallback behavior more explicitly and deciding whether SIMD token validation should remain scalar-backed

**Immediate execution queue:**
1. Review and merge Sprint 3 closeout branch into `main`.
2. Keep Sprint 4 moving from `feature/sprint-3-closeout` until merge permissions are available.
3. Keep body corpus and body-fuzz workflow as the verification baseline for future body-decoder changes.
4. Expand SIMD/scalar equivalence coverage and benchmark corpus slices.

---

## Sprint 0: Tooling Baseline and Repository Hygiene

**Goal:** Make the repository reproducible and ready for container-only development.

**Deliverables:**
- working dev image `iohttpparser-dev:latest`
- `.env` for PVS loaded locally and excluded from git
- `docs/rfc/` curated mirror and index
- `CLAUDE.md`, `CODEX.md`, base skills, and plan docs aligned
- public API namespace frozen as `ihtp_` / `IHTP_`
- first real commit on `main`

**Exit criteria:**
- container builds successfully
- `git worktree add` becomes usable after the first commit
- local docs structure matches `iohttp`
- baseline quality findings are triaged into:
  - must-fix code defects
  - accepted temporary tool noise / suppressions

---

## Sprint 1: Scalar Correctness for Request-Line and Headers

**Goal:** Lock down the scalar parser as the source of truth.

**Entry gate:**
- Sprint 0 is complete
- first commit exists
- quality baseline is stable enough that new parser defects are distinguishable from inherited noise

**Scope:**
- request-line parsing
- status-line parsing
- header extraction
- strict/lenient policy defaults
- consumed-byte accounting

**Tasks:**
- finish parser edge-case coverage
- align behavior with RFC 9110 / RFC 9112
- remove ambiguous or unsafe leniency defaults
- expand Unity tests for malformed input
- checkpoint and commit Sprint 1 parser work once the remaining parser-only gaps are enumerated

**Exit criteria:**
- `ctest --preset clang-debug` passes in container
- strict mode behavior is documented and test-covered

---

## Sprint 2: Semantics and Framing Security

**Goal:** Treat semantics as a security boundary.

**Scope:**
- `Content-Length`
- `Transfer-Encoding`
- `TE + CL`
- keep-alive / close
- duplicate headers and framing ambiguity

**Tasks:**
- harden semantics layer against smuggling patterns
- add negative corpus for ambiguous framing
- document `iohttp` vs `ringwall` profile differences

**Exit criteria:**
- conflicting `Content-Length` and `TE + CL` cases are reject-by-default
- semantics tests cover strict and lenient profiles

---

## Sprint 3: Body Decoder Completion

**Goal:** Make fixed-length and chunked decoding production-ready.

**Scope:**
- fixed-length tracking
- chunked decoder state machine
- trailer consumption
- malformed chunk handling

**Tasks:**
- complete chunk parser edge cases
- add in-place decoder tests
- add fuzz seeds for body framing

**Exit criteria:**
- body decoder behavior is deterministic and incremental
- fuzz targets compile and run in the container

---

## Sprint 4: SIMD Backend Stabilization

**Goal:** Optimize scanner backends without changing parser semantics.

**Scope:**
- scalar baseline
- SSE4.2 backend
- AVX2 backend
- runtime dispatch invariants

**Tasks:**
- build scalar-equivalence tests for SIMD paths
- add benchmark harness and corpus slices
- verify fallback on unsupported CPUs

**Current progress on `feature/sprint-4-simd-equivalence`:**
- `tests/unit/test_scanner_backends.c` compares scalar, SSE4.2, and AVX2 scanner behavior on shared find/token cases
- `tests/unit/test_scanner_backends.c` now also verifies runtime dispatch chooses the expected active backend and that public scanner APIs delegate to it
- `tests/unit/test_scanner_backends.c` now exhaustively checks single-byte token and delimiter behavior across the full byte range
- `src/ihtp_scanner_scalar.c` now exposes internal backend selection for deterministic fallback tests without changing public API
- `src/ihtp_scanner_scalar.c` now also exposes internal backend naming helpers for deterministic diagnostics and benchmark metadata
- `tests/unit/test_scanner_corpus.c` and `tests/corpus/scanner/` provide data-driven scanner equivalence coverage for embedded NUL, high-byte, empty-delimiter, and long-delimiter cases
- `tests/fuzz/fuzz_scanner.c` differentially compares scalar, dispatch, SSE4.2, and AVX2 scanner paths on fuzzed inputs
- `tests/fuzz/corpus/scanner/` and `scripts/run-scanner-fuzz.sh` provide a reproducible scanner-fuzz workflow in the container
- `src/ihtp_scanner_sse42.c` now copies short delimiter strings into a fixed local buffer before `_mm_loadu_si128`
- `bench/bench_parser.c` provides a first scanner benchmark harness for dispatch, scalar, SSE4.2, and AVX2 paths
- `bench/bench_parser.c` now supports machine-readable `--tsv` output for reproducible smoke checks
- `bench/bench_parser.c` now emits `active_backend` metadata for machine-readable benchmark diagnostics
- `scripts/run-scanner-bench.sh` provides a reproducible container workflow for a release bench smoke-run
- `scripts/check-scanner-bench.sh` verifies benchmark output shape, row count, and `simd_level -> active_backend` consistency inside the container
- container validation is green for:
  - `cmake --preset clang-debug`
  - `cmake --build --preset clang-debug`
  - `ctest --preset clang-debug`
  - `ITERATIONS=3000 bash scripts/run-scanner-bench.sh`
  - `ITERATIONS=3000 bash scripts/check-scanner-bench.sh`
  - `RUNS=64 bash scripts/run-scanner-fuzz.sh`
  - `./scripts/quality.sh`

**Exit criteria:**
- SIMD backends match scalar behavior on test corpus
- benchmark results are reproducible in container

---

## Sprint 5: Fuzzing, Sanitizers, and Static Analysis

**Goal:** Raise confidence before consumer integration.

**Scope:**
- fuzz targets
- ASan/UBSan
- MSan where practical
- cppcheck
- PVS-Studio
- CodeChecker

**Tasks:**
- integrate fuzz scripts
- curate malformed corpus from `docs/tmp/draft`
- baseline and fix analyzer findings

**Exit criteria:**
- quality pipeline passes for touched code
- corpus and fuzz harness are documented in repo

---

## Sprint 6: iohttp Integration Contract

**Goal:** Make `iohttpparser` usable as the HTTP/1.1 codec for `iohttp`.

**Scope:**
- parser API review
- request model mapping
- policy defaults for general server use
- public packaging and install story

**Tasks:**
- define adapter contract for `iohttp`
- validate public headers and pkg-config metadata
- test parser under a small containerized integration harness

**Exit criteria:**
- `iohttp` can consume the library without parser-core changes
- integration expectations are documented

---

## Sprint 7: ringwall Strict-Profile Integration

**Goal:** Provide a stricter consumer profile for `ringwall`.

**Scope:**
- fail-closed policy profile
- tighter limits
- minimal leniency
- proxy/control-plane assumptions

**Tasks:**
- codify `strict-proxy` behavior
- define migration path from current `llhttp`-oriented assumptions
- add security regression tests for proxy-sensitive cases

**Exit criteria:**
- `ringwall` integration contract is explicit
- strict profile behavior is separate from `iohttp` general profile

---

## Sprint 8: Publication and Release Preparation

**Goal:** Publish the repository into GitHub organization `ioplane` and cut the first usable release.

**Tasks:**
- create or confirm target repository with `gh api graphql`
- push canonical branches with `git`
- add release checklist and changelog entry
- validate install, pkg-config, and docs layout

**Exit criteria:**
- repository published under `https://github.com/ioplane`
- first tagged release candidate prepared

---

## Worktree Strategy

After the first commit:
- keep `.worktrees/sprint-<n>/` for active sprint branches
- use one worktree per sprint or per risky integration stream
- keep `main` clean and container-validated

Suggested naming:
- `feature/sprint-1-scalar-correctness`
- `feature/sprint-2-semantics-hardening`
- `feature/sprint-4-simd-stabilization`

---

## Definition of Done

A sprint is done only if:
- code and checks were run inside the container
- touched behavior has unit tests
- docs and local skills stay aligned with architecture changes
- no new unresolved quality findings are introduced in touched files
- sprint results are committed and, when relevant, isolated in a worktree-backed branch

## Revised Near-Term Order

1. Finish Sprint 0:
   - create the first real commit on `main`
2. Start worktree-based Sprint 1 branch:
   - `feature/sprint-1-scalar-correctness`
3. Defer new feature work in semantics/SIMD until the scalar and tooling baseline is commit-clean.
