# RFC Priority Map

## P0
- RFC 9112: syntax, framing, chunked, message boundaries
- RFC 9110: semantics, fields, method behavior
- RFC 3986: request-target structure

## P1
- RFC 6455: handshake-related parser consequences
- RFC 6265: keep cookie semantics outside parser core

## Working Rule
- Resolve ambiguous parser behavior toward strict rejection unless the repository explicitly documents a compatibility exception.
