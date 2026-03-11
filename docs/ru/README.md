# Документация На Русском

Этот каталог содержит русскоязычную документацию проекта `iohttpparser`.

Текущие основные материалы:
- [`../../README.md`](../../README.md) с обзором проекта и базовыми командами сборки
- [`../README.md`](../README.md) с общей структурой `docs/` и стабильными точками входа
- [`01-architecture.md`](01-architecture.md) с нумерованным обзором архитектуры
- [`02-comparison.md`](02-comparison.md) со сравнением против `picohttpparser` и `llhttp`
- [`03-production-hardening.md`](03-production-hardening.md) с strictness, limits и verification gates
- [`04-parser-ecosystem-comparison.md`](04-parser-ecosystem-comparison.md) со сравнением parser ecosystem и integration model
- [`05-consumer-contracts.md`](05-consumer-contracts.md) с разделением consumer contract между `iohttp` и `ioguard`
- [`06-parser-state.md`](06-parser-state.md) с описанием публичного stateful parser API
- [`07-body-decoder.md`](07-body-decoder.md) с контрактом fixed-length и chunked body decoder
- [`../plans/README.md`](../plans/README.md) с индексом планов
- [`../plans/ROADMAP.md`](../plans/ROADMAP.md) со стабильной точкой входа в roadmap
- [`../plans/BACKLOG.md`](../plans/BACKLOG.md) с отложенным техническим и процессным backlog
- [`../plans/2026-03-11-iohttpparser-final-development-plan.md`](../plans/2026-03-11-iohttpparser-final-development-plan.md) с планом доведения функционала до полноты перед интеграциями и CI
- [`../plans/2026-03-10-iohttpparser-c23-architecture-plan.md`](../plans/2026-03-10-iohttpparser-c23-architecture-plan.md) с архитектурным планом под C23
- [`../plans/2026-03-10-iohttpparser-scrum-roadmap.md`](../plans/2026-03-10-iohttpparser-scrum-roadmap.md) с текущим roadmap и спринтами
- [`../plans/2026-03-10-iohttpparser-implementation-comparison.md`](../plans/2026-03-10-iohttpparser-implementation-comparison.md) со сравнением `iohttpparser`, `picohttpparser` и `llhttp`, а также планом дальнейшей разработки и тестирования
- [`../rfc/README.md`](../rfc/README.md) с локальным зеркалом RFC для работы над парсером

По мере стабилизации проекта сюда нужно переносить постоянные русскоязычные руководства и заметки по API.
