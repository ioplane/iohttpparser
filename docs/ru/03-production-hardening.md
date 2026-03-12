[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![RFC 9110](https://img.shields.io/badge/RFC-9110-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9110.html)
[![RFC 9112](https://img.shields.io/badge/RFC-9112-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9112.html)
[![Mermaid](https://img.shields.io/badge/Mermaid-Требования-ff3670?style=for-the-badge)](https://mermaid.js.org/syntax/requirementDiagram.html)

# Усиление Для Работы В Рабочей Среде

## Базовый Режим

`iohttpparser` — чувствительный к ошибкам безопасности парсер HTTP/1.1 на
уровне байтового протокола. Для рабочей среды нужен режим отказа по умолчанию
на ошибочном и неоднозначном вводе.

## Политика

Базовый профиль для рабочей среды: `IHTP_POLICY_STRICT`.

| Поле | Значение По Умолчанию | Эффект |
|---|---|---|
| `reject_obs_fold` | `true` | отклонять устаревший перенос заголовка |
| `reject_bare_lf` | `true` | отклонять окончания строк без `CRLF` |
| `reject_te_cl` | `true` | отклонять сочетание `Transfer-Encoding` и `Content-Length` |
| `allow_spaces_in_uri` | `false` | отклонять цель запроса с пробелами |

## Лимиты

| Лимит | Макрос | Значение По Умолчанию |
|---|---|---|
| число заголовков | `IHTP_MAX_HEADERS` | 64 |
| длина строки запроса | `IHTP_MAX_REQUEST_LINE` | 8192 |
| длина строки заголовка | `IHTP_MAX_HEADER_LINE` | 8192 |

Эти лимиты входят в публичный контракт.

## Классы Отклонения

Текущие классы жёсткого отклонения:
- bare `LF`
- obsolete folded headers в строгом режиме
- конфликтующий повторяющийся `Content-Length`
- ошибочный `Transfer-Encoding`
- повторяющийся `chunked`
- цепочка `Transfer-Encoding` в запросе, не заканчивающаяся `chunked`
- ошибочный список токенов `Connection`
- недопустимые управляющие байты в цели запроса
- отсутствующий или повторяющийся `Host` в строгом режиме HTTP/1.1

```mermaid
requirementDiagram
    functionalRequirement строгая_политика {
        id: hardening-1
        text: строгий профиль отклоняет неверный и неоднозначный HTTP-ввод
        risk: high
        verifymethod: test
    }

    functionalRequirement явные_лимиты {
        id: hardening-2
        text: лимиты парсера заданы явно и покрыты тестами
        risk: medium
        verifymethod: test
    }

    functionalRequirement владение {
        id: hardening-3
        text: парсер сохраняет владение буфером на стороне потребителя и не выполняет скрытых выделений памяти в горячем пути
        risk: high
        verifymethod: inspection
    }

    element выпуск {
        type: verification
    }

    выпуск - satisfies -> строгая_политика
    выпуск - satisfies -> явные_лимиты
    выпуск - satisfies -> владение
```

## Правила Владения

- Входные байты принадлежат потребителю.
- Парсер не выделяет скрытые входные буферы.
- Разобранные диапазоны валидны только пока жив буфер потребителя.
- Состояние декодера хранит только счётчики и состояние фрейминга.

## Поверхность Проверки

| Слой | Инструменты |
|---|---|
| модульные тесты | Unity |
| корпусные тесты | корпуса для парсера, семантики и тела |
| дифференциальные тесты | `picohttpparser`, `llhttp` |
| фаззинг | парсер, сканер, декодер `chunked` |
| статический анализ | `cppcheck`, `PVS-Studio`, `CodeChecker` |
| форматирование | `clang-format` |

## Профили Потребителей

| Потребитель | Ожидаемый профиль |
|---|---|
| `iohttp` | строгий режим по умолчанию; ослабление только явно |
| `ioguard` | строгий режим, меньшие лимиты, отказ по умолчанию на неоднозначности |

## Условия Выпуска

Обязательные условия перед выпуском рабочей версии:

1. `./scripts/quality.sh` проходит.
2. Дифференциальный корпус зелёный.
3. Короткие фаззинг-прогоны зелёные.
4. Контракты для `iohttp` и `ioguard` документированы.
