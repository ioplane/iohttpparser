# Documentation Rewrite Plan

## Objective

Rewrite the stable documentation surface in strict technical language.

## Rules

- `docs/en/*` is authoritative.
- `docs/ru/*` follows the English structure and content.
- Documents describe contracts, limits, ownership, and boundaries.
- Documents do not contain roadmap text, project philosophy, or promotional
  language.
- Entry-point documents use badges that link to official sources or repository
  pages.

## Scope

1. Update documentation rules in `AGENTS.md`.
2. Add a local documentation-style skill.
3. Extend `docs/tmp/draft/git-project-recommendations-for-c23.md` with
   repository documentation-formatting guidance.
4. Rewrite:
   - `docs/README.md`
   - `docs/en/README.md`
   - `docs/ru/README.md`
   - `docs/en/01-07`
   - `docs/ru/01-07`
5. Validate with `python3 scripts/lint-docs.py`.

## Output criteria

- English and Russian numbered docs match by topic and order.
- Russian docs do not mix long English phrases into explanatory text.
- Architecture, API, and consumer documents define ownership and non-goals.
- Comparison documents use measurable criteria.
