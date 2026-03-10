# Layer Map

## Scanner
- Delimiter search
- CRLF detection
- token classification
- backend dispatch

## Parser
- request-line / status-line parsing
- header extraction
- consumed-byte accounting

## Semantics
- framing decisions
- `Content-Length` / `Transfer-Encoding`
- keep-alive / close
- ambiguity rejection

## Body Decoder
- fixed-length consumption
- chunked decoding
- trailers
