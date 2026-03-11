# iohttpparser Final Development Plan

**Goal:** finish the remaining product functionality of `iohttpparser` before moving to full integration testing, reference-comparison expansion, and only then CI/release automation.

**Definition of functional completeness for this plan:**
- the public parser API is stable for request, response, and headers-only flows
- semantics handoff is explicit for all integration-sensitive cases
- body/framing ownership is documented and example-covered
- named consumer presets are either truly distinct or intentionally frozen as equivalent with explicit rationale
- no known missing method- or status-specific behavior remains for core HTTP/1.1 integration

**Not part of this plan:**
- broad CI rollout
- release automation
- publication work
- large SIMD rewrites without proof obligations

## Current baseline

Already merged on `main`:
- scalar parser strictness
- semantics/framing hardening
- body decoder completion
- scanner equivalence and proof-focused fuzzing
- public stateful parser API
- differential validation against `picohttpparser` and `llhttp`
- consumer contracts, policy presets, and `CONNECT` example

This means the remaining work is no longer “build the parser”, but “close the last functional gaps and freeze the integration contract”.

## Remaining functional gaps

### 1. Consumer-facing semantics gaps

These are partially implemented but not yet fully closed as a library contract:
- response-side protocol upgrade handoff
- `Expect: 100-continue` integration flow
- trailer ownership after chunked decode
- CONNECT response/tunnel lifecycle wording and example completeness

### 2. Consumer preset completeness

`IHTP_POLICY_IOHTTP` and `IHTP_POLICY_IOGUARD` exist, but the project still needs an explicit final decision:
- keep them equivalent for now and document that as a stable contract
- or make them behaviorally distinct in a narrow, test-covered way

This must be resolved before claiming full functional readiness.

### 3. Public contract completeness

The library still needs a final pass on:
- example coverage
- header-level docs for ownership/lifetime/consumed-byte guarantees
- limit and policy documentation for embedders

### 4. Release-blocking functional evidence

Before integration test campaigns begin, the repository should have a single explicit checklist for:
- parser correctness surface
- semantics surface
- decoder surface
- examples and docs surface

This is still product work, not CI work.

## Proposed sprint breakdown

## Sprint 8: Functional Contract Completion

**Goal:** finish all remaining core-library functionality and freeze the semantics contract.

**Scope:**
- response upgrade handoff
- `100 Continue` flow
- trailer ownership model
- final consumer preset decision

**Tasks:**
1. Add explicit response-upgrade semantics coverage.
2. Add a minimal response-upgrade example if docs-only guidance is not sufficient.
3. Add a concrete `Expect: 100-continue` consumer example or documented sequence.
4. Add a concrete trailer-consumption ownership example after chunked decode.
5. Decide and codify the final contract for:
   - `IHTP_POLICY_IOHTTP`
   - `IHTP_POLICY_IOGUARD`
6. Make all of the above regression-covered in unit/corpus tests.

**Exit criteria:**
- no major integration-sensitive HTTP/1.1 handoff case remains undocumented
- policy preset contract is explicit and test-covered
- examples cover request flow, CONNECT flow, and the remaining ownership-sensitive cases

## Sprint 9: Public API and Documentation Freeze

**Goal:** freeze the product-facing contract before external integration campaigns.

**Scope:**
- README
- public headers
- consumer docs
- versioning expectations for embedders

**Tasks:**
1. Audit public headers for completeness and consistency.
2. Ensure `README` matches actual API and examples.
3. Make `docs/en` authoritative, then sync `docs/ru`.
4. Add a final “embedding contract” section covering:
   - buffer ownership
   - span lifetime
   - state reset/reuse
   - semantics/body-decoder handoff
   - policy and limits expectations
5. Write a functional readiness checklist for maintainers.

**Exit criteria:**
- public API and docs no longer contradict implementation
- the embedder contract is explicit enough for `iohttp` and `ioguard`
- docs are ready for integration consumers without hidden tribal knowledge

## Sprint 10: Integration Test Campaign

**Goal:** validate the frozen library contract against real consumer-style workflows.

**Scope:**
- `iohttp` integration scenarios
- `ioguard` integration scenarios
- end-to-end request/response/body handoff paths

**Tasks:**
1. Build consumer-style integration fixtures for `iohttp`.
2. Build strict-profile integration fixtures for `ioguard`.
3. Validate:
   - partial parse loops
   - semantics handoff
   - body decoder handoff
   - CONNECT tunnel flow
   - upgrade and `100 Continue` flows
4. Record any API friction as contract defects, not “consumer bugs”.

**Exit criteria:**
- the library can be embedded in both target consumers without undocumented workarounds
- remaining issues are narrow defects, not missing product surface

## Sprint 11: Reference Comparison Expansion

**Goal:** expand comparison coverage against `picohttpparser` and `llhttp` after the library contract is frozen.

**Scope:**
- broader corpus
- explicit divergence documentation
- parser + semantics comparison matrix

**Tasks:**
1. Expand differential corpus for remaining request/response edge cases.
2. Encode accepted divergence with stronger metadata instead of ad hoc comments.
3. Separate:
   - expected strict-vs-lenient divergence
   - unexpected regressions
4. Produce a concise comparison report for maintainers.

**Exit criteria:**
- the project has an explicit documented comparison posture against `picohttpparser` and `llhttp`
- no important unclassified divergence remains in the maintained corpus

## Only after functional completion

CI and release engineering should start **after** Sprints 8-11 are complete enough.

That later phase should cover:
- minimum CI matrix
- sanitizer jobs
- fuzz smoke jobs
- differential smoke jobs
- docs lint
- release candidate automation

## Supporting Tooling

The dev container should carry the profiling/debug stack needed for throughput and bottleneck campaigns so that comparisons remain reproducible across maintainers. Keep these available in the image:
- `gdb`
- `valgrind`
- `uftrace`
- `ftracer` helper components

Use them in this order during performance work:
1. repo-level benchmark/throughput scripts for coarse deltas
2. optional parser trace mode for per-layer counters
3. `uftrace` for function-level hot-path confirmation
4. `valgrind` (`callgrind`/`cachegrind`) for cost-model confirmation
5. `gdb` and `ftracer` for focused control-flow/debug investigations

## Recommended execution order

1. Sprint 8: finish remaining functional semantics and examples
2. Sprint 9: freeze public contract and documentation
3. Sprint 10: run consumer-style integration campaigns
4. Sprint 11: expand `picohttpparser` / `llhttp` comparison coverage
5. Only then start CI and release gating work
