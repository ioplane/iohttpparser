---
name: http-rfc-reference
description: Use when parser behavior depends on HTTP RFC rules for framing, request targets, headers, trailers, leniency, or security-sensitive edge cases. Mandatory when changing parsing semantics or policy defaults.
---

# HTTP RFC Reference

## Priority Order

- RFC 9112: HTTP/1.1 message syntax and framing
- RFC 9110: HTTP semantics and field behavior
- RFC 3986: request-target and URI structure
- RFC 6455: only when handshake behavior touches parser output
- RFC 6265: only when deciding what must remain outside parser core

## How To Use This Skill

When changing parser semantics:
1. Start with RFC 9112 for syntax and framing.
2. Use RFC 9110 for field meaning and semantic constraints.
3. Use RFC 3986 before changing request-target parsing assumptions.
4. Document any deliberate divergence from strict mode.

## Repository Position

- Strict mode is the default.
- Lenient mode exists only for explicit compatibility cases.
- Security-sensitive ambiguity must resolve toward rejection, not silent recovery.

## References

- `references/rfc-priority-map.md`
