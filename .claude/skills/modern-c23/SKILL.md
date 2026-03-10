---
name: modern-c23
description: Use when writing modern C23 code for iohttpparser, especially parser, SIMD, test, and safety-critical paths. Covers required language features, checked arithmetic, and patterns to avoid in parser code.
---

# Modern C23

## Required Features

- Prefer `nullptr` over `NULL`.
- Use `[[nodiscard]]` on public error-returning functions.
- Use `_Static_assert` for ABI, enum, and limit invariants.
- Use `constexpr` for compile-time constants where it improves clarity.
- Use `<stdckdint.h>` when input-derived arithmetic can overflow.

## Parser-Specific Guidance

- Parse bytes, not locale-aware strings.
- Keep data ownership explicit.
- Keep hot-path structs compact and predictable.
- Optimize only after scalar correctness and tests are stable.

## Avoid

- `_BitInt`
- vague type inference patterns
- locale-sensitive helpers for protocol parsing
- allocation-heavy convenience wrappers in parser hot paths
- non-portable SIMD assumptions without scalar fallback

## Workflow

When writing new code:
1. Choose the narrowest data type that still fits protocol limits.
2. Guard size arithmetic with checked operations.
3. Keep public API contracts explicit and `[[nodiscard]]`.
4. Add `_Static_assert` for assumptions that must never silently drift.

## References

- `references/c23-checklist.md`
