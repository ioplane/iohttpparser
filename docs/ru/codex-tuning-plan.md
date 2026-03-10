# План Тюнинга Codex

## Назначение

Этот документ превращает официальные рекомендации по Codex в переиспользуемый шаблон тюнинга для проектов и сразу применяет его к `iohttpparser`.

Основные источники:
- документация OpenAI Codex: <https://developers.openai.com/codex/>
- конфигурация и profiles
- `AGENTS.md`
- skills
- MCP servers
- delegating tasks
- OpenAI Cookbook Codex Prompting Guide: <https://cookbook.openai.com/>

## Шаблон Для Проекта

### 1. Иерархия Инструкций

- Корневой `AGENTS.md` должен быть коротким, конкретным и операционным.
- Вложенные `AGENTS.md` добавляются только там, где поведение поддерева действительно отличается.
- Постоянные правила команды хранятся в `AGENTS.md`; узкие повторяемые workflow выносятся в skills.
- Правила должны дополнять друг друга, а не конфликтовать.

### 2. Skills

- Skills стоит создавать только для повторяемых, дорогих по вниманию или рискованных workflow.
- У каждого skill должна быть одна задача, явные триггеры, references и по возможности готовые scripts/assets.
- Skills предпочтительнее длинных prompt-фрагментов.
- Локальные skills репозитория должны быть важнее внешних generic-инструментов.

### 3. MCP И Расширения

- MCP-серверы стоит добавлять только тогда, когда они дают стабильную пользу сверх встроенных возможностей Codex.
- Для локальных инструментов предпочтителен `stdio` MCP, а список серверов должен оставаться минимальным.
- Базовые полезные категории:
  - доступ к официальной документации
  - интеграция с source control / issue tracker
  - семантическая навигация по коду
- Экспериментальные MCP-серверы не должны быть частью критического пути разработки.

### 4. Profiles И Конфигурация

- Нужны отдельные profiles для:
  - интерактивной разработки
  - review / investigation
  - automation / non-interactive запусков
- Для review, parser/security задач и design-работ reasoning нужно повышать.
- Для интерактивной работы approvals должны быть консервативными, а automation должна жить в полностью изолированной среде.
- Project-scoped конфигурацию имеет смысл добавлять только если проекту действительно нужно поведение, отличное от глобального default.

### 4a. Обязательные Локальные Утилиты

- Обязательный baseline:
  - `git`
  - `gh` с рабочим `gh api graphql`
  - `rg`
  - `jq`
  - `python3`
  - `podman`
- Настоятельно рекомендуется:
  - `uv` / `uvx`
  - `clangd`
- Полезные дополнения:
  - `fd`
  - `yq`
  - `hyperfine`

Цель простая: Codex не должен тратить токены на компенсацию слабого локального инструментария.

### 5. Параллелизм И Subagents

- Делегирование и параллельная работа нужны только для явно разделимых задач.
- У каждой параллельной задачи должен быть отдельный branch или worktree.
- Одна задача должна вести к одному результату: review, docs, differential testing, benchmark analysis и т.д.
- Не стоит использовать subagents для тесно связанных правок с общим локальным состоянием.

### 6. Automation

- Для повторяемых non-interactive workflow лучше использовать `codex exec`.
- Для automation обязательны:
  - изолированный runner/container
  - явный prompt file
  - machine-readable output, если результат читает другая система
  - жёстко ограниченные filesystem/network permissions
- Первые хорошие цели для automation:
  - обновление docs
  - review summaries
  - issue triage
  - генерация отчётов

### 7. Контроль Качества

- Codex должен использовать уже существующие project checks, а не придумывать новые.
- Лучше иметь небольшой набор стабильных gate:
  - build
  - tests
  - formatting
  - static analysis
  - sanitizer/fuzz smoke
- Точные команды должны быть явно зафиксированы в docs и `AGENTS.md`.

## Применение К `iohttpparser`

### Текущее Состояние

