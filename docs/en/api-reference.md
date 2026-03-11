# API Reference

This page is the Doxygen entry point for the public `iohttpparser` surface.

Use it together with:

- [parser-state.md](parser-state.md) for incremental parser lifecycle
- [body-decoder.md](body-decoder.md) for chunked/fixed-length decoder handoff
- [05-consumer-contracts.md](05-consumer-contracts.md) for `iohttp` and `ioguard` integration expectations

Public API headers:

- `include/iohttpparser/iohttpparser.h`
- `include/iohttpparser/ihtp_types.h`
- `include/iohttpparser/ihtp_parser.h`
- `include/iohttpparser/ihtp_semantics.h`
- `include/iohttpparser/ihtp_body.h`
- `include/iohttpparser/ihtp_scanner.h`

Module map:

- `Public API`:
  umbrella header and version helpers
- `Types and Policies`:
  public enums, structs, limits, and named policy presets
- `Parser API`:
  stateless and stateful request/response/header parsing
- `Semantics API`:
  framing, keep-alive, upgrade, `Expect`, and trailer handoff
- `Body Decoder API`:
  chunked and fixed-length decoder contracts
- `Scanner API`:
  low-level delimiter search and token validation helpers

Key properties of the public contract:

- strict-by-default HTTP/1.1 parsing
- zero-copy spans that point into caller-owned input
- explicit parser, semantics, and body-decoder separation
- stateful and stateless parser entry points
- consumer-owned handoff for upgrades, `Expect: 100-continue`, and trailers

Embedder checklist:

- keep the input buffer alive for as long as zero-copy spans are in use
- treat parser, semantics, and body decoding as separate stages
- use named policy presets instead of open-coded anonymous policies
- reset parser state only when starting a new message on a fresh logical stream
