# PSI Run Summary

Run id: `20260312T002621Z-34c2e79`

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
| req-small | 39358038.90 | 22889567.56 | 20141999.08 | 19123076.89 | 17933819.54 | 16054814.99 |
| req-headers | 13228205.26 | 7460704.47 | 9060302.98 | 8956119.89 | 8055212.03 | 7724660.95 |
| resp-small | 37967472.13 | 18525103.51 | 21599111.84 | 21862006.79 | 22206618.37 | 21841803.53 |
| resp-headers | 17709099.53 | 9566233.95 | 11956838.92 | 12397916.33 | 11864417.24 | 11921626.75 |
| resp-upgrade | 28050353.19 | 13800504.33 | 15683676.55 | 15993263.64 | 15472213.10 | 16060830.72 |
| req-line-only | 60950727.74 | 27521999.02 | 28199167.33 | 25807886.86 | 20059206.75 | 17810674.49 |
| req-line-hot | 39247499.84 | 21557840.71 | 22983116.60 | 21186584.32 | 17451484.65 | 15248111.94 |
| req-line-long-target | 45358056.50 | 19993514.10 | 23747573.74 | 21529113.93 | 11707660.65 | 10892331.97 |
| req-line-connect | 55697413.77 | 29466442.29 | 24990956.40 | 23159433.01 | 19705178.99 | 18113110.21 |
| req-line-options | 57650245.19 | 26677718.36 | 25898895.63 | 23309496.27 | 21042701.64 | 18890712.79 |
| req-pico-bench | 6963894.02 | 3210905.42 | 4440002.27 | 4312771.13 | 3798434.91 | 3659272.95 |
| hdr-common-heavy | 14058550.21 | 7428723.44 | 10177030.98 | 9562949.77 | 10016985.80 | 9719435.85 |
| hdr-name-heavy | 9202400.78 | 4493858.01 | 4644697.37 | 4595589.50 | 4671939.56 | 4518440.33 |
| hdr-uncommon-valid | 16378410.73 | 9113597.39 | 9111688.71 | 8835407.97 | 9093552.83 | 9097334.70 |
| hdr-value-ascii-clean | 11162388.64 | 5294126.02 | 7806550.86 | 7542810.92 | 8015089.21 | 7574829.66 |
| hdr-value-heavy | 8523755.81 | 3557400.85 | 4659582.87 | 4821170.26 | 4912893.90 | 4906303.46 |
| hdr-value-obs-text | 16573002.96 | 9979826.78 | 10981249.90 | 10957293.24 | 12293506.74 | 11410230.30 |
| hdr-value-trim-heavy | 9944828.08 | 5026035.11 | 7094183.19 | 7080439.53 | 7210994.86 | 7215372.15 |
| hdr-count-04-minimal | 20546487.25 | 10687805.15 | 15008754.61 | 13447625.60 | 14075722.60 | 13364262.69 |
| hdr-count-16-minimal | 5493789.34 | 3738322.30 | 5440991.84 | 5275256.00 | 4458996.73 | 5145542.97 |
| hdr-count-32-minimal | 2907935.94 | 1929737.26 | 2902727.96 | 2933831.49 | 2882587.58 | 2873855.60 |

## CONNECT median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-connect-median.tsv`.

| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |
|---|---:|---:|---:|---:|---:|---:|
| req-connect | 23477372.10 | 10724707.64 | 13961958.41 | 12950026.37 | 11375010.08 | 11370703.00 |
