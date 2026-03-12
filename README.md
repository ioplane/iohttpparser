[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![C23](https://img.shields.io/badge/ISO-IEC%209899%3A2024-00599C?style=for-the-badge)](https://www.iso.org/standard/82075.html)
[![RFC 9110](https://img.shields.io/badge/RFC-9110-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9110.html)
[![RFC 9112](https://img.shields.io/badge/RFC-9112-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9112.html)
[![Doxygen](https://img.shields.io/badge/Doxygen-Reference-2C4AA8?style=for-the-badge)](https://www.doxygen.nl/)

[![en](https://img.shields.io/badge/lang-en-blue.svg)](docs/en/README.md)
[![ru](https://img.shields.io/badge/lang-ru-green.svg)](docs/ru/README.md)

# iohttpparser

HTTP/1.1 wire parser for C23.

## Scope

Included:
- request line parsing
- status line parsing
- header field parsing
- parser state for incremental parsing
- framing semantics
- fixed-length body accounting
- chunked body decoding

Excluded:
- URI normalization
- routing
- cookies
- content-coding decode
- transport ownership

## Properties

| Property | Value |
|---|---|
| API model | pull-based |
| Output model | zero-copy spans |
| Parser modes | stateless and stateful |
| Policy baseline | strict |
| Scanner backends | scalar, SSE4.2, AVX2 |
| License | MIT or LGPL-2.1-or-later |

## Layer Model

| Layer | Responsibility |
|---|---|
| Scanner | delimiter search and token checks |
| Parser | request/status line and header parse |
| Semantics | framing and connection decisions |
| Body decoder | fixed-length and chunked body handling |

## Minimal Example

```c
#include <iohttpparser/iohttpparser.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *wire =
        "GET /health HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    ihtp_request_t req = {0};
    size_t consumed = 0;

    if (ihtp_parse_request(wire, strlen(wire), &req, NULL, &consumed) == IHTP_OK) {
        printf("%s %.*s HTTP/1.%d\n",
               ihtp_method_to_str(req.method),
               (int)req.path_len,
               req.path,
               req.version);
    }

    return 0;
}
```

## Build

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

Container workflow:

```bash
podman build -t iohttpparser-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iohttpparser-dev:latest
```

## Public API

| Area | API |
|---|---|
| Stateless parse | `ihtp_parse_request()`, `ihtp_parse_response()`, `ihtp_parse_headers()` |
| Stateful parse | `ihtp_parser_state_t`, `ihtp_parser_state_init()`, `ihtp_parse_*_stateful()` |
| Semantics | `ihtp_request_apply_semantics()`, `ihtp_response_apply_semantics()` |
| Body decode | `ihtp_decode_fixed()`, `ihtp_decode_chunked()` |

## Consumer Presets

| Preset | Current behavior |
|---|---|
| `IHTP_POLICY_IOHTTP` | strict RFC profile |
| `IHTP_POLICY_IOGUARD` | strict RFC profile |

## References

| Document | Purpose |
|---|---|
| [docs/README.md](docs/README.md) | documentation index |
| [docs/en/01-architecture.md](docs/en/01-architecture.md) | architecture |
| [docs/en/05-consumer-contracts.md](docs/en/05-consumer-contracts.md) | consumer contract |
| [docs/en/06-parser-state.md](docs/en/06-parser-state.md) | stateful parser |
| [docs/en/07-body-decoder.md](docs/en/07-body-decoder.md) | body decoder |
| [docs/en/08-testing-methodology.md](docs/en/08-testing-methodology.md) | testing and comparison method |
| [docs/en/10-extended-contract-methodology.md](docs/en/10-extended-contract-methodology.md) | methodology for extended-contract capabilities |
| [docs/en/11-extended-contract-results.md](docs/en/11-extended-contract-results.md) | result status for extended-contract capabilities |
| [docs/en/api-reference.md](docs/en/api-reference.md) | Doxygen entry page |

## Status Codes

| Code | Meaning |
|---|---|
| `IHTP_OK` | parse complete |
| `IHTP_INCOMPLETE` | more input required |
| `IHTP_ERROR` | malformed input |
| `IHTP_ERROR_TOO_LONG` | line length exceeded |
| `IHTP_ERROR_TOO_MANY_HEADERS` | header count exceeded |

## License

Dual-licensed:
- [MIT](LICENSE)
- [LGPL-2.1-or-later](COPYING)
