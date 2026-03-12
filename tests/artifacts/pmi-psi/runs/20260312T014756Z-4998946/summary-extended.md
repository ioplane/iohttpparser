# Extended PSI Run Summary

Run id: `20260312T014756Z-4998946`

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
| stateful-reuse-request | req-small/iohttpparser-stateful-strict | 7981577.56 | 1019.98 | 125.29 |

### Semantics And Body Handoff

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| request-chunked-parse | req-headers/iohttpparser-stateful-strict | 7852263.74 | 666.48 | 127.35 |
| request-chunked-parse-semantics | request-chunked-parse | 8154989.49 | 692.17 | 122.62 |
| request-chunked-parse-semantics-body | request-chunked-parse-semantics | 4075425.45 | 380.89 | 245.37 |
| response-fixed-parse-semantics-body | resp-headers/iohttpparser-stateful-strict | 16895333.75 | 692.84 | 59.19 |

### iohttp-style Consumer Flows

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| consumer-iohttp-expect-trailers | request-chunked-parse-semantics-body | 3913056.27 | 966.53 | 255.55 |
| consumer-iohttp-fixed-response | response-fixed-parse-semantics-body | 8018723.08 | 1032.38 | 124.71 |
| consumer-iohttp-pipeline | request-chunked-parse-semantics-body | 3418498.73 | 704.19 | 292.53 |

### Upgrade Handoff

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| response-upgrade-parse | resp-upgrade/iohttpparser-stateful-strict | 10138496.42 | 744.50 | 98.63 |
| response-upgrade-parse-semantics | response-upgrade-parse | 10942576.59 | 803.55 | 91.39 |

### ioguard-style Consumer Flows

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| consumer-ioguard-connect | req-connect/iohttpparser-stateful-strict | 17046011.45 | 1056.66 | 58.66 |
| consumer-ioguard-reject-te-cl | request-chunked-parse-semantics | 7838169.09 | 680.23 | 127.58 |
