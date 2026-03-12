# PSI Run Summary

Run id: `20260312T002020Z-34c2e79`

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
| iohttpparser-lenient | hdr-common-heavy | 8867233.14 | 1048.60 | 112.77 |
| iohttpparser-lenient | hdr-count-04-minimal | 13947750.33 | 571.97 | 71.70 |
| iohttpparser-lenient | hdr-count-16-minimal | 5083488.29 | 557.52 | 196.72 |
| iohttpparser-lenient | hdr-count-32-minimal | 2886262.23 | 580.79 | 346.47 |
| iohttpparser-lenient | hdr-name-heavy | 4351675.90 | 1531.38 | 229.80 |
| iohttpparser-lenient | hdr-uncommon-valid | 9174948.28 | 1688.73 | 108.99 |
| iohttpparser-lenient | hdr-value-ascii-clean | 7100138.40 | 2931.94 | 140.84 |
| iohttpparser-lenient | hdr-value-heavy | 4527793.99 | 2983.77 | 220.86 |
| iohttpparser-lenient | hdr-value-obs-text | 11443498.98 | 1822.53 | 87.39 |
| iohttpparser-lenient | hdr-value-trim-heavy | 7427718.09 | 2684.69 | 134.63 |
| iohttpparser-lenient | req-headers | 7401774.93 | 1312.95 | 135.10 |
| iohttpparser-lenient | req-line-connect | 17246089.28 | 608.54 | 57.98 |
| iohttpparser-lenient | req-line-hot | 16106676.45 | 645.14 | 62.09 |
| iohttpparser-lenient | req-line-long-target | 11436898.14 | 839.84 | 87.44 |
| iohttpparser-lenient | req-line-only | 18948356.73 | 596.33 | 52.78 |
| iohttpparser-lenient | req-line-options | 17788525.21 | 627.69 | 56.22 |
| iohttpparser-lenient | req-pico-bench | 3825385.68 | 2564.66 | 261.41 |
| iohttpparser-lenient | req-small | 17119781.63 | 800.01 | 58.41 |
| iohttpparser-lenient | resp-headers | 11299861.49 | 1250.06 | 88.50 |
| iohttpparser-lenient | resp-small | 20324717.85 | 988.54 | 49.20 |
| iohttpparser-lenient | resp-upgrade | 16245289.58 | 1192.94 | 61.56 |
| iohttpparser-stateful-lenient | hdr-common-heavy | 9732766.46 | 1150.95 | 102.75 |
| iohttpparser-stateful-lenient | hdr-count-04-minimal | 13126893.68 | 538.31 | 76.18 |
| iohttpparser-stateful-lenient | hdr-count-16-minimal | 5085300.71 | 557.72 | 196.65 |
| iohttpparser-stateful-lenient | hdr-count-32-minimal | 2874177.45 | 578.36 | 347.93 |
| iohttpparser-stateful-lenient | hdr-name-heavy | 4526464.89 | 1592.89 | 220.92 |
| iohttpparser-stateful-lenient | hdr-uncommon-valid | 9430391.62 | 1735.75 | 106.04 |
| iohttpparser-stateful-lenient | hdr-value-ascii-clean | 8092129.54 | 3341.57 | 123.58 |
| iohttpparser-stateful-lenient | hdr-value-heavy | 4916217.57 | 3239.73 | 203.41 |
| iohttpparser-stateful-lenient | hdr-value-obs-text | 12314673.40 | 1961.28 | 81.20 |
| iohttpparser-stateful-lenient | hdr-value-trim-heavy | 7344979.70 | 2654.79 | 136.15 |
| iohttpparser-stateful-lenient | req-headers | 8111568.79 | 1438.86 | 123.28 |
| iohttpparser-stateful-lenient | req-line-connect | 19686083.74 | 694.64 | 50.80 |
| iohttpparser-stateful-lenient | req-line-hot | 17409766.27 | 697.34 | 57.44 |
| iohttpparser-stateful-lenient | req-line-long-target | 11619293.84 | 853.24 | 86.06 |
| iohttpparser-stateful-lenient | req-line-only | 20443403.10 | 643.38 | 48.92 |
| iohttpparser-stateful-lenient | req-line-options | 19964413.43 | 704.46 | 50.09 |
| iohttpparser-stateful-lenient | req-pico-bench | 3915122.95 | 2624.83 | 255.42 |
| iohttpparser-stateful-lenient | req-small | 17440113.92 | 814.98 | 57.34 |
| iohttpparser-stateful-lenient | resp-headers | 11605741.36 | 1283.90 | 86.16 |
| iohttpparser-stateful-lenient | resp-small | 22532200.20 | 1095.91 | 44.38 |
| iohttpparser-stateful-lenient | resp-upgrade | 16251689.26 | 1193.41 | 61.53 |
| iohttpparser-stateful-strict | hdr-common-heavy | 9894837.18 | 1170.12 | 101.06 |
| iohttpparser-stateful-strict | hdr-count-04-minimal | 12926471.77 | 530.09 | 77.36 |
| iohttpparser-stateful-strict | hdr-count-16-minimal | 5361213.50 | 587.98 | 186.52 |
| iohttpparser-stateful-strict | hdr-count-32-minimal | 2979328.75 | 599.52 | 335.65 |
| iohttpparser-stateful-strict | hdr-name-heavy | 4583042.79 | 1612.80 | 218.20 |
| iohttpparser-stateful-strict | hdr-uncommon-valid | 9070545.72 | 1669.52 | 110.25 |
| iohttpparser-stateful-strict | hdr-value-ascii-clean | 7604285.46 | 3140.12 | 131.50 |
| iohttpparser-stateful-strict | hdr-value-heavy | 4952785.71 | 3263.83 | 201.91 |
| iohttpparser-stateful-strict | hdr-value-obs-text | 11269832.93 | 1794.87 | 88.73 |
| iohttpparser-stateful-strict | hdr-value-trim-heavy | 7469757.47 | 2699.89 | 133.87 |
| iohttpparser-stateful-strict | req-headers | 9372128.76 | 1662.46 | 106.70 |
| iohttpparser-stateful-strict | req-line-connect | 23884218.86 | 842.78 | 41.87 |
| iohttpparser-stateful-strict | req-line-hot | 21589981.21 | 864.77 | 46.32 |
| iohttpparser-stateful-strict | req-line-long-target | 20922852.63 | 1536.43 | 47.79 |
| iohttpparser-stateful-strict | req-line-only | 26730559.83 | 841.24 | 37.41 |
| iohttpparser-stateful-strict | req-line-options | 25874864.76 | 913.02 | 38.65 |
| iohttpparser-stateful-strict | req-pico-bench | 4633170.04 | 3106.23 | 215.83 |
| iohttpparser-stateful-strict | req-small | 19706928.41 | 920.91 | 50.74 |
| iohttpparser-stateful-strict | resp-headers | 11701973.67 | 1294.55 | 85.46 |
| iohttpparser-stateful-strict | resp-small | 21979480.84 | 1069.02 | 45.50 |
| iohttpparser-stateful-strict | resp-upgrade | 15577834.46 | 1143.93 | 64.19 |
| iohttpparser-strict | hdr-common-heavy | 9385290.24 | 1109.86 | 106.55 |
| iohttpparser-strict | hdr-count-04-minimal | 13282237.45 | 544.68 | 75.29 |
| iohttpparser-strict | hdr-count-16-minimal | 5206416.59 | 571.00 | 192.07 |
| iohttpparser-strict | hdr-count-32-minimal | 2747153.54 | 552.80 | 364.01 |
| iohttpparser-strict | hdr-name-heavy | 4503155.69 | 1584.69 | 222.07 |
| iohttpparser-strict | hdr-uncommon-valid | 8893434.82 | 1636.92 | 112.44 |
| iohttpparser-strict | hdr-value-ascii-clean | 7547642.61 | 3116.73 | 132.49 |
| iohttpparser-strict | hdr-value-heavy | 4845294.59 | 3193.00 | 206.39 |
| iohttpparser-strict | hdr-value-obs-text | 11310837.89 | 1801.40 | 88.41 |
| iohttpparser-strict | hdr-value-trim-heavy | 7054685.91 | 2549.86 | 141.75 |
| iohttpparser-strict | req-headers | 8636101.00 | 1531.90 | 115.79 |
| iohttpparser-strict | req-line-connect | 20906027.51 | 737.69 | 47.83 |
| iohttpparser-strict | req-line-hot | 18937433.84 | 758.53 | 52.81 |
| iohttpparser-strict | req-line-long-target | 20249863.06 | 1487.01 | 49.38 |
| iohttpparser-strict | req-line-only | 25006038.96 | 786.97 | 39.99 |
| iohttpparser-strict | req-line-options | 23206451.67 | 818.86 | 43.09 |
| iohttpparser-strict | req-pico-bench | 4421302.17 | 2964.19 | 226.18 |
| iohttpparser-strict | req-small | 18781182.04 | 877.65 | 53.24 |
| iohttpparser-strict | resp-headers | 11191997.54 | 1238.13 | 89.35 |
| iohttpparser-strict | resp-small | 21161443.72 | 1029.24 | 47.26 |
| iohttpparser-strict | resp-upgrade | 15378145.92 | 1129.26 | 65.03 |
| llhttp | hdr-common-heavy | 7996413.45 | 945.62 | 125.06 |
| llhttp | hdr-count-04-minimal | 12820214.50 | 525.73 | 78.00 |
| llhttp | hdr-count-16-minimal | 3736284.80 | 409.77 | 267.65 |
| llhttp | hdr-count-32-minimal | 1855202.61 | 373.31 | 539.02 |
| llhttp | hdr-name-heavy | 4505330.79 | 1585.45 | 221.96 |
| llhttp | hdr-uncommon-valid | 8749303.28 | 1610.39 | 114.29 |
| llhttp | hdr-value-ascii-clean | 5252000.30 | 2168.77 | 190.40 |
| llhttp | hdr-value-heavy | 3542059.08 | 2334.18 | 282.32 |
| llhttp | hdr-value-obs-text | 9608784.12 | 1530.33 | 104.07 |
| llhttp | hdr-value-trim-heavy | 5025531.71 | 1816.44 | 198.98 |
| llhttp | req-headers | 7718200.47 | 1369.08 | 129.56 |
| llhttp | req-line-connect | 28420907.39 | 1002.86 | 35.19 |
| llhttp | req-line-hot | 20766931.07 | 831.81 | 48.15 |
| llhttp | req-line-long-target | 20057012.06 | 1472.85 | 49.86 |
| llhttp | req-line-only | 28274789.67 | 889.84 | 35.37 |
| llhttp | req-line-options | 26951896.79 | 951.02 | 37.10 |
| llhttp | req-pico-bench | 3084443.52 | 2067.91 | 324.21 |
| llhttp | req-small | 23086261.47 | 1078.82 | 43.32 |
| llhttp | resp-headers | 9294448.64 | 1028.21 | 107.59 |
| llhttp | resp-small | 17268608.63 | 839.90 | 57.91 |
| llhttp | resp-upgrade | 13783454.08 | 1012.16 | 72.55 |
| picohttpparser | hdr-common-heavy | 14038378.12 | 1660.12 | 71.23 |
| picohttpparser | hdr-count-04-minimal | 20052742.72 | 822.32 | 49.87 |
| picohttpparser | hdr-count-16-minimal | 5687158.13 | 623.73 | 175.83 |
| picohttpparser | hdr-count-32-minimal | 2896115.72 | 582.77 | 345.29 |
| picohttpparser | hdr-name-heavy | 9148983.51 | 3219.58 | 109.30 |
| picohttpparser | hdr-uncommon-valid | 16455555.60 | 3028.80 | 60.77 |
| picohttpparser | hdr-value-ascii-clean | 11114067.45 | 4589.45 | 89.98 |
| picohttpparser | hdr-value-heavy | 8376253.26 | 5519.86 | 119.39 |
| picohttpparser | hdr-value-obs-text | 16849628.36 | 2683.53 | 59.35 |
| picohttpparser | hdr-value-trim-heavy | 9373145.14 | 3387.85 | 106.69 |
| picohttpparser | req-headers | 13528817.53 | 2399.79 | 73.92 |
| picohttpparser | req-line-connect | 53243387.50 | 1878.74 | 18.78 |
| picohttpparser | req-line-hot | 39499501.81 | 1582.13 | 25.32 |
| picohttpparser | req-line-long-target | 47184623.29 | 3464.90 | 21.19 |
| picohttpparser | req-line-only | 61841236.23 | 1946.22 | 16.17 |
| picohttpparser | req-line-options | 61946738.19 | 2185.85 | 16.14 |
| picohttpparser | req-pico-bench | 7464712.07 | 5004.59 | 133.96 |
| picohttpparser | req-small | 39021946.14 | 1823.50 | 25.63 |
| picohttpparser | resp-headers | 16637665.83 | 1840.56 | 60.10 |
| picohttpparser | resp-small | 38948287.19 | 1894.34 | 25.68 |
| picohttpparser | resp-upgrade | 26679319.78 | 1959.14 | 37.48 |

## CONNECT median matrix

| Parser | Scenario | req/s median | MiB/s median | ns/req median |
|---|---|---:|---:|---:|
| iohttpparser-lenient | req-connect | 11631219.17 | 1098.15 | 85.98 |
| iohttpparser-stateful-lenient | req-connect | 12329750.80 | 1164.10 | 81.10 |
| iohttpparser-stateful-strict | req-connect | 14084369.17 | 1329.76 | 71.00 |
| iohttpparser-strict | req-connect | 13322225.26 | 1257.80 | 75.06 |
| llhttp | req-connect | 11396907.56 | 1076.02 | 87.74 |
| picohttpparser | req-connect | 24087731.37 | 2274.21 | 41.51 |
