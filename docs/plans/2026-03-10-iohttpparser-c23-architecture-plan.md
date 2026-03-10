# iohttpparser C23 Architecture Plan

## Goal

Build `iohttpparser` as a strict, incremental, zero-allocation HTTP/1.1 wire parser for C23. The library should be reusable by both `iohttp` and `ringwall`, while keeping HTTP semantics above the parser and transport/runtime concerns outside the parser core.

## Design Principles

- Parse bytes, not strings or Unicode text.
- Keep the hot path allocation-free and zero-copy.
- Separate wire syntax, message semantics, and body framing.
- Default to RFC 9110 / RFC 9112 strictness; expose leniency only through explicit policy profiles.
- Keep the core transport-agnostic so it works with `io_uring`, plain sockets, or test harnesses.

## Scope

Included in parser core:
- request-line and status-line parsing
- headers and trailers
- `Content-Length`, `Transfer-Encoding`, `chunked`, `Connection`, `Upgrade`, `Expect`
- keep-alive / close decision
- incremental parsing and consumed-byte reporting

Excluded from parser core:
- percent-decoding and URI normalization
- routing, cookies, multipart, compression decoding
- WebSocket frame parsing
- app-specific auth or proxy logic

## Layered Architecture

### Layer 1: Scanner
- Provide `scalar`, `sse42`, and `avx2` backends.
- Optimize delimiter search, token validation, and CRLF detection.
- Keep one common contract so higher layers do not depend on SIMD flavor.

### Layer 2: Parser
- Parse request/status line and header fields into zero-copy spans.
- Maintain explicit parser state for partial reads.
- Return `status`, `consumed`, and parsed views without callbacks as the primary API.

### Layer 3: Semantics
- Validate framing and reject ambiguity.
- Enforce: conflicting `Content-Length`, invalid header names, illegal whitespace, `TE + CL` ambiguity, invalid chunk-size, and smuggling patterns.
- Apply policy profiles:
  - `general` for `iohttp`
  - `strict-proxy` for `ringwall`

### Layer 4: Body Decoder
- Track fixed-length bodies.
- Decode chunked bodies incrementally.
- Expose trailers and end-of-message state separately from header parsing.

## Public API Shape

- Keep a pull-based API:
  - `ihtp_parse_request(...)`
  - `ihtp_parse_response(...)`
  - `ihtp_parse_headers(...)`
  - `ihtp_decode_chunked(...)`
  - `ihtp_decode_fixed(...)`
- Use parser-owned state plus caller-owned input buffers.
- Return spans into the original buffer; never hide copies inside the library.

## Integration Contracts

### iohttp
- Use `iohttpparser` as the HTTP/1.1 codec below a version-neutral request model.
- Map parser output into the unified `io_request_t` abstraction.
- Enable controlled leniency only where interoperability requires it.

### ringwall
- Use `iohttpparser` behind a stricter policy adapter.
- Prefer smaller limits, no legacy compatibility mode, and fail-closed behavior.
- Treat parser output as a security boundary for proxy and control-plane traffic.

## C23 Requirements

- Use `nullptr`, `[[nodiscard]]`, `constexpr`, `_Static_assert`, and `<stdckdint.h>`.
- Keep public error returns checked and explicit.
- Avoid locale-dependent helpers and non-portable C extensions.

## Validation Strategy

- Unit tests per layer.
- Fuzz targets for request-line, headers, and chunked framing.
- Negative corpus for request smuggling and malformed framing.
- Differential checks against `picohttpparser` and `llhttp` where semantics are comparable.
- Benchmarks for `scalar`, `sse42`, and `avx2` on representative request sizes.

## Implementation Phases

1. Freeze RFC-derived policy matrix and limits.
2. Finish strict scalar parser and semantics path.
3. Complete chunked and fixed-length body decoder behavior.
4. Build negative corpus, fuzzing, and sanitizer coverage.
5. Stabilize public API and adapter contracts for `iohttp` and `ringwall`.
6. Optimize scanner backends with SSE4.2 and AVX2.
7. Add integration benchmarks and regression gates.
