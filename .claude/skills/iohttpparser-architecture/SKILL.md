---
name: iohttpparser-architecture
description: Use when implementing or refactoring parser layers, public APIs, policy profiles, buffer ownership, or integration boundaries in iohttpparser. Mandatory for work in include/iohttpparser/, src/, and architecture-sensitive tests.
---

# iohttpparser Architecture

Read [docs/plans/2026-03-10-iohttpparser-c23-architecture-plan.md](../../../docs/plans/2026-03-10-iohttpparser-c23-architecture-plan.md) first for the repository-level plan.

## Core Model

- Keep the library split into 4 layers:
  - scanner
  - parser
  - semantics
  - body decoder
- Keep parser core transport-agnostic. `io_uring`, sockets, routing, proxy policy, and app logic are consumers, not parser internals.
- Preserve zero-copy spans into caller-owned buffers.
- Default to strict RFC 9110 / RFC 9112 behavior and expose leniency only through explicit policy profiles.

## Layer Boundaries

- Scanner:
  - delimiter search
  - token validation
  - CRLF detection
  - scalar/SSE4.2/AVX2 backend dispatch

- Parser:
  - request-line / status-line parsing
  - header field extraction
  - incremental consumed-byte reporting

- Semantics:
  - `Content-Length`
  - `Transfer-Encoding`
  - `Connection`
  - `Upgrade`
  - keep-alive / close
  - smuggling and ambiguity rejection

- Body decoder:
  - fixed-length consumption
  - chunked decoding
  - trailer handling

## Exclusions

Do not move these into parser core:
- percent-decoding
- URI normalization
- routing
- cookies
- multipart
- content-coding decoding
- WebSocket frames
- app-specific auth or proxy logic

## Consumer Profiles

- `iohttp`: broader interoperability, but strict by default
- `ringwall`: fail-closed profile with smaller limits and less leniency

## Workflow

When changing architecture-sensitive code:
1. Decide which layer owns the change.
2. Check whether the change belongs in parser core or a higher consumer layer.
3. Keep policy differences in `ihtp_policy_t` or a successor policy structure, not in ad hoc branches.
4. Update unit tests and architecture docs if boundaries shift.

## References

- `references/layer-map.md`
- `references/policy-matrix.md`
