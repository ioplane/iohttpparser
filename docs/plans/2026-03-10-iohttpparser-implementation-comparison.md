# Сравнение `iohttpparser`, `picohttpparser` и `llhttp`

## Цель

Сравнить текущую реализацию `iohttpparser` с `picohttpparser` и `llhttp`, затем на основании различий зафиксировать практический план дальнейшей разработки и тестирования.

## Что проанализировано

Локальная кодовая база:
- `README.md`
- `include/iohttpparser/ihtp_parser.h`
- `include/iohttpparser/ihtp_body.h`
- `include/iohttpparser/ihtp_types.h`
- `src/ihtp_semantics.c`
- `src/ihtp_scanner_sse42.c`
- `tests/unit/test_scanner_backends.c`
- `tests/fuzz/fuzz_scanner.c`
- `bench/bench_parser.c`
- `CMakePresets.json`
- `scripts/quality.sh`

Внешние референсы:
- `picohttpparser`: <https://github.com/h2o/picohttpparser/tree/master>
- `llhttp`: <https://github.com/nodejs/llhttp>

## Текущее состояние `iohttpparser`

Сейчас библиотека уже не выглядит как “черновой парсер”. В репозитории есть:
- 4-слойная модель `scanner -> parser -> semantics -> body decoder`
- pull-based zero-copy API без callback-ов
- strict-by-default policy с ограниченной lenient-настройкой
- body framing для `Content-Length` и `chunked`
- semantics-проверки для `Content-Length`, `Transfer-Encoding`, `Connection`, `Host`, keep-alive и no-body response cases
- SIMD backends `scalar`, `sse42`, `avx2`
- unit tests, corpus-driven tests, fuzz targets, benchmark harness
- quality pipeline: `clang-format`, `cppcheck`, `PVS-Studio`, `CodeChecker`

Главный архитектурный плюс текущей базы: security-sensitive semantics вынесена из синтаксического parser core. Главный текущий зазор: у parser layer пока нет отдельного публичного state object для по-настоящему потокового parsing request/status line и headers; инкрементальность сейчас держится на accumulated buffer + `IHTP_INCOMPLETE`.

## Сравнение реализаций

| Критерий | `iohttpparser` | `picohttpparser` | `llhttp` |
|---|---|---|---|
| Архитектура | Явно разнесённые `scanner/parser/semantics/body` | Минималистичный parser + chunked decoder | Большая state machine с интегрированными parser/framing decisions |
| Модель API | Pull-based, zero-copy, без callback-ов | Pull-based, zero-copy, caller-owned header array | Event/callback-driven API через `llhttp_execute()` |
| Инкрементальность | Повторный вызов на накопленном буфере, body decoder имеет state | Stateless parse с `last_len` для growing buffer | Полноценный parser state, pause/resume, upgrade handling |
| Аллокации | Нет скрытых аллокаций в hot path | Нет скрытых аллокаций | Parser state и settings явные, callbacks у embedder |
| Security semantics | Явный слой для `TE/CL`, duplicate `Content-Length`, `Host`, keep-alive | Почти отсутствует, ответственность на consumer | Много правил внутри parser machine + множество lenient toggles |
| Политики strict/lenient | Короткий, контролируемый policy surface | Почти нет policy surface | Очень широкий набор leniency flags |
| SIMD/perf модель | Собственный scanner dispatch `scalar/sse42/avx2` | Известен своей компактностью и скоростью, без отдельной multi-backend архитектуры | Сильный perf focus, но не через отдельный scanner layer |
| Проверяемость | Unit + corpus + fuzz + analyzers + bench | Простой test/bench, хороший baseline для differential tests | Сильная история verification за счёт `llparse` и explicit graph |
| Подходящесть для `iohttp`/`ringwall` | Наиболее близок к целевой архитектуре этих проектов | Хорош как syntax/perf reference, слаб как security boundary | Богат по покрытию, но хуже подходит как primary pull-based core |

## Ключевые выводы

### Относительно `picohttpparser`

`picohttpparser` полезен как эталон минимализма:
- маленький surface area
- zero-copy
- stateless parsing
- удобный differential baseline для request/response/header parsing

Но для целей `iohttp` и особенно `ringwall` он слишком “низкий уровень”:
- нет отдельного semantics layer
- нет явной модели policy
- smuggling-sensitive decisions остаются на стороне потребителя

Практический вывод: использовать `picohttpparser` как референс по простоте syntax-path и как differential oracle там, где semantics сопоставима. Не копировать его архитектуру один в один.

### Относительно `llhttp`

`llhttp` сильнее нас в трёх вещах:
- полноценный streaming parser state
- очень широкое покрытие edge cases и operational modes
- более зрелая verification story за счёт генератора и explicit state graph

Но `llhttp` не стоит брать как прямой архитектурный шаблон:
- callback/event-driven модель хуже подходит для `iohttp` и `ringwall`
- parser machine у него сильнее смешивает syntax, framing и integration decisions
- широкий набор leniency flags для security-sensitive library опасен как baseline

