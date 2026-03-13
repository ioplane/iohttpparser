[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![picohttpparser](https://img.shields.io/badge/GitHub-picohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/h2o/picohttpparser)
[![llhttp](https://img.shields.io/badge/GitHub-llhttp-181717?style=for-the-badge&logo=github)](https://github.com/nodejs/llhttp)
[![Valgrind](https://img.shields.io/badge/Valgrind-callgrind-b22222?style=for-the-badge)](https://valgrind.org/)
[![SVG](https://img.shields.io/badge/SVG-Графики-f97316?style=for-the-badge)](https://www.w3.org/Graphics/SVG/)

# Результаты Испытаний

## Связанные Документы

| Документ | Назначение |
|---|---|
| [02-comparison.md](./02-comparison.md) | сравниваемые возможности и область применения |
| [08-testing-methodology.md](./08-testing-methodology.md) | программа и методика испытаний |
| [10-extended-contract-methodology.md](./10-extended-contract-methodology.md) | методика для возможностей расширенного контракта |
| [11-extended-contract-results.md](./11-extended-contract-results.md) | состояние результатов по возможностям вне общей матрицы |
| [../plans/2026-03-11-sprint-11-comparison-report.md](../plans/2026-03-11-sprint-11-comparison-report.md) | подробные заметки по сравнению и профилированию |

## Область

Документ хранит публикуемые в репозитории результаты ПСИ по:
- функциональной проверке
- сравнению пропускной способности парсеров
- прикладным сценариям потребителей

Документ публикует общую сравнительную матрицу.

Возможности, которым нужен расширенный взгляд на контракт, описаны в:
- [10-extended-contract-methodology.md](./10-extended-contract-methodology.md)
- [11-extended-contract-results.md](./11-extended-contract-results.md)

## Набор Артефактов

Текущий каталог артефактов:

`tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/`

Точки входа на уровне репозитория:
- [`tests/artifacts/pmi-psi/README.md`](../../tests/artifacts/pmi-psi/README.md)
- [`tests/artifacts/pmi-psi/index.tsv`](../../tests/artifacts/pmi-psi/index.tsv)
- [`tests/artifacts/pmi-psi/latest.txt`](../../tests/artifacts/pmi-psi/latest.txt)
- [`tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/summary.md`](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/summary.md)
- [`tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/throughput-median.tsv`](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/throughput-median.tsv)
- [`tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/throughput-connect-median.tsv`](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/throughput-connect-median.tsv)
- [`tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/summary-extended.md`](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/summary-extended.md)
- [`tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/scanner-bench.tsv`](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/scanner-bench.tsv)

```mermaid
flowchart LR
  A["Функциональные и производительные прогоны"]
  B["Артефакты TSV"]
  C["Графики SVG"]
  D["summary.md"]
  E["09-test-results.md"]

  A --> B
  B --> C
  B --> D
  C --> E
  D --> E
```

## Сводка Выполнения

| Поле | Значение |
|---|---|
| идентификатор прогона | `20260313T210231Z-3b9c398` |
| ревизия git | `3b9c398` |
| функциональный preset | `clang-debug` |
| число итераций в стенде пропускной способности | `200000` |
| число прогонов для медианы | `5` |
| статус | `PASS` |

## Функциональные Результаты

Результат `ctest --preset clang-debug --output-on-failure`:

| Показатель | Значение |
|---|---|
| всего тестов | `12` |
| ошибок | `0` |
| доля успешных тестов | `100%` |
| общее время | `0.02 sec` |

Проверенный набор исполняемых файлов:
- `test_scanner`
- `test_scanner_backends`
- `test_scanner_corpus`
- `test_parser`
- `test_parser_state`
- `test_differential_corpus`
- `test_semantics_differential`
- `test_semantics`
- `test_semantics_corpus`
- `test_iohttp_integration`
- `test_body_decoder`
- `test_body_decoder_corpus`

## Профили Сравнения

| Профиль | Значение |
|---|---|
| `picohttpparser` | минимальный нулевой разбор без расширенного контракта |
| `llhttp` | эталонное сгенерированное ядро разбора |
| `iohttpparser-stateful-strict` | предпочтительный производительный путь для потребителей |
| `iohttpparser-strict` | строгая оболочка без отдельного состояния |
| `iohttpparser-stateful-lenient` | интерфейс состояния в режиме совместимости |
| `iohttpparser-lenient` | оболочка без состояния в режиме совместимости |

## Прикладные Сценарии

### Расшифровка Сценариев

| Сценарий | Назначение |
|---|---|
| `req-small` | короткий запрос с минимальным блоком заголовков |
| `req-headers` | запрос с более крупным и реалистичным набором заголовков |
| `resp-small` | короткий ответ без большого блока заголовков |
| `resp-headers` | ответ с более крупным блоком заголовков |
| `resp-upgrade` | передача ответа `101 Switching Protocols` потребителю |
| `req-connect` | запрос `CONNECT` в форме authority |

### Общая Трёхсторонняя Матрица

![Прикладные сценарии, медиана req/s](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/charts/common-three-way.svg)

### Трёхсторонний Фокус На CONNECT

![CONNECT, медиана req/s](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/charts/connect-three-way.svg)

### req-small

Короткий запрос с минимальным блоком заголовков.

| Парсер | медиана req/s | медиана MiB/s | медиана ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `40,385,707.74` | `1,887.23` | `24.76` |
| `llhttp` | `22,907,400.38` | `1,070.46` | `43.65` |
| `iohttpparser-stateful-strict` | `18,356,247.59` | `857.79` | `54.48` |
| `iohttpparser-lenient` | `17,002,573.76` | `794.53` | `58.81` |
| `iohttpparser-stateful-lenient` | `16,721,689.39` | `781.41` | `59.80` |
| `iohttpparser-strict` | `16,147,832.76` | `754.59` | `61.93` |

### req-headers

Запрос с более крупным и реалистичным набором заголовков.

| Парсер | медиана req/s | медиана MiB/s | медиана ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `13,729,089.91` | `2,435.31` | `72.84` |
| `iohttpparser-stateful-strict` | `9,119,757.79` | `1,617.69` | `109.65` |
| `iohttpparser-stateful-lenient` | `8,250,082.78` | `1,463.43` | `121.21` |
| `iohttpparser-strict` | `8,181,233.63` | `1,451.22` | `122.23` |
| `iohttpparser-lenient` | `7,990,219.65` | `1,417.33` | `125.15` |
| `llhttp` | `7,872,263.39` | `1,396.41` | `127.03` |

### resp-small

Короткий ответ без большого блока заголовков.

| Парсер | медиана req/s | медиана MiB/s | медиана ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `38,847,518.44` | `1,889.44` | `25.74` |
| `iohttpparser-stateful-lenient` | `21,923,305.26` | `1,066.29` | `45.61` |
| `iohttpparser-stateful-strict` | `21,488,479.01` | `1,045.14` | `46.54` |
| `iohttpparser-strict` | `21,222,363.57` | `1,032.20` | `47.12` |
| `iohttpparser-lenient` | `20,950,136.89` | `1,018.96` | `47.73` |
| `llhttp` | `17,837,704.54` | `867.58` | `56.06` |

### resp-headers

Ответ с более крупным блоком заголовков.

| Парсер | медиана req/s | медиана MiB/s | медиана ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `17,362,885.08` | `1,920.79` | `57.59` |
| `iohttpparser-stateful-lenient` | `12,080,476.75` | `1,336.42` | `82.78` |
| `iohttpparser-lenient` | `11,825,557.42` | `1,308.22` | `84.56` |
| `iohttpparser-stateful-strict` | `11,277,604.26` | `1,247.60` | `88.67` |
| `iohttpparser-strict` | `11,022,673.14` | `1,219.40` | `90.72` |
| `llhttp` | `9,553,977.80` | `1,056.92` | `104.67` |

### resp-upgrade

Передача ответа `101 Switching Protocols` потребителю.

| Парсер | медиана req/s | медиана MiB/s | медиана ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `28,136,039.44` | `2,066.11` | `35.54` |
| `iohttpparser-stateful-lenient` | `16,685,821.99` | `1,225.29` | `59.93` |
| `iohttpparser-lenient` | `15,634,622.62` | `1,148.10` | `63.96` |
| `iohttpparser-stateful-strict` | `15,592,956.47` | `1,145.04` | `64.13` |
| `iohttpparser-strict` | `15,271,738.44` | `1,121.45` | `65.48` |
| `llhttp` | `14,175,027.57` | `1,040.91` | `70.55` |

### req-connect

Запрос `CONNECT` в форме authority.

| Парсер | медиана req/s | медиана MiB/s | медиана ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `23,549,243.00` | `2,223.37` | `42.46` |
| `iohttpparser-stateful-strict` | `14,317,185.50` | `1,351.74` | `69.85` |
| `iohttpparser-strict` | `13,411,627.73` | `1,266.24` | `74.56` |
| `iohttpparser-stateful-lenient` | `12,248,678.55` | `1,156.44` | `81.64` |
| `iohttpparser-lenient` | `11,501,737.60` | `1,085.92` | `86.94` |
| `llhttp` | `11,360,359.29` | `1,072.57` | `88.03` |

## Вспомогательные Сценарии Профилирования

Эти сценарии не являются прикладными историями потребителя. Они нужны для
локализации стоимости отдельных участков парсера.

| Сценарий | Назначение |
|---|---|
| `req-line-only` | стоимость разбора стартовой строки без большого блока заголовков |
| `req-line-hot` | типовой короткий путь запроса |
| `req-line-long-target` | стоимость проверки длинного пути запроса |
| `req-line-connect` | путь метода и формы authority для `CONNECT` |
| `req-line-options` | путь метода для `OPTIONS *` |
| `req-pico-bench` | длинный запрос из `picohttpparser/bench.c` |
| `hdr-common-heavy` | набор частых заголовков |
| `hdr-name-heavy` | стоимость классификации имён заголовков |
| `hdr-uncommon-valid` | редкие, но корректные имена заголовков |
| `hdr-value-ascii-clean` | путь значения с чистыми ASCII-байтами |
| `hdr-value-heavy` | длинный реалистичный путь значений |
| `hdr-value-obs-text` | путь значений с байтами `obs-text` |
| `hdr-value-trim-heavy` | путь обрезки внешних пробельных байтов |
| `hdr-count-04-minimal` | постоянная стоимость цикла для четырёх минимальных заголовков |
| `hdr-count-16-minimal` | постоянная стоимость цикла для шестнадцати минимальных заголовков |
| `hdr-count-32-minimal` | постоянная стоимость цикла для тридцати двух минимальных заголовков |

Полная числовая матрица опубликована в:
- [`throughput-median.tsv`](../../tests/artifacts/pmi-psi/runs/20260313T210231Z-3b9c398/throughput-median.tsv)
- [2026-03-11-sprint-11-comparison-report.md](../plans/2026-03-11-sprint-11-comparison-report.md)

## Интерпретация

- Функциональные ПСИ завершились без ошибок.
- Текущий прогон уже содержит слитые оптимизации горячего пути из PR `#25`.
- Для этого же прогона в `11` опубликованы расширенные результаты по контракту и вариантам сканера.
- `picohttpparser` остаётся лидером по чистой пропускной способности во всех опубликованных сценариях.
- `iohttpparser-stateful-strict` теперь является правильной производительной базой для потребителей.
- `iohttpparser-stateful-strict` быстрее `llhttp` в сценариях:
  - `req-headers`
  - `resp-small`
  - `resp-headers`
  - `resp-upgrade`
  - `req-connect`
- `llhttp` остаётся быстрее на самом коротком пути запроса `req-small`.
- Оболочки без состояния остаются медленнее интерфейса состояния, потому что по контракту очищают выходную структуру перед каждым вызовом.
