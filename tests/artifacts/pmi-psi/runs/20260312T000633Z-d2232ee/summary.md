# PSI Run Summary

Run id: `20260312T000633Z-d2232ee`

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

| Parser | Scenario | req/s median | MiB/s median | ns/req median |
|---|---|---:|---:|---:|
| iohttpparser-lenient | req-headers | 5615055.18 | 996.02 | 178.09 |
| iohttpparser-lenient | req-small | 15347760.69 | 717.20 | 65.16 |
| iohttpparser-lenient | resp-headers | 8141790.25 | 900.70 | 122.82 |
| iohttpparser-lenient | resp-small | 16892389.82 | 821.60 | 59.20 |
| iohttpparser-lenient | resp-upgrade | 13000813.27 | 954.69 | 76.92 |
| iohttpparser-strict | req-headers | 5703031.35 | 1011.62 | 175.35 |
| iohttpparser-strict | req-small | 15802711.37 | 738.46 | 63.28 |
| iohttpparser-strict | resp-headers | 8975082.39 | 992.88 | 111.42 |
| iohttpparser-strict | resp-small | 18919582.13 | 920.20 | 52.86 |
| iohttpparser-strict | resp-upgrade | 12829842.42 | 942.13 | 77.94 |
| llhttp | req-headers | 7799817.98 | 1383.56 | 128.21 |
| llhttp | req-small | 22734335.87 | 1062.38 | 43.99 |
| llhttp | resp-headers | 9275587.30 | 1026.12 | 107.81 |
| llhttp | resp-small | 17222275.95 | 837.65 | 58.06 |
| llhttp | resp-upgrade | 13391228.97 | 983.36 | 74.68 |
| picohttpparser | req-headers | 13106707.01 | 2324.91 | 76.30 |
| picohttpparser | req-small | 39873576.84 | 1863.29 | 25.08 |
| picohttpparser | resp-headers | 17615896.30 | 1948.78 | 56.77 |
| picohttpparser | resp-small | 40101939.13 | 1950.45 | 24.94 |
| picohttpparser | resp-upgrade | 27851809.43 | 2045.24 | 35.90 |

## CONNECT median matrix

| Parser | Scenario | req/s median | MiB/s median | ns/req median |
|---|---|---:|---:|---:|
| iohttpparser-lenient | req-connect | 9600916.77 | 906.46 | 104.16 |
| iohttpparser-strict | req-connect | 9788064.90 | 924.13 | 102.17 |
| llhttp | req-connect | 11501856.00 | 1085.93 | 86.94 |
| picohttpparser | req-connect | 24784351.36 | 2339.98 | 40.35 |
