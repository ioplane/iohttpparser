# PSI Run Summary

Run id: `20260312T014417Z-4998946`

## Source code used for the run

- `bench/bench_throughput_compare.c`
- `scripts/run-throughput-compare.sh`
- `scripts/run-throughput-median.sh`
- `scripts/run-pmi-psi.sh`
- `tests/unit/test_differential_corpus.c`
- `tests/unit/test_semantics_differential.c`
- `tests/unit/test_iohttp_integration.c`

## Functional result

`ctest --preset clang-debug --output-on-failure` completed successfully.

## Common median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-median.tsv`.

| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |
|---|---:|---:|---:|---:|---:|---:|
| req-small | 41012340.41 | 22328358.84 | 19974797.80 | 18696689.31 | 18029691.84 | 17222485.06 |
| req-headers | 13757967.50 | 7503249.47 | 8965967.52 | 8691342.40 | 8402852.94 | 7728741.57 |
| resp-small | 35584744.54 | 17873145.71 | 20979653.30 | 22220651.96 | 22358479.77 | 21934168.10 |
| resp-headers | 17054980.14 | 9860236.57 | 12084146.75 | 11800798.28 | 12464903.51 | 11596253.25 |
| resp-upgrade | 28033458.49 | 11509832.16 | 16219758.26 | 14980574.31 | 16319557.83 | 15984877.03 |
| req-line-only | 63580005.74 | 28275149.44 | 25425379.31 | 25990646.49 | 22121772.17 | 19332256.14 |
| req-line-hot | 39257144.90 | 20539599.94 | 22505995.32 | 20744798.42 | 16848235.89 | 16844726.66 |
| req-line-long-target | 47926358.27 | 18971544.96 | 23695609.99 | 20857225.72 | 11854276.80 | 11194055.33 |
| req-line-connect | 61667279.74 | 28546813.06 | 25795654.05 | 23221244.00 | 19867821.37 | 18350208.03 |
| req-line-options | 63036698.39 | 22793711.22 | 25724240.26 | 23451401.37 | 19234717.90 | 17238200.65 |
| req-pico-bench | 7598779.00 | 3119736.42 | 4421369.71 | 4492819.63 | 3769359.38 | 3769498.13 |
| hdr-common-heavy | 13787831.74 | 7501617.44 | 10003793.94 | 9161697.78 | 9862686.26 | 9966125.14 |
| hdr-name-heavy | 9374509.74 | 4542074.93 | 4720716.62 | 4320628.86 | 4389410.79 | 4533897.33 |
| hdr-uncommon-valid | 16192481.98 | 8929463.98 | 9224812.27 | 9158794.49 | 9689202.36 | 9079825.10 |
| hdr-value-ascii-clean | 10996416.05 | 5140571.89 | 7869366.01 | 7771013.41 | 7906839.72 | 7290850.47 |
| hdr-value-heavy | 8319650.63 | 3479655.39 | 4936043.32 | 4998201.15 | 4646653.59 | 4684833.23 |
| hdr-value-obs-text | 16762940.42 | 9911613.68 | 10162084.74 | 11281011.29 | 11979849.89 | 11012169.82 |
| hdr-value-trim-heavy | 9848973.38 | 4940561.83 | 7278138.50 | 7126635.39 | 7619178.23 | 7315880.86 |
| hdr-count-04-minimal | 20911612.47 | 13631167.06 | 14064837.07 | 13064810.54 | 12828087.97 | 13011877.50 |
| hdr-count-16-minimal | 5630918.52 | 3795482.84 | 5264762.54 | 5279886.52 | 4959737.10 | 5179242.85 |
| hdr-count-32-minimal | 2896710.64 | 1901175.29 | 2957657.01 | 2914143.42 | 2905643.15 | 2865296.51 |

## CONNECT median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-connect-median.tsv`.

| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |
|---|---:|---:|---:|---:|---:|---:|
| req-connect | 24636427.92 | 11297970.12 | 14698236.03 | 13562741.68 | 12350393.51 | 11993786.50 |
