# iohttpparser Scrum Roadmap

**Goal:** Bring `iohttpparser` from parser prototype to a production-ready C23 HTTP/1.1 parser library with strict default semantics, SIMD scanner backends, validated quality tooling, and consumer-ready integration contracts for `iohttp` and `ioguard` (formerly `ringwall`).

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

## Current Status (2026-03-11)

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
- decoder contracts are now documented in `include/iohttpparser/ihtp_body.h`, `README.md`, `docs/en/07-body-decoder.md`, and `docs/ru/07-body-decoder.md`
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

**Merged sprint summary:**
- Sprint 1: scalar request/status/header correctness is merged on `main`
- Sprint 2: framing and semantics hardening is merged on `main`
- Sprint 3: body decoder completion, body corpus, and body-fuzz workflow are merged on `main`
- Sprint 4: SIMD/scanner equivalence, fuzzing, and benchmark smoke tooling are merged on `main`
- Sprint 5: public stateful parser API is merged on `main`
- Sprint 6: parser-level and semantics-level differential validation against `picohttpparser` and `llhttp` is merged on `main`
- Sprint 7: consumer contracts, public semantics API, ownership flags, named consumer presets, and `CONNECT` guidance are merged on `main`

**Quality evidence on current `main` (2026-03-11):**
- `./scripts/quality.sh` passes in the dev container
- regression surface is currently green for:
  - scalar/scanner/unit tests
  - parser-state tests
  - parser and semantics differential tests
  - semantics corpus tests
  - body decoder and body corpus tests
  - `clang-format`
  - `cppcheck`
  - `PVS-Studio`
  - `CodeChecker`

**Current functionality baseline:**
- strict-by-default HTTP/1.1 request, response, and headers-only parsing
- public stateless and stateful parser APIs
- hardened semantics for framing, keep-alive, `Host`, `Transfer-Encoding`, and `Content-Length`
- incremental fixed-length and chunked body decoding with trailer handling
- explicit consumer handoff for:
  - protocol upgrades
  - `Expect: 100-continue`
  - trailer ownership
  - `CONNECT` tunneling decisions
- consumer presets:
  - `IHTP_POLICY_IOHTTP`
  - `IHTP_POLICY_IOGUARD`
- reference examples:
  - `examples/basic_parse.c`
  - `examples/connect_tunnel.c`

**Readiness assessment:**
- parser/scanner correctness: high
- semantics and framing security: high
- body decoding and incremental behavior: high
- public API and consumer documentation: medium-high
- release engineering and CI gating: medium
- overall library readiness: late alpha / early beta candidate, approximately `80%`

**Main remaining gaps before functional completeness:**
- a few integration-oriented semantics cases still need explicit consumer-facing guidance and examples:
  - response-side upgrade handoff
  - `100 Continue` integration flow
  - trailer consumption ownership beyond the current minimal contract
- consumer presets exist, but the final contract still needs to be frozen:
  - keep `IHTP_POLICY_IOHTTP` and `IHTP_POLICY_IOGUARD` equivalent for now with explicit rationale
  - or make them intentionally distinct in a narrow, test-covered way
- embedder-facing documentation still needs one final freeze pass on lifetime/ownership/reset guarantees
- functional completion should be followed by real integration campaigns against `iohttp` and `ioguard`
- only after functional completion and integration campaigns should CI/release gating be treated as the next priority
- SSE4.2 token validation intentionally remains scalar-backed until a proven SIMD-equivalent implementation is justified

**Immediate execution queue:**
1. Start Sprint 8 for final functional completion:
   - response upgrade handoff
   - `100 Continue` flow
   - trailer ownership examples
   - final preset-contract decision
2. Run Sprint 9 as a public API and documentation freeze pass.
3. Run Sprint 10 as a consumer-style integration campaign for `iohttp` and `ioguard`.
4. Run Sprint 11 as an expanded `picohttpparser` / `llhttp` comparison campaign.
5. Only after those four phases start CI and release-gating work.

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

## Sprint 5: Public Stateful Parser API

**Goal:** Make incremental parser use explicit in the public API without abandoning the existing stateless surface.

**Scope:**
- `ihtp_parser_state_t`
- stateful request, response, and headers-only entry points
- reset/init lifecycle
- consumer-facing parser-state docs

**Tasks:**
- expose parser-state lifecycle in public headers
- preserve existing stateless API as a compatibility layer over the stateful path
- document parser-state behavior and incremental expectations

**Exit criteria:**
- stateful parser APIs are public and regression-covered
- parser-state lifecycle is documented in `README` and docs

---

## Sprint 6: Differential Validation Against Reference Parsers

**Goal:** Validate `iohttpparser` behavior against established HTTP/1.1 parser references without inheriting their unsafe leniency.

**Scope:**
- data-driven differential corpus
- `picohttpparser` adapter
- `llhttp` adapter
- explicit strict-vs-lenient divergence expectations

