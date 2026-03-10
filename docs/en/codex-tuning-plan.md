# Codex Tuning Plan

## Purpose

This document turns the official Codex guidance into a reusable per-project tuning template and applies it to `iohttpparser`.

Primary sources:
- OpenAI Codex docs: <https://developers.openai.com/codex/>
- Configuration and profiles
- `AGENTS.md` guidance
- Skills
- MCP servers
- Delegating tasks
- OpenAI Cookbook Codex Prompting Guide: <https://cookbook.openai.com/>

## Per-Project Template

### 1. Instruction Hierarchy

- Keep the root `AGENTS.md` short, concrete, and operational.
- Add nested `AGENTS.md` files only where subtree behavior truly differs.
- Put stable team rules in `AGENTS.md`; put narrow repeatable workflows in skills.
- Keep rules additive and non-contradictory.

### 2. Skills

- Create skills only for workflows that are repeated, high-friction, or easy to do wrong.
- Each skill should have one job, explicit triggers, references, and reusable scripts/assets where possible.
- Prefer skills over long prompt fragments.
- Keep repository-specific skills authoritative over generic external tools.

### 3. MCP and Extensions

- Add MCP servers only when they provide durable value that Codex does not already provide.
- Prefer `stdio` MCP for local tools and keep the server list minimal.
- Recommended default categories:
  - official documentation lookup
  - source-control / issue-tracker integration
  - semantic code navigation
- Treat experimental MCP servers as optional, not critical-path infrastructure.

### 4. Profiles and Config

- Define separate profiles for:
  - interactive development
  - review / investigation
  - automation / non-interactive runs
- Increase reasoning for review, parser/security work, and design tasks.
- Keep approvals conservative for interactive work and fully isolated for automation.
- Prefer project-scoped configuration only when the project needs behavior that differs from the user-global default.

### 4a. Required Local Utilities

- Mandatory baseline:
  - `git`
  - `gh` with authenticated `gh api graphql`
  - `rg`
  - `jq`
  - `python3`
  - `podman`
- Strongly recommended:
  - `uv` / `uvx`
  - `clangd`
- Useful extras:
  - `fd`
  - `yq`
  - `hyperfine`

The goal is simple: Codex should not waste tokens compensating for weak local tooling.

### 5. Parallelism and Subagents

- Use delegated/parallel work only for clearly separable tasks.
- Give each parallel task an isolated branch or worktree.
- Keep one task focused on one outcome: review, docs, differential testing, benchmark analysis, etc.
- Avoid subagents for tightly coupled edits that need shared local state.

### 6. Automation

- Use `codex exec` for repeatable non-interactive workflows.
- In automation, require:
  - isolated runner/container
  - explicit prompt file
  - machine-readable output when another system consumes the result
  - tightly scoped filesystem/network permissions
- Good first automation targets are docs refresh, review summaries, issue triage, and report generation.

### 7. Quality Controls

- Make Codex consume existing project checks rather than inventing new ones.
- Prefer a small number of stable gates:
  - build
  - tests
  - formatting
  - static analysis
  - sanitizer/fuzz smoke
- Put the exact commands in repo docs and `AGENTS.md`.

## Application to `iohttpparser`

### Current State

- Strong root instructions already exist in [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), and [CODEX.md](../../CODEX.md).
- Repository-local skills are already the right mechanism for architecture and coding standards.
- The project already has a clean container-based build and quality workflow.
- `context7` is useful and already configured globally.
- `Serena` is useful only as an optional semantic helper; it should not be treated as required infrastructure yet.

### Recommended Target Setup

#### 1. Keep `AGENTS.md` authoritative

- Keep repo policy in `AGENTS.md`.
- Do not move core engineering policy into Serena memories, MCP prompts, or ad-hoc custom prompts.
- Add nested `AGENTS.md` only if `tests/`, `deploy/`, or future consumer adapters need different behavior.

#### 2. Keep skills as the primary extension mechanism

- Continue using `.claude/skills/` for:
  - architecture
  - coding standards
  - RFC/security guidance
  - future integration playbooks for `iohttp` and `ringwall`
- Add skills before adding new MCP servers when the knowledge is local to this repository.

#### 3. Recommended Codex profiles

- `default-dev`
  - interactive coding
  - workspace write
  - normal approvals
  - medium/high reasoning depending on parser/security task
- `review`
  - high reasoning
  - read-heavy workflow
  - optimized for review, diff analysis, and failure investigation
- `automation`
  - `codex exec`
  - isolated container or CI runner
  - no reliance on interactive memory
  - explicit output contract

#### 3a. Required utility baseline for this repository

- Mandatory:
  - `git`
  - `gh` with authenticated GraphQL access
  - `rg`
  - `jq`
  - `python3`
  - `podman`
- Strongly recommended:
  - `uv` / `uvx`
  - local `clangd`
- Optional:
  - `fd`
  - `yq`
  - `hyperfine`

This repository benefits more from strong CLI tooling than from additional GUI plugins.

#### 4. Recommended MCP set

- Keep:
  - `context7` for official docs lookup
- Optional:
  - `serena` as semantic navigation over `clangd`
- Do not add more MCP servers unless they remove a repeated bottleneck.
- For GitHub, shell + `gh` is currently sufficient; a dedicated MCP server is optional, not required.

#### 5. Serena policy for this project

- Use `clangd`, not `ccls`, for Serena.
- Use Serena as a read-mostly semantic layer only.
- Disable Serena-side editing and memories for this repository.
- Do not depend on Serena for critical workflow until its current symbol-health issues are resolved.

#### 6. Immediate tuning actions

1. Add a project `.codex/config.toml` only if `iohttpparser` needs settings that should not affect other repositories.
2. Add a project custom prompt that reminds Codex to:
   - follow `AGENTS.md`
   - use `.claude/skills/`
   - prefer container execution for build/test/quality
3. Keep heavy build/test/fuzz tasks inside Podman; keep semantic/LSP tooling local.
4. Add one automation prompt for:
   - differential test batch
   - docs/report refresh
   - review summary generation

#### 7. Medium-term actions

1. Add project-scoped Codex profiles when workflow divergence becomes real.
2. Add automation around recurring review/report tasks before adding more MCP servers.
3. Add consumer-specific skills for `iohttp` and `ringwall`.
4. Re-evaluate Serena only after its health-check path is stable on this repository.

## Recommended Baseline for `iohttpparser`

- Codex remains the primary agent.
- `AGENTS.md` and local skills remain the source of truth.
- Podman remains the execution environment for build/test/quality.
- `context7` remains the primary external documentation integration.
- Serena remains optional and non-blocking.

This gives the project a simple model: Codex for execution and edits, skills for local process knowledge, containerized commands for reproducibility, and MCP only where it clearly improves signal.
