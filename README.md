[![en](https://img.shields.io/badge/lang-en-blue.svg)](docs/en/README.md)
[![ru](https://img.shields.io/badge/lang-ru-green.svg)](docs/ru/README.md)

# iohttpparser

<p align="center">
  <a href="docs/plans/2026-03-10-iohttpparser-c23-architecture-plan.md"><img src="https://img.shields.io/badge/C23-ISO%2FIEC%209899%3A2024-blue?style=for-the-badge" alt="C23"></a>
  <a href="docs/rfc/rfc9112.txt"><img src="https://img.shields.io/badge/RFC%209112-HTTP%2F1.1-green?style=for-the-badge" alt="RFC 9112"></a>
  <a href="docs/plans/2026-03-10-iohttpparser-c23-architecture-plan.md"><img src="https://img.shields.io/badge/SIMD-SSE4.2%20%7C%20AVX2-orange?style=for-the-badge" alt="SIMD"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT%20%7C%20LGPL--2.1-purple?style=for-the-badge" alt="License"></a>
</p>

High-performance HTTP/1.1 parser for C23 with SIMD acceleration and RFC 9112 compliance.

## Features

- **Pull-based incremental API** — no callbacks, returns status enum
- **Zero-copy** — parsed fields point into the original buffer
- **SIMD-accelerated** — runtime dispatch: scalar / SSE4.2 / AVX2
- **RFC 9112 strict by default** — lenient mode opt-in via policy profiles
- **4-layer architecture** — Scanner / Parser / Semantics / Body decoder
- **C23** — `[[nodiscard]]`, `nullptr`, `constexpr`, `<stdckdint.h>`
- **Dual licensed** — MIT or LGPL-2.1-or-later

## Quick Start

```c
#include <iohttpparser/iohttpparser.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *raw =
        "GET /api/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t s = ihtp_parse_request(raw, strlen(raw), &req, nullptr, &consumed);
    if (s == IHTP_OK) {
        printf("%s %.*s HTTP/1.%d\n",
               ihtp_method_to_str(req.method),
               (int)req.path_len, req.path,
               req.version);
    }
}
```

## Architecture

```
Layer 1: Scanner      — byte classification, SIMD-accelerated find/validate
Layer 2: Parser       — request-line, status-line, header fields
Layer 3: Semantics    — Content-Length/Transfer-Encoding, keep-alive, Host validation
Layer 4: Body decoder — chunked transfer decoding, fixed-length tracking
```

## Building

```bash
# Prerequisites: Clang 22+ or GCC 15+, CMake 3.25+
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

## Integration

### CMake (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(iohttpparser
    GIT_REPOSITORY https://github.com/ioplane/iohttpparser.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(iohttpparser)
target_link_libraries(myapp PRIVATE iohttpparser::iohttpparser)
```

### pkg-config

```bash
pkg-config --cflags --libs iohttpparser
```

## API Summary

| Function | Description |
|----------|-------------|
| `ihtp_parse_request()` | Parse HTTP request (request-line + headers) |
| `ihtp_parse_request_stateful()` | Parse HTTP request with explicit parser state |
| `ihtp_parse_response()` | Parse HTTP response (status-line + headers) |
| `ihtp_parse_response_stateful()` | Parse HTTP response with explicit parser state |
| `ihtp_parse_headers()` | Parse standalone header block |
| `ihtp_parse_headers_stateful()` | Parse standalone header block with explicit parser state |
| `ihtp_parser_state_init()` | Initialize parser state for request/response/headers mode |
| `ihtp_parser_state_reset()` | Reset parser progress while preserving parser mode |
| `ihtp_decode_chunked()` | Decode chunked transfer encoding (in-place) |
| `ihtp_decode_fixed()` | Track fixed-length body consumption |
| `ihtp_method_from_str()` | Resolve method string to enum |
| `ihtp_method_to_str()` | Method enum to string |
| `ihtp_version()` | Library version string |

## Stateful Parser API

- `ihtp_parser_state_t` exposes explicit progress for request, response, and headers-only parsing
- `state.cursor` tracks consumed bytes inside the accumulated buffer
- `state.phase` exposes `start-line`, `headers`, `done`, or `error`
- stateless `ihtp_parse_request()` / `ihtp_parse_response()` / `ihtp_parse_headers()` stay available and are layered on top of the same parser path

```c
ihtp_request_t req = {0};
ihtp_parser_state_t st;
size_t consumed = 0;

ihtp_parser_state_init(&st, IHTP_PARSER_MODE_REQUEST);

if (ihtp_parse_request_stateful(&st, wire, partial_len, &req, nullptr, &consumed) ==
    IHTP_INCOMPLETE) {
    /* grow the same accumulated buffer and call again */
}
```

See:
- [`docs/en/parser-state.md`](docs/en/parser-state.md)
- [`docs/ru/parser-state.md`](docs/ru/parser-state.md)

## Body Decoder Contracts

- `ihtp_decode_chunked()` is incremental: reuse the same decoder across calls as more bytes arrive
- `*bufsz` is always rewritten to decoded payload bytes kept in the caller buffer
- on complete chunked decode, the non-negative return value is the count of undecoded trailing bytes
- with `consume_trailer = false`, the terminal chunk trailer section stays in trailing bytes
- with `consume_trailer = true`, trailer lines are consumed through the terminating empty line before completion
- `ihtp_fixed_decoder_t` tracks only payload accounting: `remaining`, `total_decoded`, and overflow rejection

## Status Codes

| Code | Meaning |
|------|---------|
| `IHTP_OK` (0) | Parsing complete |
| `IHTP_INCOMPLETE` (-2) | Need more data |
| `IHTP_ERROR` (-1) | Malformed input |
| `IHTP_ERROR_TOO_LONG` (-3) | Exceeds size limit |
| `IHTP_ERROR_TOO_MANY_HEADERS` (-4) | Header count exceeded |

## License

Dual-licensed: [MIT](LICENSE) or [LGPL-2.1-or-later](COPYING) at your option.

SPDX-License-Identifier: `MIT OR LGPL-2.1-or-later`
