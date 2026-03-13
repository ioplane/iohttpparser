# iohttpparser Final Development Plan

**Scope:** define the post-functional plan for `iohttpparser` after completion of parser, semantics, body, consumer-contract, comparison, and PMI/PSI work.

## Baseline

Already completed on `main`:
- parser-core implementation
- stateful and stateless public APIs
- semantics hardening
- body decoder
- consumer contracts for `iohttp` and `ioguard`
- differential comparison against `picohttpparser` and `llhttp`
- common PMI/PSI publication
- extended-contract methodology and result publication

This means the repository is no longer in feature-development mode.

## Product Completion Statement

The following statements are already true:
- the public parser contract is defined and documented;
- the wire-level semantics contract is defined and documented;
- the body-decoder contract is defined and documented;
- `iohttp` and `ioguard` consumer flows are covered by tests and published results;
- common parser-core comparison is published;
- extended-contract verification is published.

Functional development is complete.

## Remaining Work Categories

### 1. Release-readiness work

This work is complete on `main`.

Completed items:
- self-contained CI release gate
- artifact publication checks
- release checklist
- release-candidate evidence publication

### 2. Remaining evidence publication

This work is complete on `main`.

Completed items:
- named preset proof
- isolated zero-copy span ownership cost
- scanner backend results inside the PMI/PSI artifact bundle

### 3. Optional performance tuning

Further optimization is optional.

Rules:
- no contract weakening
- no scope expansion
- no hidden behavior changes
- every accepted optimization must keep:
  - `quality.sh` green
  - differential tests green
  - consumer integration tests green
  - reproducible throughput/profiling evidence

## Recommended Work Phases

## Phase A: Release Gate

**Status:** complete.

Tasks:
1. Add CI for:
   - `cmake --preset clang-debug`
   - `cmake --build --preset clang-debug`
   - `ctest --preset clang-debug`
   - `./scripts/quality.sh`
   - `python3 scripts/lint-docs.py`
2. Add differential smoke execution.
3. Add artifact integrity checks for published PMI/PSI outputs.
4. Define required status checks for protected `main`.

Exit criteria: satisfied.

## Phase B: Evidence Completion

**Status:** complete.

Tasks:
1. Publish named preset zero-overhead proof.
2. Publish isolated zero-copy span ownership cost.
3. Publish scanner backend result bundle inside PMI/PSI artifacts.
4. Update:
   - `09-test-results.md`
   - `11-extended-contract-results.md`
   - artifact manifests

Exit criteria: satisfied.

## Phase C: Release Candidate Preparation

**Status:** complete.

**Goal:** prepare the repository for the first release-candidate cut.

Published implementation:
- numbered release-candidate checklist in `docs/en/12` and `docs/ru/12`;
- root `SUPPORT.md`;
- `scripts/run-release-candidate.sh`;
- published artifacts under `tests/artifacts/release-candidate/`;
- published run id `20260313T215048Z-132129c`.

Tasks:
Completed tasks:
1. Freeze the release-candidate checklist.
2. Verify:
   - examples
   - public headers
   - Doxygen output
   - numbered docs
   - artifact publication path
3. Record supported host and container profiling/debug toolchain versions.
4. Define release notes structure.
5. Publish release-candidate verification artifacts.

Exit criteria:
Satisfied:
- the repository can produce a repeatable release-candidate package with documented verification evidence;
- `scripts/run-release-candidate.sh` completes successfully;
- `tests/artifacts/release-candidate/` contains a published run bundle.

## Phase D: Optional Performance Follow-up

**Goal:** perform post-completion performance work only where the evidence justifies it.

Allowed targets:
- generic value-validation path
- remaining uncommon header path cost
- publication-quality profiler reports

Disallowed targets:
- semantic scope changes
- parser/body contract rewrites
- large SIMD rewrites without proof obligations

Exit criteria:
- each accepted performance change has published before/after evidence;
- comparison posture against `picohttpparser` and `llhttp` stays documented.

## Definition of Done For The Repository

The repository is considered fully complete for first release-candidate work when all of the following are true:
- functional surface is complete;
- public contract is frozen and documented;
- consumer integration evidence is published;
- comparison evidence is published;
- CI and release gate are automated;
- remaining publication targets from `11` are closed;
- release-candidate verification artifacts are published.

## Immediate Next Step

Proceed to first tagged-release policy and packaging decisions.
