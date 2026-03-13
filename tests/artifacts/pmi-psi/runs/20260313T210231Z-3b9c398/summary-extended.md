# Extended PSI Run Summary

Run id: `20260313T210231Z-3b9c398`

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
| stateful-reuse-request | req-small/iohttpparser-stateful-strict | 7173783.07 | 916.75 | 139.40 |

### Semantics And Body Handoff

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| request-chunked-parse | req-headers/iohttpparser-stateful-strict | 7653287.11 | 649.59 | 130.66 |
| request-chunked-parse-semantics | request-chunked-parse | 7590657.40 | 644.27 | 131.74 |
| request-chunked-parse-semantics-body | request-chunked-parse-semantics | 4932477.34 | 460.99 | 202.74 |
| response-fixed-parse-semantics-body | resp-headers/iohttpparser-stateful-strict | 17064526.09 | 699.78 | 58.60 |

### iohttp-style Consumer Flows

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| consumer-iohttp-expect-trailers | request-chunked-parse-semantics-body | 3581515.46 | 884.64 | 279.21 |
| consumer-iohttp-fixed-response | response-fixed-parse-semantics-body | 7849751.05 | 1010.62 | 127.39 |
| consumer-iohttp-pipeline | request-chunked-parse-semantics-body | 3736765.92 | 769.75 | 267.61 |

### Upgrade Handoff

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| response-upgrade-parse | resp-upgrade/iohttpparser-stateful-strict | 9647043.15 | 708.41 | 103.66 |
| response-upgrade-parse-semantics | response-upgrade-parse | 10398724.53 | 763.61 | 96.17 |

### ioguard-style Consumer Flows

| Scenario | Baseline | req/s median | MiB/s median | ns/op median |
|---|---|---:|---:|---:|
| consumer-ioguard-connect | req-connect/iohttpparser-stateful-strict | 17192341.12 | 1065.73 | 58.17 |
| consumer-ioguard-reject-te-cl | request-chunked-parse-semantics | 8234980.90 | 714.67 | 121.43 |
