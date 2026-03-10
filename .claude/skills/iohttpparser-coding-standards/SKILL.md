---
name: iohttpparser-coding-standards
description: Use when writing or reviewing any C code in iohttpparser, especially public headers, parser core, SIMD backends, unit tests, and security-sensitive validation logic. Mandatory for all new and modified C files.
---

# iohttpparser Coding Standards

## File Structure

- Match header first in `.c` files.
- Use include guards, not `#pragma once`.
- Keep public declarations in `include/iohttpparser/`.
- Keep internal-only declarations in `src/`.

## Naming

- Functions: `ihtp_verb_noun()` or `ihtp_module_verb_noun()`
- Types: `ihtp_name_t`
- Macros and enum values: `IHTP_*`
- Tests: `tests/unit/test_<area>.c`

## API and Error Model

- Preserve `ihtp_status_t` semantics unless the task explicitly changes public API.
- Keep public parser outputs zero-copy and tied to caller buffer lifetime.
- Use `[[nodiscard]]` for public error-returning APIs.

## Style Rules

- Follow `.clang-format`: 4 spaces, Linux braces, 100-column limit, right-aligned pointer stars.
- Prefer small functions and narrow state transitions over large mixed-purpose blocks.
- Comments explain why, not what.

## Safety Rules

- No hidden allocation in parser hot path.
- No locale-dependent parsing helpers.
- Use checked arithmetic where input size can overflow intermediate calculations.
- Reject ambiguity explicitly; do not add silent recovery paths in security-sensitive logic.

## Test Expectations

- Add or update unit tests for each behavior change.
- Add negative tests for malformed or ambiguous framing.
- Keep SIMD changes covered by scalar-equivalence tests where possible.
