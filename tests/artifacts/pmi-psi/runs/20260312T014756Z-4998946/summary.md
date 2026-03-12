# PSI Run Summary

Run id: `20260312T014756Z-4998946`

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
| req-small | 38695677.07 | 23759749.96 | 19635405.86 | 17987319.66 | 18004235.86 | 17164781.47 |
| req-headers | 13525043.61 | 7702701.60 | 9334685.68 | 8108611.61 | 8126390.07 | 7829076.23 |
| resp-small | 37669387.46 | 17006629.18 | 22148835.75 | 19865508.52 | 19663459.88 | 20422035.66 |
| resp-headers | 16830116.55 | 9694390.17 | 11824577.19 | 11730949.04 | 11925962.43 | 11631476.22 |
| resp-upgrade | 23766424.53 | 11994313.02 | 15261886.34 | 15362211.37 | 16180843.58 | 15416374.30 |
| req-line-only | 59880616.02 | 27851770.65 | 28269985.71 | 23669350.53 | 20835748.98 | 20442460.71 |
| req-line-hot | 38768630.75 | 20782297.64 | 22436961.39 | 19450638.12 | 17023490.37 | 16556280.43 |
| req-line-long-target | 48971080.86 | 20584547.63 | 23372901.32 | 20933267.88 | 11497022.07 | 11293312.39 |
| req-line-connect | 60112048.86 | 29829389.32 | 25109133.71 | 20969475.47 | 19161722.54 | 18703415.64 |
| req-line-options | 62480513.89 | 27378013.16 | 25193654.17 | 20275380.21 | 19741742.47 | 18913722.50 |
| req-pico-bench | 7673502.59 | 3148016.33 | 4538934.47 | 4335130.56 | 3681188.46 | 3656435.47 |
| hdr-common-heavy | 13783910.06 | 8119125.10 | 9907453.98 | 9297738.12 | 9765440.47 | 9686217.88 |
| hdr-name-heavy | 8464771.84 | 4664482.82 | 4710340.88 | 4732376.98 | 4809968.83 | 4371499.94 |
| hdr-uncommon-valid | 16811557.07 | 9155263.52 | 9198429.53 | 9159096.48 | 9312332.22 | 9189983.80 |
| hdr-value-ascii-clean | 10527019.44 | 5313068.30 | 7439946.43 | 7751511.95 | 7896432.29 | 7625855.69 |
| hdr-value-heavy | 8460092.31 | 3483633.84 | 4831159.08 | 4863405.78 | 4922976.71 | 4634628.05 |
| hdr-value-obs-text | 16286720.55 | 10000978.10 | 10855757.17 | 10249164.36 | 12011476.73 | 11298212.65 |
| hdr-value-trim-heavy | 9840654.71 | 5068585.82 | 7437782.48 | 6903250.73 | 7532594.10 | 7462750.61 |
| hdr-count-04-minimal | 20851587.16 | 13805179.65 | 14277080.73 | 13646014.47 | 14345149.31 | 13895051.92 |
| hdr-count-16-minimal | 5716355.85 | 3885734.31 | 5375512.33 | 5338605.46 | 5243347.15 | 5239327.23 |
| hdr-count-32-minimal | 2905466.83 | 1937546.78 | 2810077.47 | 2950374.35 | 2891842.95 | 2887746.45 |

## CONNECT median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-connect-median.tsv`.

| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |
|---|---:|---:|---:|---:|---:|---:|
| req-connect | 23793069.77 | 11256663.52 | 14397430.12 | 13286381.92 | 12044569.24 | 11857549.10 |
