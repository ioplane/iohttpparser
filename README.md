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
| `ihtp_request_apply_semantics()` | Resolve request framing and keep-alive decisions |
| `ihtp_response_apply_semantics()` | Resolve response framing and keep-alive decisions |
| `ihtp_parser_state_init()` | Initialize parser state for request/response/headers mode |
| `ihtp_parser_state_reset()` | Reset parser progress while preserving parser mode |
| `ihtp_decode_chunked()` | Decode chunked transfer encoding (in-place) |
| `ihtp_decode_fixed()` | Track fixed-length body consumption |
| `ihtp_method_from_str()` | Resolve method string to enum |
| `ihtp_method_to_str()` | Method enum to string |
| `ihtp_version()` | Library version string |

## Stateful Parser API

- all parsed spans in request, response, and header structs point into the
  caller-owned input buffer
- for stateful parsing, keep reusing the same accumulated buffer while new
  bytes are appended
- `ihtp_parser_state_reset()` rewinds parser progress for a new message but does
  not clear caller-owned output structs for you
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
- generated API reference via `cmake --build --preset clang-debug --target docs`

## Consumer Policy Presets

- `IHTP_POLICY_IOHTTP` is the named strict-by-default preset for `iohttp`
- `IHTP_POLICY_IOGUARD` is the named fail-closed preset for `ioguard`
- both currently map to the strict RFC profile and exist so consumer code can
  depend on explicit intent rather than anonymous `IHTP_POLICY_STRICT`
- the current alias contract is exact across the public policy surface:
  - `reject_obs_fold`
  - `reject_bare_lf`
  - `reject_te_cl`
  - `allow_spaces_in_uri`

## Body Decoder Contracts

- `ihtp_decode_chunked()` is incremental: reuse the same decoder across calls as more bytes arrive
- `*bufsz` is always rewritten to decoded payload bytes kept in the caller buffer
- on complete chunked decode, the non-negative return value is the count of undecoded trailing bytes
- with `consume_trailer = false`, the terminal chunk trailer section stays in trailing bytes
- with `consume_trailer = true`, trailer lines are consumed through the terminating empty line before completion
- `ihtp_fixed_decoder_t` tracks only payload accounting: `remaining`, `total_decoded`, and overflow rejection
- trailing bytes remain caller-owned in the same buffer immediately after the
  decoded payload prefix

## Semantics Contracts

- `ihtp_request_apply_semantics()` now exposes:
  - `req.protocol_upgrade`
  - `req.expects_continue`
  - `req.has_trailer_fields`
- `ihtp_response_apply_semantics()` now exposes:
  - `resp.protocol_upgrade`
  - `resp.has_trailer_fields`
- trailer advertisement is accepted only for chunked messages; strict mode rejects `Trailer` on non-chunked framing

## Integration Example

`examples/basic_parse.c` now demonstrates:
- incremental request parsing with `ihtp_parser_state_t`
- consumer-oriented policy selection via `IHTP_POLICY_IOHTTP`
- semantics handoff for `Expect` and trailer ownership
- chunked body decoding after header completion

`examples/connect_tunnel.c` demonstrates:
- stateful parsing of a `CONNECT` request in authority-form
- consumer-oriented policy selection via `IHTP_POLICY_IOGUARD`
- tunnel handoff using `req.method == IHTP_METHOD_CONNECT`
- the fact that `CONNECT` stays a method-driven integration decision, not a
  separate parser boolean

## Additional Examples

`examples/response_upgrade.c` demonstrates:
- stateful parsing of a `101 Switching Protocols` response
- response-side `protocol_upgrade` handoff
- the point where upgraded bytes become consumer-owned rather than HTTP-owned

`examples/expect_trailers.c` demonstrates:
- `Expect: 100-continue` handoff
- chunked request-body decoding with `consume_trailer = false`
- the fact that trailer bytes can remain consumer-owned after chunked decode

At the moment `IHTP_POLICY_IOHTTP` and `IHTP_POLICY_IOGUARD` are intentional
named aliases of the strict profile. They exist to stabilize consumer-facing
configuration names before a narrower policy split is justified and test-covered.

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