Практический вывод: использовать `llhttp` как референс полноты, corpus и behavioral coverage, но не как образец public API.

### Относительно нашей реализации

`iohttpparser` уже сейчас лучше обоих конкурентов выровнен под целевые интеграции:
- лучше `picohttpparser` по security semantics и policy
- лучше `llhttp` по соответствию pull-based zero-copy integration model

Но до production-grade зрелости ещё не хватает:
- явного parser state для streaming headers/status/request parsing
- differential test harness против `picohttpparser` и `llhttp`
- более полной матрицы RFC edge cases вокруг `Upgrade`, `CONNECT`, `Expect`, trailers и legacy tolerance boundaries
- benchmark gates не только для scanner, но и для parser/semantics/body целиком

## План дальнейшей разработки

### 1. Завершить parser-state модель

Добавить явный state object для layer 2:
- request-line/status-line/header parsing без повторного разбора всего накопленного буфера
- возможность feed/resume без callback API
- отдельный контракт для partial header blocks

Это главное отставание от `llhttp` и главный следующий архитектурный шаг.

### 2. Стабилизировать границу `parser` и `semantics`

Нельзя тащить логику `llhttp` внутрь syntax parser. Нужно оставить:
- parser только для wire syntax
- semantics для framing/security decisions

Новые задачи этого блока:
- доработать `Upgrade`
- завершить `CONNECT` / tunnel semantics
- явно закрыть `Expect: 100-continue`
- проверить trailers как отдельный semantics surface

### 3. Принять решение по SIMD token-path

Сейчас `sse42` безопасно ускоряет `find_char`, но `is_token` остаётся scalar-backed. Нужно выбрать одно из двух:
- либо доказуемо эквивалентный SIMD `is_token`
- либо зафиксировать scalar fallback как сознательное production-решение

Это должно решаться не “по скорости на глаз”, а только после corpus + fuzz + benchmark доказательной базы.

### 4. Добавить differential harness

Нужен отдельный runner, который сравнивает `iohttpparser` с:
- `picohttpparser` для syntax-only cases
- `llhttp` для parser/framing behavior там, где режимы сопоставимы

Важно: differential tests не должны заставлять нас копировать чужую lenient behavior. Источник истины для итогового поведения всё равно RFC + наш strict policy.

### 5. Довести интеграционный слой для потребителей

Нужно зафиксировать adapter contracts для:
- `iohttp`
- `ringwall`

Это особенно важно для:
- limits/profile mapping
- keep-alive/body-mode decisions
- upgrade/tunnel handoff
- zero-copy lifetime guarantees

## План дальнейшего тестирования

### 1. Матрица тестов по слоям

Сохранить и расширить раздельные наборы:
- `scanner`: equivalence, corpus, fuzz, bench
- `parser`: syntax corpus, incomplete-buffer cases, explicit state transitions
- `semantics`: smuggling, framing ambiguity, policy matrix
- `body decoder`: chunked/fixed corpora, trailing-byte contracts, trailers

### 2. Differential testing

Добавить отдельные suites:
- `iohttpparser` vs `picohttpparser`
- `iohttpparser` vs `llhttp`

Категории:
- request-line parsing
- status-line parsing
- header extraction
- chunked framing
- incomplete input behavior

Для cases, где поведение намеренно различается, differential suite должна помечать это как expected divergence.

### 3. Sanitizer и analyzer matrix

Текущий pipeline уже силён, но его нужно сделать обязательной матрицей:
- `clang-debug`
- `gcc-debug`
- `clang-asan`
- `clang-msan`
- `clang-tsan`
- `clang-fuzz`
- `quality.sh`

Отдельно стоит добавить short smoke-runs для fuzz targets в регулярный CI.

### 4. Security corpus

Нужен расширенный негативный корпус по классам атак:
- `TE + CL`
- conflicting `Content-Length`
- malformed `Transfer-Encoding`
- obs-fold / bare `LF`
- whitespace before colon / illegal header names
- no-body response precedence
- trailer abuse
- request smuggling boundary cases

### 5. Performance regression gates

Нужны не только “bench для ручного запуска”, но и воспроизводимые пороги:
- scanner scalar vs dispatch
- parser throughput на short/medium/large messages
- semantics overhead
- chunked decoder throughput

Важный принцип: скорость считается бонусом только после доказанной эквивалентности scalar truth.

## Итоговая рекомендация

Развивать `iohttpparser` не как “ещё один быстрый парсер”, а как строгий, проверяемый и хорошо интегрируемый HTTP/1.1 core для `iohttp` и `ringwall`.

Заимствовать у `picohttpparser` нужно простоту и удобство differential testing. Заимствовать у `llhttp` нужно полноту corpus, дисциплину streaming state и verification mindset. Архитектурно же стоит сохранить собственный курс: pull-based API, явные layer boundaries и strict-by-default semantics.