**Tasks:**
- build a reusable differential runner in `ctest`
- compare request and response parsing against `picohttpparser` and `llhttp`
- encode acceptable divergence in case metadata instead of ad hoc test code
- keep vendored/reference parser warning suppressions target-local

**Exit criteria:**
- reference-backed request and response differential corpus exists
- acceptable divergence is documented and regression-covered
- container baseline stays green with differential tests enabled

---

## Sprint 7: Consumer Contracts and Integration Guidance

**Goal:** Make `iohttpparser` usable as a documented integration surface for `iohttp` and `ioguard`.

**Scope:**
- public semantics API
- ownership flags
- named consumer policy presets
- consumer-oriented examples and docs

**Tasks:**
- expose public semantics helpers and ownership flags
- document `iohttp` and `ioguard` handoff rules
- add examples for generic request flow and `CONNECT` tunnel flow

**Exit criteria:**
- `iohttp` and `ioguard` integration contracts are explicit
- public semantics ownership is documented and regression-covered

---

## Sprint 8: Functional Contract Completion

**Goal:** finish the remaining core-library functionality before external integration campaigns.

**Scope:**
- response-side upgrade handoff
- `100 Continue` flow
- trailer ownership after chunked decode
- final policy-preset contract

**Tasks:**
- add explicit response-upgrade semantics coverage and guidance
- add `Expect: 100-continue` consumer flow guidance and/or example
- add trailer ownership guidance and/or example after chunked decode
- decide whether `IHTP_POLICY_IOHTTP` and `IHTP_POLICY_IOGUARD` stay equivalent or diverge
- regression-cover the final contract

**Exit criteria:**
- no important integration-sensitive HTTP/1.1 handoff case remains undocumented
- policy preset behavior is explicit and test-covered
- the remaining product gaps are no longer inside the core library contract

---

## Sprint 9: Public API and Documentation Freeze

**Goal:** freeze the public embedder contract before integration testing at the consumer level.

**Scope:**
- public headers
- `README`
- consumer docs
- ownership/lifetime/reset guarantees

**Tasks:**
- audit and normalize public header comments and guarantees
- ensure `README` matches actual API and examples
- keep `docs/en` authoritative and sync `docs/ru`
- write a final embedder-facing checklist for parser, semantics, and decoder handoff

**Exit criteria:**
- public API and docs no longer contradict implementation
- the repository is ready for real integration campaigns without hidden tribal knowledge

---

## Sprint 10: Consumer Integration Campaign

**Goal:** validate the frozen library contract against real consumer-style workflows in `iohttp` and `ioguard`.

**Scope:**
- `iohttp`-style request/response/body handoff
- `ioguard`-style strict-profile and CONNECT flows
- accumulated-buffer and incremental-call integration loops

**Tasks:**
- build consumer-style fixtures and integration scenarios
- validate partial parse loops and semantics/body ownership handoff
- validate CONNECT, upgrade, and `100 Continue` flows in consumer terms
- record any remaining API friction as library defects

**Exit criteria:**
- the library can be embedded in both target consumers without undocumented workarounds
- remaining issues are narrow defects, not missing functional surface

---

## Sprint 11: Reference Comparison Expansion

**Goal:** expand the maintained comparison posture against `picohttpparser` and `llhttp` after the library contract is frozen.

**Scope:**
- broader differential corpus
- explicit divergence classification
- parser and semantics comparison matrix

**Tasks:**
- expand differential corpus for remaining request/response edge cases
- classify expected strict-vs-lenient divergence explicitly
- separate accepted divergence from unexpected regressions
- produce a concise maintained comparison report

**Exit criteria:**
- the repository has an explicit documented comparison posture against `picohttpparser` and `llhttp`
- no important unclassified divergence remains in the maintained corpus

---

## Sprint 12: CI and Release Gating

**Goal:** turn the now functionally-complete and integration-validated repository into a repeatable release gate.

**Scope:**
- CI-required commands
- sanitizer presets
- fuzz smoke
- differential smoke
- docs lint and example validation

**Tasks:**
- define the mandatory CI/release validation matrix
- encode short-running smoke jobs for fuzz and differential coverage
- document which analyzer and sanitizer jobs are required vs advisory
- prepare release-candidate checklist and versioning policy

**Exit criteria:**
- release candidate gate is explicit and reproducible
- `main` has a documented minimum CI matrix for parser, semantics, decoder, docs, and quality checks

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

1. Sprint 8:
   - finish the remaining functional semantics and examples
2. Sprint 9:
   - freeze public API and documentation
3. Sprint 10:
   - run consumer-style integration campaigns for `iohttp` and `ioguard`
4. Sprint 11:
   - expand maintained comparison coverage against `picohttpparser` and `llhttp`
5. After Sprint 11:
   - start CI and release gating work
6. Long-tail technical decision:
   - decide whether SIMD token validation remains scalar-backed or earns a dedicated SIMD implementation with proof obligations
