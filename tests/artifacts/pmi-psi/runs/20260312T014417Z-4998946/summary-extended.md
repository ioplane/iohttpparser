# Extended PSI Run Summary

Run id: `20260312T014417Z-4998946`

## Scope

This summary publishes the extended-contract performance layer.

## Source code used for the run

- `bench/bench_extended_contract.c`
- `scripts/run-throughput-extended.sh`
- `scripts/run-throughput-extended-median.sh`
- `scripts/run-pmi-psi.sh`
- `tests/unit/test_iohttp_integration.c`
- `tests/unit/test_semantics.c`
- `tests/unit/test_body_decoder.c`

## Median matrix

Full values are published in `throughput-extended-median.tsv`.

### Parser State

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| stateful-reuse-request | req-small/iohttpparser-stateful-strict | 7927827.59 | 1013.12 | 126.14 |

### Semantics And Body Handoff

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| request-chunked-parse | req-headers/iohttpparser-stateful-strict | 7922386.28 | 672.43 | 126.22 |
| request-chunked-parse-semantics | request-chunked-parse | 7986512.38 | 677.87 | 125.21 |
| request-chunked-parse-semantics-body | request-chunked-parse-semantics | 5152236.88 | 481.53 | 194.09 |

### iohttp-style Consumer Flows

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| consumer-iohttp-expect-trailers | request-chunked-parse-semantics-body | 3860890.79 | 953.65 | 259.01 |
| consumer-iohttp-pipeline | request-chunked-parse-semantics-body | 3965702.23 | 816.91 | 252.16 |

### Upgrade Handoff

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| response-upgrade-parse | resp-upgrade/iohttpparser-stateful-strict | 9951652.38 | 730.78 | 100.49 |
| response-upgrade-parse-semantics | response-upgrade-parse | 10646478.11 | 781.80 | 93.93 |

### ioguard-style Consumer Flows

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| consumer-ioguard-connect | req-connect/iohttpparser-stateful-strict | 17209201.07 | 1066.78 | 58.11 |
| consumer-ioguard-reject-te-cl | request-chunked-parse-semantics | 8338019.30 | 723.61 | 119.93 |
