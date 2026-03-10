# Repository Guidelines

## Project Structure & Module Organization
`src/` contains the parser implementation split by layer: scanner, parser, semantics, and body decoder. Public headers live in `include/iohttpparser/`; internal-only declarations stay in `src/ihtp_internal.h`. Unit tests are in `tests/unit/`, fuzz targets in `tests/fuzz/`, and small integration examples in `examples/`. Development tooling lives in `scripts/` and `deploy/podman/`. Long-form plans belong in `docs/plans/`; `docs/tmp/` is scratch space and should not be treated as authoritative documentation.

## Build, Test, and Development Commands
Develop inside the container defined in `deploy/podman/Containerfile`:
```bash
podman build -t iohttpparser-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iohttpparser-dev:latest
```
Primary local workflow:
```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```
Use `cmake --build --preset clang-debug --target format` to rewrite formatting and `--target format-check` to verify it. Run `./scripts/quality.sh` before submitting changes; it builds, tests, checks formatting, and runs static analysis tools when installed.

## Coding Style & Naming Conventions
Target C23 only; C extensions are disabled. Follow `.clang-format`: 4-space indentation, Linux brace style, 100-column limit, and right-aligned pointer stars (`int *ptr`). Keep public and internal symbols consistent with the existing scheme: `ihtp_` for functions, `IHTP_` for macros and enum values, and `_t` for typedefs. Prefer small, single-purpose translation units and keep public headers stable.

## Testing Guidelines
Unit tests use Unity and are enabled through `IOHTTPPARSER_BUILD_TESTS`. Name new unit tests `tests/unit/test_<area>.c` and register them in `CMakeLists.txt` with `ihtp_add_test(...)`. Add fuzz coverage under `tests/fuzz/` when parser edge cases are hard to encode as deterministic unit tests. Validate at least `ctest --preset clang-debug`; sanitizer presets such as `clang-asan` are preferred for parser or memory-safety changes.

## Commit & Pull Request Guidelines
This checkout has no local commit history yet, so follow the convention already documented in `CONTRIBUTING.md`: `feat:`, `fix:`, `refactor:`, `test:`, and `docs:` with short imperative subjects. Pull requests should describe the behavior change, list the commands run, and note any analyzer results or skipped checks. Include sample inputs or output snippets when modifying parsing behavior or public APIs.
