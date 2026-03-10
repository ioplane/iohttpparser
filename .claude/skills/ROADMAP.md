# iohttpparser Skills Roadmap

## Purpose

This roadmap defines the skill set that should exist for `iohttpparser` so the repository follows the same working model as `iohttp`, `liboas`, and `ringwall`: a small mandatory base layer plus specialized skills for architecture, standards, security, and domain workflows.

## Target Layout

```text
.claude/
  skills/
    iohttpparser-architecture/
    iohttpparser-coding-standards/
    modern-c23/
    http-rfc-reference/
    simd-scanner-patterns/
    parser-security-fuzzing/
    iohttp-ringwall-integration/
    benchmark-and-corpus/
```

## Phase 1: Mandatory Base Skills

### `iohttpparser-architecture`
- Mandatory for new parser modules and public API work.
- Cover layer boundaries, ownership model, policy profiles, public/internal headers, and roadmap phases.

### `iohttpparser-coding-standards`
- Mandatory for all C files.
- Cover naming, include order, header/source structure, error handling, comments, tests, and public API rules.

### `modern-c23`
- Reuse the pattern from `liboas`, but tune examples to parser, SIMD, and security-sensitive C code.
- Cover `nullptr`, `[[nodiscard]]`, checked arithmetic, constants, and anti-patterns.

### `http-rfc-reference`
- Curated RFC guide for RFC 9110, RFC 9112, RFC 3986, RFC 6455, and RFC 6265.
- Focus on parser-relevant sections, not full RFC dumps.

## Phase 2: Specialized Skills

### `simd-scanner-patterns`
- Backend structure for `scalar`, `sse42`, `avx2`, later `neon`.
- Cover dispatch, invariants, benchmarks, and fallback rules.

### `parser-security-fuzzing`
- Smuggling corpus, malformed framing cases, fuzz strategy, sanitizer matrix, and negative tests.

### `iohttp-ringwall-integration`
- Describe adapter contracts, policy differences, and how parser output maps into each consumer.

### `benchmark-and-corpus`
- Define corpus layout, benchmark inputs, regression thresholds, and differential test workflow.

## Bundled Resources To Plan

Per skill, prefer small `SKILL.md` files plus targeted `references/`:
- `references/policy-matrix.md`
- `references/rfc-priority-map.md`
- `references/simd-backends.md`
- `references/security-corpus.md`

Add scripts only where repetition justifies them:
- `scripts/run-corpus.sh`
- `scripts/run-fuzz.sh`
- `scripts/run-bench.sh`
- `scripts/diff-parsers.sh`

## Delivery Order

1. Add repository-level `CLAUDE.md`.
2. Create the 4 base skills.
3. Add architecture and security references.
4. Add specialized SIMD and fuzzing skills.
5. Add integration and benchmark skills after the public API stabilizes.
6. Iterate skills after real implementation sprints expose repeated mistakes or missing guidance.

## Acceptance Criteria

- Every base skill is concise and directly triggerable from its description.
- Skill bodies stay lean; detailed material lives in `references/`.
- Skills encode repository-specific rules rather than generic C advice.
- New contributors can implement scanner, parser, semantics, and decoder work without rediscovering architecture decisions.
