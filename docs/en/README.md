[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iohttpparser)
[![C23](https://img.shields.io/badge/ISO-IEC%209899%3A2024-00599C?style=for-the-badge)](https://www.iso.org/standard/82075.html)
[![RFC 9110](https://img.shields.io/badge/RFC-9110-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9110.html)
[![RFC 9112](https://img.shields.io/badge/RFC-9112-1a73e8?style=for-the-badge)](https://www.rfc-editor.org/rfc/rfc9112.html)
[![Mermaid](https://img.shields.io/badge/Mermaid-Flowchart-ff3670?style=for-the-badge)](https://mermaid.js.org/syntax/flowchart.html)

# English Documentation

Authoritative documentation for `iohttpparser`.

## Documentation Map

```mermaid
graph TD
    IDX["docs/en/README.md"]

    A1["01-architecture.md"]
    A2["02-comparison.md"]
    A3["03-production-hardening.md"]
    A4["04-parser-ecosystem-comparison.md"]
    A5["05-consumer-contracts.md"]
    A6["06-parser-state.md"]
    A7["07-body-decoder.md"]
    A8["08-testing-methodology.md"]
    A9["09-test-results.md"]
    A10["10-extended-contract-methodology.md"]
    A11["11-extended-contract-results.md"]
    A12["12-release-candidate-checklist.md"]
    API["api-reference.md"]

    IDX --> A1
    IDX --> A2
    IDX --> A3
    IDX --> A4
    IDX --> A5
    IDX --> A6
    IDX --> A7
    IDX --> A8
    IDX --> A9
    IDX --> A10
    IDX --> A11
    IDX --> A12
    IDX --> API

    style IDX fill:#1a73e8,color:#fff
```

## Numbered Documents

| # | Document | Scope |
|---|---|---|
| 01 | [Architecture](./01-architecture.md) | Scope, layers, ownership, integration boundaries |
| 02 | [Comparison](./02-comparison.md) | Feature and contract comparison with `picohttpparser` and `llhttp` |
| 03 | [Production Hardening](./03-production-hardening.md) | Strict policy, limits, rejection classes, verification surface |
| 04 | [Parser Ecosystem Comparison](./04-parser-ecosystem-comparison.md) | Responsibility split between parser core, consumer, and adjacent layers |
| 05 | [Consumer Contracts](./05-consumer-contracts.md) | Integration contract for `iohttp` and `ioguard` |
| 06 | [Parser State](./06-parser-state.md) | Stateful parser API and ownership rules |
| 07 | [Body Decoder](./07-body-decoder.md) | Chunked and fixed-length decoder contracts |
| 08 | [Testing Methodology](./08-testing-methodology.md) | PMI, PSI, comparison rules, and artifact publication |
| 09 | [Test Results](./09-test-results.md) | Published PMI/PSI results and artifact index |
| 10 | [Extended Contract Methodology](./10-extended-contract-methodology.md) | Methodology for capabilities outside the common parser-core matrix |
| 11 | [Extended Contract Results](./11-extended-contract-results.md) | Result status for the extended `iohttpparser` contract |
| 12 | [Release Candidate Checklist](./12-release-candidate-checklist.md) | Release-candidate verification, published evidence, and release-notes structure |

## Reference

| Document | Purpose |
|---|---|
| [api-reference.md](./api-reference.md) | Doxygen entry page |
| [../../SUPPORT.md](../../SUPPORT.md) | Support channels and required report inputs |
| [../README.md](../README.md) | Top-level docs index |
| [../plans/README.md](../plans/README.md) | Plans index |
| [../rfc/README.md](../rfc/README.md) | Local RFC mirror |
