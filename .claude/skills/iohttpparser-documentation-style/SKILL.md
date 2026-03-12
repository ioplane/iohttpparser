# iohttpparser Documentation Style

## Purpose

Use this skill when creating or editing project documentation in `README.md`,
`docs/`, `docs/plans/`, `AGENTS.md`, or contributor-facing markdown files.

## Required style

- English is authoritative. Write or revise `docs/en/*` first.
- Russian is a translation/adaptation layer. Update `docs/ru/*` only after the
  English version is stable.
- Use strict technical language.
- Write facts, interfaces, constraints, and examples.
- Do not write narrative introductions, project philosophy, or motivational
  text.
- Do not use marketing language such as "powerful", "better", "elegant",
  "modern", or "intentionally" without a measurable criterion.
- Prefer tables, lists, API contracts, limits, and state descriptions.
- Use short examples only when they clarify behavior or ownership.
- Use Mermaid for diagrams. Do not add image-only diagrams for architecture or
  flow descriptions when Mermaid is sufficient.
- Use extended GitHub Markdown when it improves scanability:
  - tables for contracts and limits
  - fenced code blocks with language tags
  - blockquote alerts such as `> [!NOTE]` only for strict warnings or
    constraints
  - `<details>` only for long secondary examples
- Every stable document must begin with badges.
- Badge links must point to official standards, official documentation, the
  official repository, or the project repository.
- Keep language consistent. Avoid mixed English/Russian sentences.
- Russian documentation must use Russian prose. Keep English only for API
  identifiers, macro names, external project names, protocol names, and RFC
  identifiers.

## Required structure

- `docs/README.md` is the stable top-level index.
- `docs/en/README.md` and `docs/ru/README.md` are language indexes.
- Numbered documents in `docs/en/` and `docs/ru/` must stay in the same order
  and cover the same topics.
- `docs/tmp/` is non-authoritative scratch space and must not be treated as the
  canonical documentation surface.

## Badges

- Use badges in every stable document.
- Badge links must point to official sources, repositories, or project sites.
- Preferred targets:
  - ISO C standard page
  - RFC Editor pages
  - Doxygen official site
  - Mermaid official site
  - the GitHub repository itself
  - official repositories for compared or integrated projects

## Mermaid diagram selection

Choose the diagram type by the document function:

- architecture description: `architecture-beta` or `flowchart`
- integration handoff: `sequenceDiagram`
- parser or decoder state transitions: `stateDiagram-v2`
- requirements and release gates: `requirementDiagram`
- classification or topology: `mindmap`, `block`, or `flowchart`
- comparison positioning: `quadrantChart` only when the axes are explicit and
  measurable

Prefer the dedicated Mermaid syntax page for each chosen diagram type instead of
linking every badge to the generic Mermaid home page.

Do not use decorative diagrams without technical value.

## Rewrite workflow

1. Read the current file and identify narrative, mixed-language text, and
   unverifiable claims.
2. Reduce the document to:
   - scope
   - contract
   - invariants
   - non-goals
   - examples
3. Replace prose paragraphs with tables, bullet lists, and short normative
   statements where possible.
4. Rewrite English first.
5. Translate/adapt Russian after the English version is complete.
6. Run `python3 scripts/lint-docs.py`.

## Repository-specific rules

- For architecture docs, define layer boundaries and ownership explicitly.
- For comparison docs, compare scope, contract, and integration cost. Do not
  use advocacy language.
- For consumer docs, separate parser, semantics, and body-decoder
  responsibilities.
- For API docs, describe ownership, lifetime, limits, return codes, and state
  transitions.
