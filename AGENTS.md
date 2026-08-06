# iohttpparser — Agent Contract

This is the canonical agent contract for the `iohttpparser` repository. Claude
Code, Codex, and Gemini CLI all read it.

## Quick Facts

- **Language**: C23 (`-std=c23`, `CMAKE_C_EXTENSIONS OFF`); C extensions disabled.
- **Project**: HTTP/1.1 parser library with zero-copy API and SIMD scanner backends.
- **License**: MIT OR LGPL-2.1-or-later.
- **Platform**: Linux-first development, portable parser core.
- **Primary use**: shared HTTP/1.1 parser for `iohttp` and `ringwall`.

## Project Structure & Module Organization

`src/` contains the parser implementation split by layer: scanner, parser,
semantics, and body decoder. Public headers live in `include/iohttpparser/`;
internal-only declarations stay in `src/ihtp_internal.h`. Unit tests are in
`tests/unit/`, fuzz targets in `tests/fuzz/`, and small integration examples in
`examples/`. Development tooling lives in `scripts/` and `deploy/podman/`.
Long-form plans belong in `docs/plans/`; `docs/tmp/` is scratch space and should
not be treated as authoritative documentation.

```text
include/iohttpparser/   # Public API headers
src/                    # Scanner, parser, semantics, body decoder
src/ihtp_internal.h     # Internal-only declarations
tests/unit/             # Unity tests
tests/fuzz/             # Fuzz targets
examples/               # Small usage examples
scripts/                # Development tooling
deploy/podman/          # Container build/run tooling
docs/plans/             # Architecture and implementation plans
docs/rfc/               # Local RFC mirror
docs/tmp/               # Local scratch area, not source of truth
.claude/skills/         # Repository-local skills and roadmap
```

## Build, Test, and Development Commands

Develop inside the container defined in `deploy/podman/Containerfile`:

```bash
podman build -t iohttpparser-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iohttpparser-dev:latest
```

Prefer doing development and quality checks inside the container.

Primary local workflow:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

Use `cmake --build --preset clang-debug --target format` to rewrite formatting
and `--target format-check` to verify it. Run `./scripts/quality.sh` before
submitting changes; it builds, tests, checks formatting, and runs static
analysis tools when installed.

## Dev Container

- Base image: `podman build -t ioplane-base:latest -f /opt/projects/repositories/container-images/tools/ioplane-base/Containerfile .`
- All `io*` projects share `localhost/ioplane-base:latest` as the common toolchain layer.
- Build: `podman build -t iohttpparser-dev:latest -f deploy/podman/Containerfile .`
- Run: `podman run --rm -it -v $(pwd):/workspace:Z iohttpparser-dev:latest`

## Profiling Toolchain

The development image also carries profiling/debug tools for parser hot-path work:

- `gdb`
- `valgrind`
- `uftrace`
- `ftracer` helpers (`frun`, `fresolve`, `/usr/local/lib/ftracer/ftracer.o`)

