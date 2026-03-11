# Sprint 11 Comparison Report

## Scope

This report covers the first comparison campaign after Sprint 10 consumer integration was merged.

The campaign focuses on:

- parser-level comparison against `picohttpparser` and `llhttp`
- semantics-level comparison against `llhttp`
- host SIMD capability and backend-equivalence validation
- consumer-relevant scenarios added during Sprint 10

## Method

1. Reuse the maintained differential harnesses:
   - `tests/unit/test_differential_corpus.c`
   - `tests/unit/test_semantics_differential.c`
2. Extend the corpus with Sprint 10 consumer-sensitive cases:
   - `CONNECT` request authority-form parsing
   - `101 Switching Protocols` response parsing
   - request/response protocol-upgrade semantics
3. Validate scanner equivalence on the current host:
   - `test_scanner_backends`
   - `test_scanner_corpus`
4. Measure direct backend performance with:
   - `bench/bench_parser.c`
   - TSV output saved under `docs/tmp/comparison/results/`

## Reference Snapshots

- upstream `picohttpparser`: `f832609`
- upstream `llhttp`: `c893b86`

These snapshots were downloaded into `docs/tmp/comparison/` for the comparison campaign.

## Host SIMD Baseline

The current host is `x86_64` under KVM and exposes:

- `sse4_2`
- `avx2`

This means the Sprint 11 scanner campaign can exercise:

- scalar
- SSE4.2
- AVX2
- runtime dispatch on top of the same host

## Results

### Differential Campaign

The extended comparison cluster passed:

- `test_differential_corpus`
- `test_semantics_differential`
- `test_scanner_backends`
- `test_scanner_corpus`
- `test_iohttp_integration`

Covered consumer-sensitive additions now include:

- parser-level `CONNECT` request comparison against `picohttpparser` and `llhttp`
- parser-level `101 Switching Protocols` response comparison against `picohttpparser` and `llhttp`
- semantics-level request upgrade comparison against `llhttp`
- semantics-level response `101 Upgrade` comparison against `llhttp`

### Classified Divergences

- `llhttp` response-upgrade semantics treats `keep_alive` differently from `iohttpparser`.
  `iohttpparser` marks HTTP persistence as ended once protocol ownership transfers.
  The differential corpus now records this explicitly with separate reference and `ihtp` expectations.
- `llhttp` upgrade paths surface as `HPE_PAUSED_UPGRADE`, so the Sprint 11 harness maps that to successful parsing/semantics capture instead of treating it as a generic error.
- No new unexplained request or response parser divergences were found in the added `CONNECT` and `101 Switching Protocols` cases.

### RFC-Grounded Interpretation

- The `101 Upgrade` keep-alive split is not treated as a defect in `iohttpparser`.
  For consumer-facing handoff, the library follows the stricter ownership model:
  once the protocol switches, HTTP persistence semantics are no longer surfaced as reusable keep-alive state.
- This matches the already documented embedder contract for `iohttp` and `ioguard`:
  upgraded bytes become consumer-owned immediately after the header block.

### SIMD Verification

Scanner equivalence checks passed on this host for:

- scalar
- SSE4.2
- AVX2
- dispatch-vs-direct backend behavior

Benchmark raw data is saved under:

- `docs/tmp/comparison/results/scanner-bench.tsv`

Supporting host/reference metadata is saved under:

- `docs/tmp/comparison/results/host.txt`
- `docs/tmp/comparison/results/references.txt`

### Benchmark Summary

Average `ns/op` across the 64 recorded backend/case rows:

- `avx2`: `8.41`
- `dispatch`: `8.52`
- `sse42`: `9.12`
- `scalar`: `11.17`

Practical interpretation:

- `dispatch` tracks `avx2` closely on this host because the active backend is `avx2`.
- `avx2` wins several realistic scan paths, including:
  - `query-delim`
  - `status-space`
  - `header` token validation
  - `wide-valid` token validation
- `sse42` still wins some specific long-search cases:
  - `long-header-crlf`
  - `wide-not-found`
- `scalar` remains best on a few very short microcases:
  - `request-space`
  - `conn-token`
  - `invalid-colon`

Engineering conclusion:

- keep runtime dispatch enabled
- keep scalar as the baseline truth implementation
- do not force additional SIMD specialization simply because a SIMD backend exists
- continue treating token-validation SIMD work as proof-driven, not benchmark-driven alone

## Assessment

The comparison posture is now materially stronger than before Sprint 11:

- Sprint 10 consumer scenarios are no longer integration-only; they are also represented in the maintained differential matrix where the reference libraries can express them.
- Known reference differences are now encoded explicitly instead of being left as implicit surprises.
- SIMD validation has been rechecked on a host that actually exposes both `sse4_2` and `avx2`.
- Consumer-facing ownership decisions now have comparison evidence behind them, not just unit-level assertions.

## Next Step

The next batch should move from comparison expansion to broader reporting and integration interpretation:

- decide whether any additional comparison cases are still missing for `iohttp` / `ioguard`
- keep the `101 Upgrade` keep-alive divergence classified as intentional unless a consumer disproves it
- treat the current TSV results as a baseline for future performance work, not as a release gate by themselves
