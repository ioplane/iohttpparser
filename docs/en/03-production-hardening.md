[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![RFC 9110](https://img.shields.io/badge/RFC-9110-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9110.html)
[![RFC 9112](https://img.shields.io/badge/RFC-9112-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9112.html)
[![Mermaid](https://img.shields.io/badge/Mermaid-Requirements-ff3670?style=for-the-badge)](https://mermaid.js.org/syntax/requirementDiagram.html)

# Production Hardening

## Baseline

`iohttpparser` is a security-sensitive HTTP/1.1 wire parser. Production use
requires fail-closed behavior for malformed and ambiguous input.

## Policy Surface

The production baseline is `IHTP_POLICY_STRICT`.

| Field | Default | Effect |
|---|---|---|
| `reject_obs_fold` | `true` | reject obsolete folded header syntax |
| `reject_bare_lf` | `true` | reject line endings without `CRLF` |
| `reject_te_cl` | `true` | reject `Transfer-Encoding` plus `Content-Length` ambiguity |
| `allow_spaces_in_uri` | `false` | reject request targets with spaces |

## Limits

| Limit | Macro | Default |
|---|---|---|
| header count | `IHTP_MAX_HEADERS` | 64 |
| request line bytes | `IHTP_MAX_REQUEST_LINE` | 8192 |
| header line bytes | `IHTP_MAX_HEADER_LINE` | 8192 |

These limits are part of the public contract.

## Rejection Classes

Current hard rejection classes include:
- bare `LF`
- obsolete folded headers in strict mode
- conflicting duplicate `Content-Length`
- malformed `Transfer-Encoding`
- duplicate `chunked`
- request `Transfer-Encoding` chains not ending in `chunked`
- malformed `Connection` token lists
- invalid request-target control bytes
- missing or duplicate `Host` in strict HTTP/1.1 request handling

```mermaid
requirementDiagram
    functionalRequirement strict_policy {
        id: hardening-1
        text: strict profile rejects malformed and ambiguous HTTP input
        risk: high
        verifymethod: test
    }

    functionalRequirement explicit_limits {
        id: hardening-2
        text: parser limits are explicit and test-covered
        risk: medium
        verifymethod: test
    }

    functionalRequirement ownership {
        id: hardening-3
        text: parser keeps caller-owned buffers and no hidden hot-path allocation
        risk: high
        verifymethod: inspection
    }

    element release_gate {
        type: verification
    }

    release_gate - satisfies -> strict_policy
    release_gate - satisfies -> explicit_limits
    release_gate - satisfies -> ownership
```

## Ownership Rules

- The caller owns input bytes.
- The parser does not allocate hidden input buffers.
- Parsed spans remain valid only while the caller buffer remains valid.
- Decoder state stores counters and framing state only.

## Verification Surface

| Layer | Tooling |
|---|---|
| unit tests | Unity |
| corpus tests | parser, semantics, body corpora |
| differential tests | `picohttpparser`, `llhttp` |
| fuzzing | parser, scanner, chunked decoder |
| static analysis | `cppcheck`, `PVS-Studio`, `CodeChecker` |
| formatting | `clang-format` |

## Consumer Profiles

| Consumer | Expected profile |
|---|---|
| `iohttp` | strict by default; explicit leniency only when configured |
| `ioguard` | strict, smaller limits, fail closed on ambiguity |

## Release Conditions

Required before a production-tagged release:

1. `./scripts/quality.sh` passes.
2. Differential corpus is green.
3. Fuzz smoke runs are green.
4. Consumer contracts for `iohttp` and `ioguard` are documented.
