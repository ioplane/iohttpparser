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

This is the primary remaining work.

Required items:
- CI matrix for core validation
- sanitizer jobs
- fuzz smoke jobs
- differential smoke jobs
- artifact publication checks
- release checklist
- release automation policy

### 2. Remaining evidence publication

The repository still lacks dedicated published evidence for:
- named preset zero-overhead proof;
- isolated zero-copy span ownership cost;
- scanner backend results inside the PMI/PSI artifact bundle.

These are not feature gaps. They are publication and observability gaps.

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

**Goal:** make the current product verifiable and repeatable in automation.

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

Exit criteria:
- every required validation step has an automated gate;
- `main` is protected by checks that reflect the current product contract.

## Phase B: Evidence Completion

**Goal:** finish the remaining publication targets from the extended-contract result matrix.

Tasks:
1. Publish named preset zero-overhead proof.
2. Publish isolated zero-copy span ownership cost.
3. Publish scanner backend result bundle inside PMI/PSI artifacts.
4. Update:
   - `09-test-results.md`
   - `11-extended-contract-results.md`
   - artifact manifests

Exit criteria:
- all remaining publication targets from `11-extended-contract-results.md` are closed.

## Phase C: Release Candidate Preparation

**Goal:** prepare the repository for the first release-candidate cut.

Tasks:
1. Freeze the release-candidate checklist.
2. Verify:
   - examples
   - public headers
   - Doxygen output
   - numbered docs
   - artifact publication path
3. Record supported host and container profiling/debug toolchain versions.
4. Define release notes structure.

Exit criteria:
- the repository can produce a repeatable release-candidate package with documented verification evidence.

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
- remaining publication targets from `11` are closed.

## Immediate Next Step

Start Phase A: Release Gate.
