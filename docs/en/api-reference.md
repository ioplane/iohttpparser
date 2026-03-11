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

Key properties of the public contract:

- strict-by-default HTTP/1.1 parsing
- zero-copy spans that point into caller-owned input
- explicit parser, semantics, and body-decoder separation
- stateful and stateless parser entry points
- consumer-owned handoff for upgrades, `Expect: 100-continue`, and trailers
