# Sprint 11: Comparison Campaign

## Goal

Expand the existing `picohttpparser` and `llhttp` differential coverage into a structured comparison campaign that distinguishes:

- intentional strictness
- acceptable behavioral divergence
- actual library defects

## Initial Focus

1. Classify request/response divergences that matter to real consumers:
   - bare `LF`
   - `TE + CL`
   - duplicate `Content-Length`
   - non-chunked `Transfer-Encoding`
   - `CONNECT`
   - `101 Upgrade`
2. Tie the comparison matrix back to Sprint 10 consumer scenarios instead of comparing parser behavior in isolation.
3. Keep `IHTP_POLICY_IOHTTP` and `IHTP_POLICY_IOGUARD` explicitly tracked as current strict aliases, but mark any future preset divergence as a separate, opt-in follow-up.

## First Batch

- extend the differential corpus with response/body handoff cases that now exist in `test_iohttp_integration`
- record which `picohttpparser` / `llhttp` differences are expected and why
- avoid changing parser behavior unless the comparison exposes a concrete defect
- run the comparison through a reproducible helper: `scripts/run-sprint11-comparison.sh`

## Completed In This Batch

- parser-level differential coverage now includes:
  - `CONNECT` authority-form request parsing against `picohttpparser` and `llhttp`
  - `101 Switching Protocols` response parsing against `picohttpparser` and `llhttp`
- semantics-level differential coverage now includes:
  - request-side `Upgrade` handling against `llhttp`
  - response-side `101 Upgrade` handling against `llhttp`
- the campaign now records a real classified divergence:
  - `llhttp` keeps `keep_alive=true` on `101 Upgrade`
  - `iohttpparser` sets `keep_alive=false` and transfers ownership to the upgraded protocol
- host SIMD verification and direct-backend benchmarks are now captured under `docs/tmp/comparison/results/`
- the comparison runner was normalized to:
  - execute correctness checks first
  - save raw scanner-bench output separately
  - extract a clean `scanner-bench.tsv` artifact for machine-readable analysis

## Residual Risks Carried From Sprint 10

- named consumer presets are currently aliases of strict mode, so comparison should not imply policy divergence that does not exist
- CONNECT coverage is request-side only; if response-side CONNECT semantics are introduced later, the comparison matrix must be expanded with those cases
