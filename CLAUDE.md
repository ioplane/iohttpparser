# iohttpparser - Project Instructions for Claude Code

## Quick Facts

- **Language**: C23 (`-std=c23`, `CMAKE_C_EXTENSIONS OFF`)
- **Project**: HTTP/1.1 parser library with zero-copy API and SIMD scanner backends
- **License**: MIT OR LGPL-2.1-or-later
- **Platform**: Linux-first development, portable parser core
- **Primary use**: shared HTTP/1.1 parser for `iohttp` and `ringwall`

## Build Commands

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
cmake --build --preset clang-debug --target format
cmake --build --preset clang-debug --target format-check
```

## Dev Container

- Base image: `podman build -t ioplane-base:latest -f /opt/projects/repositories/container-images/tools/ioplane-base/Containerfile .`
- All `io*` projects share `localhost/ioplane-base:latest` as the common toolchain layer.
- Build: `podman build -t iohttpparser-dev:latest -f deploy/podman/Containerfile .`
- Run: `podman run --rm -it -v $(pwd):/workspace:Z iohttpparser-dev:latest`
- Prefer doing development and quality checks inside the container.

## Key Directories

```text
include/iohttpparser/   # Public API headers
src/                    # Scanner, parser, semantics, body decoder
tests/unit/             # Unity tests
tests/fuzz/             # Fuzz targets
examples/               # Small usage examples
docs/plans/             # Architecture and implementation plans
docs/rfc/               # Local RFC mirror
docs/tmp/               # Local scratch area, not source of truth
.claude/skills/         # Repository-local skills and roadmap
```

## Parser Rules

- Keep the 4-layer split: scanner -> parser -> semantics -> body decoder.
- Keep parser core transport-agnostic. Do not couple it to `io_uring`, sockets, routing, or app logic.
- Default to strict RFC 9110 / RFC 9112 behavior. Leniency must be explicit through policy profiles.
- Preserve zero-copy spans and avoid heap allocation in hot paths.
- Treat `iohttp` and `ringwall` as separate consumer profiles, not as reasons to blur parser boundaries.

## Code Conventions

- Public symbols use `ihtp_`; macros and enum values use `IHTP_`; typedefs end with `_t`.
- Use `nullptr`, `[[nodiscard]]`, `_Static_assert`, and checked arithmetic where size math matters.
- Keep pointer style right-aligned: `int *ptr`.
- Match the existing public API style in `include/iohttpparser/`.

## Testing Rules

- Add or update unit tests for every parser behavior change.
- Prefer negative tests for malformed framing, ambiguous headers, and smuggling cases.
- Use fuzzing for request-line, headers, and chunked decoding.
- Benchmark SIMD backends only after scalar correctness is stable.

## Skills Reference

Base skills live in `.claude/skills/`:
- `iohttpparser-architecture`
- `iohttpparser-coding-standards`
- `modern-c23`
- `http-rfc-reference`

Roadmap: `.claude/skills/ROADMAP.md`
