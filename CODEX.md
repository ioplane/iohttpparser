# iohttpparser - Project Instructions for Codex

## Start Here

Read these in order before making non-trivial changes:
- `AGENTS.md`
- `CLAUDE.md`
- `docs/plans/2026-03-10-iohttpparser-c23-architecture-plan.md`
- `.claude/skills/ROADMAP.md`

## Core Working Rules

- Preserve the current public API shape in `include/iohttpparser/` unless the task explicitly requires an API change.
- Keep parser responsibilities narrow: wire parsing, framing, and semantics checks only.
- Do not move routing, URI normalization, cookies, compression, or transport code into parser core.
- Default to strict behavior. Any lenient behavior must be explicit, documented, and covered by tests.
- Prefer scalar correctness first, then optimize scanner backends with SSE4.2 and AVX2.
- Keep zero-copy outputs and avoid hidden allocations in hot paths.

## Code and Review Focus

- Public names: `ihtp_*`
- Status model: `ihtp_status_t`
- Policy model: `ihtp_policy_t`
- Layer ownership: scanner -> parser -> semantics -> body decoder
- Security focus: `Content-Length`, `Transfer-Encoding`, chunked framing, header validation, smuggling defense

## Required Follow-Through

- Update unit tests when behavior changes.
- Update docs when layer boundaries, policy defaults, or public API semantics change.
- Keep `CLAUDE.md`, `CODEX.md`, and `.claude/skills/` aligned when repository rules change.

## Local Skills

Repository-local skills live under `.claude/skills/`. They are the closest thing to project memory for repeated architecture and standards decisions, even when the active agent is not Claude.
