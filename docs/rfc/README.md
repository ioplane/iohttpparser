# RFC Index

Generated: 2026-03-09 22:19 UTC

This directory stores curated RFC references for `iohttpparser`.
The set is intentionally parser-centric, not a full HTTP ecosystem mirror.

## Priority Guide

- `P0`: required for parser core and default policy behavior
- `P1`: important supporting references
- `P2`: adjacent semantics kept outside parser core
- `P3`: integration context for consumers such as `iohttp`

## RFCs

| RFC | Priority | Group | Local file | Purpose |
|-----|----------|-------|------------|---------|
| [3986](https://www.rfc-editor.org/rfc/rfc3986) | P0 | Core | rfc3986.txt | Request-target structure and decomposition. |
| [9110](https://www.rfc-editor.org/rfc/rfc9110) | P0 | Core | rfc9110.txt | Field semantics and method rules. |
| [9112](https://www.rfc-editor.org/rfc/rfc9112) | P0 | Core | rfc9112.txt | Primary wire-format and framing reference. |
| [5234](https://www.rfc-editor.org/rfc/rfc5234) | P1 | Core | rfc5234.txt | Grammar notation used by HTTP RFCs. |
| [7405](https://www.rfc-editor.org/rfc/rfc7405) | P1 | Core | rfc7405.txt | ABNF extensions relevant to parser interpretation. |
| [6265](https://www.rfc-editor.org/rfc/rfc6265) | P2 | Adjacent | rfc6265.txt | Cookie semantics to keep outside parser core. |
| [6455](https://www.rfc-editor.org/rfc/rfc6455) | P2 | Adjacent | rfc6455.txt | Handshake-related behavior after HTTP upgrade. |
| [7239](https://www.rfc-editor.org/rfc/rfc7239) | P2 | Adjacent | rfc7239.txt | Proxy metadata semantics relevant to consumers. |
| [7578](https://www.rfc-editor.org/rfc/rfc7578) | P2 | Adjacent | rfc7578.txt | Body-level parsing intentionally outside core. |
| [9111](https://www.rfc-editor.org/rfc/rfc9111) | P2 | Core | rfc9111.txt | Useful boundary for cache-related headers. |
| [9113](https://www.rfc-editor.org/rfc/rfc9113) | P3 | Integration | rfc9113.txt | Version-neutral semantic layer context for iohttp. |
| [9114](https://www.rfc-editor.org/rfc/rfc9114) | P3 | Integration | rfc9114.txt | Version-neutral semantic layer context for iohttp. |

## Profiles

- `core`: RFC 9110, RFC 9112, RFC 3986, RFC 5234, RFC 7405
- `adjacent`: RFC 6265, RFC 6455, RFC 7239, RFC 7578
- `integration`: RFC 9113, RFC 9114, RFC 9111
- `all`: RFC 9110, RFC 9111, RFC 9112, RFC 3986, RFC 5234, RFC 7405, RFC 6265, RFC 6455, RFC 7239, RFC 7578, RFC 9113, RFC 9114