Use them inside the dev container after a normal debug build. Recommended order:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```

Then:

```bash
uftrace record -- ./build/clang-debug/bench/bench_throughput_compare --scenario req-pico-bench
valgrind --tool=callgrind ./build/clang-debug/bench/bench_throughput_compare --scenario req-pico-bench
gdb --args ./build/clang-debug/bench/bench_throughput_compare --scenario req-pico-bench
```

Treat `ftracer` as an advanced helper stack rather than a standalone binary: use
`frun`/`fresolve` with `/usr/local/lib/ftracer/ftracer.o` when a lightweight
trace is more useful than full `uftrace` or `callgrind`.

## Required Host Utilities

Keep these available on the host for effective agent and contributor workflows:

- `git`
- `gh` with working `gh api graphql` authentication
- `rg` (`ripgrep`)
- `jq`
- `python3`
- `podman`

Strongly recommended:

- `uv` / `uvx`
- `clangd`
- `fd`
- `yq`
- `hyperfine`
- `gdb`
- `valgrind`
- `uftrace`
- `ftracer`

## Parser Rules

- Keep the 4-layer split: scanner -> parser -> semantics -> body decoder.
- Keep parser core transport-agnostic. Do not couple it to `io_uring`, sockets, routing, or app logic.
- Default to strict RFC 9110 / RFC 9112 behavior. Leniency must be explicit through policy profiles.
- Preserve zero-copy spans and avoid heap allocation in hot paths.
- Treat `iohttp` and `ringwall` as separate consumer profiles, not as reasons to blur parser boundaries.

## Coding Style & Naming Conventions

Target C23 only; C extensions are disabled. Follow `.clang-format`: 4-space
indentation, Linux brace style, 100-column limit, and right-aligned pointer
stars (`int *ptr`). Keep public and internal symbols consistent with the
existing scheme: `ihtp_` for functions, `IHTP_` for macros and enum values, and
`_t` for typedefs. Prefer small, single-purpose translation units and keep
public headers stable. Match the existing public API style in
`include/iohttpparser/`.

- Use `nullptr`, `[[nodiscard]]`, `_Static_assert`, and checked arithmetic where size math matters.

## Testing Guidelines

Unit tests use Unity and are enabled through `IOHTTPPARSER_BUILD_TESTS`. Name new
unit tests `tests/unit/test_<area>.c` and register them in `CMakeLists.txt` with
`ihtp_add_test(...)`. Add fuzz coverage under `tests/fuzz/` when parser edge
cases are hard to encode as deterministic unit tests. Validate at least `ctest
--preset clang-debug`; sanitizer presets such as `clang-asan` are preferred for
parser or memory-safety changes.

- Add or update unit tests for every parser behavior change.
- Prefer negative tests for malformed framing, ambiguous headers, and smuggling cases.
- Use fuzzing for request-line, headers, and chunked decoding.
- Benchmark SIMD backends only after scalar correctness is stable.

## Documentation Style

Write English documentation first. Treat `docs/en/*` as authoritative and
`docs/ru/*` as the translation/adaptation layer.

Documentation must use strict technical language:

- write facts, contracts, limits, ownership rules, and examples
- do not write narrative introductions, philosophy sections, or marketing text
- keep comparisons measurable and testable
- keep non-goals explicit and tied to API or layer boundaries
- use Mermaid for diagrams
- use extended GitHub Markdown only where it improves technical readability:
  tables, fenced code blocks with language tags, short blockquote alerts for
  strict warnings, and `<details>` for secondary examples
- use badges in every stable document; badge links must point to official
  sources, official repositories, or official project sites
- Russian documentation must use Russian prose; keep English only for API
  identifiers, macro names, external project names, protocol names, and RFC
  identifiers

Mermaid diagram type must match the content:

- architecture: `architecture-beta` or `flowchart`
- interaction or handoff: `sequenceDiagram`
- state transitions: `stateDiagram-v2`
- release gates or requirements: `requirementDiagram`
- classification or topology: `mindmap`, `block`, or `flowchart`
- comparison positioning: `quadrantChart` only when axes are explicit and
  measurable
- prefer the official Mermaid syntax page for the selected diagram type when
  linking Mermaid badges

Keep the stable structure aligned with other `io*` projects:

- `docs/README.md`
- `docs/en/README.md`
- `docs/ru/README.md`
- numbered documents in `docs/en/` and `docs/ru/`
- `docs/plans/README.md`, `docs/plans/ROADMAP.md`, `docs/plans/BACKLOG.md`

`docs/tmp/` is non-authoritative scratch space.

## Changelog And Versioning

Maintain [CHANGELOG.md](CHANGELOG.md) for every merge that changes behavior,
public API, verification evidence, release automation, or published
documentation.

Rules:

- Follow [Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/).
- Keep `Unreleased` at the top.
- Add entries only under the standard sections that apply:
  `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`.
- Write short factual bullets. Do not write narrative paragraphs in the changelog.
- Record externally visible changes only. Do not list routine refactors unless they change behavior, contract, or release evidence.
- Update the changelog in the same branch as the code or documentation change. Do not defer it to a later cleanup branch.

Versioning rules:

- Use Semantic Versioning tags with a leading `v`.
- Current pre-1.0 policy:
  - `v0.y.z` for normal unstable releases
  - `v0.y.z-rc.N` for release candidates
- Do not create `1.0.0` until public API, release gate, and published evidence are all declared stable.
- For this repository, the first public release candidate line is `v0.1.0-rc.N`.

## Commit & Pull Request Guidelines

This checkout has no local commit history yet, so follow the convention already
documented in `CONTRIBUTING.md`: `feat:`, `fix:`, `refactor:`, `test:`, and
`docs:` with short imperative subjects. Pull requests should describe the
behavior change, list the commands run, and note any analyzer results or skipped
checks. Include sample inputs or output snippets when modifying parsing behavior
or public APIs.

## Skills Reference

Base skills live in `.claude/skills/`:

- `iohttpparser-architecture`
- `iohttpparser-coding-standards`
- `modern-c23`
- `http-rfc-reference`

Roadmap: `.claude/skills/ROADMAP.md`
