# PSI Run Summary

Run id: `20260313T210231Z-3b9c398`

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
| req-small | 40385707.74 | 22907400.38 | 18356247.59 | 16147832.76 | 16721689.39 | 17002573.76 |
| req-headers | 13729089.91 | 7872263.39 | 9119757.79 | 8181233.63 | 8250082.78 | 7990219.65 |
| resp-small | 38847518.44 | 17837704.54 | 21488479.01 | 21222363.57 | 21923305.26 | 20950136.89 |
| resp-headers | 17362885.08 | 9553977.80 | 11277604.26 | 11022673.14 | 12080476.75 | 11825557.42 |
| resp-upgrade | 28136039.44 | 14175027.57 | 15592956.47 | 15271738.44 | 16685821.99 | 15634622.62 |
| req-line-only | 62675255.68 | 28468433.84 | 26210918.21 | 24023169.87 | 21875754.03 | 19468426.30 |
| req-line-hot | 40496929.73 | 21099441.24 | 21535335.93 | 17802910.74 | 16811401.63 | 15235967.43 |
| req-line-long-target | 48089673.78 | 20978064.50 | 22453054.59 | 19219741.90 | 11621525.95 | 11125405.40 |
| req-line-connect | 63325968.52 | 27047799.01 | 25711822.15 | 21209584.74 | 20064793.23 | 17835903.80 |
| req-line-options | 59118495.93 | 26766896.20 | 27038383.15 | 20541439.48 | 19729749.78 | 17569974.18 |
| req-pico-bench | 7630888.72 | 3207538.28 | 4595405.77 | 4119853.71 | 3347272.04 | 3861665.94 |
| hdr-common-heavy | 14119672.98 | 7817491.28 | 10085704.79 | 9402108.63 | 9632872.91 | 9519043.20 |
| hdr-name-heavy | 9084211.96 | 4667237.84 | 4460687.19 | 4472631.06 | 4521647.97 | 4530327.27 |
| hdr-uncommon-valid | 16780859.01 | 9402431.74 | 8879926.08 | 9249106.65 | 9341846.81 | 9249187.92 |
| hdr-value-ascii-clean | 11384523.08 | 5164090.66 | 7723633.86 | 7619213.06 | 7502138.86 | 6971292.15 |
| hdr-value-heavy | 8768061.88 | 3545508.56 | 4667891.32 | 4601846.30 | 4690084.05 | 4450565.65 |
| hdr-value-obs-text | 17059833.27 | 9990481.07 | 11020294.70 | 10768156.11 | 11486560.84 | 11345282.79 |
| hdr-value-trim-heavy | 9858324.54 | 5152707.57 | 7525828.83 | 6036506.32 | 7375626.65 | 7328534.46 |
| hdr-count-04-minimal | 21092265.06 | 13724967.03 | 12656690.62 | 13239704.79 | 13494370.65 | 13384678.75 |
| hdr-count-16-minimal | 5640673.47 | 3884668.77 | 5278457.50 | 5269294.27 | 5040991.33 | 4449032.98 |
| hdr-count-32-minimal | 2861859.61 | 1931835.23 | 2891732.52 | 2846453.73 | 2729439.52 | 2828569.30 |

## CONNECT median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-connect-median.tsv`.

| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |
|---|---:|---:|---:|---:|---:|---:|
| req-connect | 23549243.00 | 11360359.29 | 14317185.50 | 13411627.73 | 12248678.55 | 11501737.60 |