- В проекте уже есть сильные корневые инструкции: [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [CODEX.md](../../CODEX.md).
- Локальные skills уже являются правильным механизмом для архитектуры и coding standards.
- У проекта уже есть чистый container-based workflow для build и quality.
- `context7` полезен и уже настроен глобально.
- `Serena` сейчас уместна только как опциональный semantic helper; считать её обязательной инфраструктурой пока не стоит.

### Рекомендуемая Целевая Конфигурация

#### 1. Сохранять `AGENTS.md` как источник истины

- Политики репозитория должны оставаться в `AGENTS.md`.
- Нельзя переносить ключевые инженерные правила в Serena memories, MCP prompts или случайные custom prompts.
- Вложенные `AGENTS.md` нужны только если `tests/`, `deploy/` или будущим consumer adapters понадобится отдельное поведение.

#### 2. Оставить skills основным механизмом расширения

- Продолжать использовать `.claude/skills/` для:
  - architecture
  - coding standards
  - RFC/security guidance
  - будущих integration playbooks для `iohttp` и `ringwall`
- Если знание живёт внутри репозитория, сначала нужен skill, а не новый MCP-сервер.

#### 3. Рекомендуемые Codex profiles

- `default-dev`
  - интерактивная разработка
  - workspace write
  - обычные approvals
  - medium/high reasoning в зависимости от parser/security задачи
- `review`
  - high reasoning
  - read-heavy workflow
  - фокус на review, diff analysis и разборе failures
- `automation`
  - `codex exec`
  - изолированный container или CI runner
  - без зависимости от интерактивной памяти
  - с явным output contract

#### 3a. Обязательный набор утилит для этого репозитория

- Обязательно:
  - `git`
  - `gh` с аутентифицированным GraphQL-доступом
  - `rg`
  - `jq`
  - `python3`
  - `podman`
- Настоятельно рекомендуется:
  - `uv` / `uvx`
  - локальный `clangd`
- Опционально:
  - `fd`
  - `yq`
  - `hyperfine`

Для этого репозитория сильный CLI-набор полезнее, чем дополнительные GUI-плагины.

#### 4. Рекомендуемый набор MCP

- Оставить:
  - `context7` для доступа к официальной документации
- Опционально:
  - `serena` как semantic navigation слой поверх `clangd`
- Не добавлять новые MCP-серверы без доказанного повторяющегося bottleneck.
- Для GitHub сейчас достаточно shell + `gh`; отдельный MCP-сервер опционален.

#### 5. Политика Serena для этого проекта

- Для Serena использовать `clangd`, а не `ccls`.
- Использовать Serena только как read-mostly semantic layer.
- Отключить Serena-side editing и memories для этого репозитория.
- Не делать Serena частью критического workflow, пока её health-check path на этом проекте не стабилизирован.

#### 6. Ближайшие шаги по тюнингу

1. Добавить project `.codex/config.toml` только если `iohttpparser` действительно требует настроек, которые не должны влиять на другие репозитории.
2. Добавить project custom prompt, который напоминает Codex:
   - следовать `AGENTS.md`
   - использовать `.claude/skills/`
   - предпочитать container execution для build/test/quality
3. Оставить тяжёлые build/test/fuzz задачи внутри Podman, а semantic/LSP tooling держать локально.
4. Добавить один automation prompt для:
   - differential test batch
   - docs/report refresh
   - review summary generation

#### 7. Среднесрочные шаги

1. Добавить project-scoped Codex profiles, когда отличие workflow станет реальным, а не теоретическим.
2. Автоматизировать recurring review/report задачи раньше, чем добавлять новые MCP-серверы.
3. Добавить consumer-specific skills для `iohttp` и `ringwall`.
4. Переоценить Serena после стабилизации её health-check path на этом репозитории.

## Рекомендуемый Baseline Для `iohttpparser`

- Основным агентом остаётся Codex.
- `AGENTS.md` и локальные skills остаются источником истины.
- Podman остаётся execution environment для build/test/quality.
- `context7` остаётся основным внешним источником документации.
- Serena остаётся опциональной и не-блокирующей.

Это даёт простую и устойчивую модель: Codex отвечает за выполнение и правки, skills за локальные process rules, контейнерные команды за воспроизводимость, а MCP используется только там, где он реально повышает signal.
