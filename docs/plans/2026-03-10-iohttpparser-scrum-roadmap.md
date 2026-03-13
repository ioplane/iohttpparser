# iohttpparser Scrum Roadmap

**Goal:** maintain `iohttpparser` as a production-ready C23 HTTP/1.1 parser library with:
- strict default wire-level behavior;
- explicit parser, semantics, and body-decoder contracts;
- reproducible comparison and PSI artifacts;
- consumer-ready integration for `iohttp` and `ioguard`.

## Current Status (2026-03-14)

### Repository state

- `main` is clean and synchronized with `origin/main`
- current head: `132129c`
- development and validation are performed inside the dev container
- published PMI/PSI artifacts are stored in `tests/artifacts/pmi-psi/`

### Completed product work

- parser-core correctness is complete for:
  - request parsing
  - response parsing
  - headers-only parsing
- public API surface is complete for:
  - stateless parsing
  - stateful parsing
  - semantics application
  - body decoding
- semantics and framing hardening are complete for:
  - `Content-Length`
  - `Transfer-Encoding`
  - `TE + CL`
  - duplicate `Content-Length`
  - no-body response precedence
  - `Host`
  - keep-alive / close
- body layer is complete for:
  - chunked decoding
  - fixed-length accounting
  - trailer handling
  - incremental decode behavior
- consumer contract work is complete for:
  - `CONNECT`
  - `Expect: 100-continue`
  - protocol upgrade handoff
  - trailer ownership
  - `IHTP_POLICY_IOHTTP`
  - `IHTP_POLICY_IOGUARD`
- differential comparison is complete against:
  - `picohttpparser`
  - `llhttp`
- published testing and result documents exist for:
  - common PMI/PSI matrix
  - extended-contract methodology
  - extended-contract results
- profiler stack is integrated in the dev image:
  - `gdb`
  - `valgrind`
  - `uftrace`
  - `ftracer`

### Documentation state

- stable numbered documentation exists in:
  - `docs/en/01-12`
  - `docs/ru/01-12`
- common comparative results are published in:
  - `docs/en/09-test-results.md`
  - `docs/ru/09-test-results.md`
- extended-contract methodology and results are published in:
  - `docs/en/10-extended-contract-methodology.md`
  - `docs/en/11-extended-contract-results.md`
  - `docs/ru/10-extended-contract-methodology.md`
  - `docs/ru/11-extended-contract-results.md`
- numeric charts are published as repository-tracked `SVG` artifacts

## Readiness Assessment

| Area | Status |
|---|---|
| parser-core correctness | complete |
| semantics and framing security | complete |
| body decoding | complete |
| consumer integration contract | complete |
| differential comparison posture | complete |
| PMI/PSI publication | complete |
| public documentation | complete |
| release gating and CI | complete |
| release-candidate verification | complete |

Overall product state:
- functional readiness: `95%`
- documentation readiness: `95%`
- verification readiness: `96%`
- release readiness: `88%`

## Closed Sprints

| Sprint | Status | Result |
|---|---|---|
| Sprint 0 | complete | tooling baseline and repository hygiene |
| Sprint 1 | complete | scalar parser correctness |
| Sprint 2 | complete | semantics and framing hardening |
| Sprint 3 | complete | body decoder completion |
| Sprint 4 | complete | scanner equivalence and proof base |
| Sprint 5 | complete | public stateful parser API |
| Sprint 6 | complete | differential verification |
| Sprint 7 | complete | consumer contracts |
| Sprint 8 | complete | remaining functional semantics and examples |
| Sprint 9 | complete | public API and documentation freeze |
| Sprint 10 | complete | consumer integration campaign |
| Sprint 11 | complete | comparison campaign and report |
| Sprint 12 | complete | PMI/PSI publication |

## Remaining Work

### 1. Release-candidate preparation

This work is complete.

Published release-candidate baseline:
- run id: `20260313T215048Z-132129c`
- artifact root: `tests/artifacts/release-candidate/`

### 2. Optional performance work

Further performance work is optional and must not change the contract. Current conclusions:
- `picohttpparser` remains the raw-throughput leader because it has a thinner parser contract
- `llhttp` is no longer the primary performance concern on hot-path stateful scenarios
- remaining optimization work should focus on cost visibility and regression control, not on widening scope

## Next Execution Queue

1. Maintain the published release-candidate evidence bundle.
2. Keep common PMI/PSI and extended-contract artifacts current after changes.
3. Perform optional performance follow-up only with published before/after evidence.
4. Prepare the first tagged release once release naming and packaging policy are fixed.

## Working Rules

- keep development container-first
- treat `docs/en` as authoritative and `docs/ru` as synchronized derivative documentation
- use `SVG` for numeric result charts
- use `Mermaid` only for structural diagrams
- accept performance changes only when they preserve:
  - `quality.sh`
  - differential tests
  - consumer integration tests
  - published reproducible measurements
