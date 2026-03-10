# Body Decoder Contracts

`iohttpparser` layer 4 handles body framing only. It does not own transport buffers, allocate hidden state, or decode application-level content codings.

## Chunked Decoder

- `ihtp_decode_chunked()` is incremental. Reuse the same `ihtp_chunked_decoder_t` across calls as more bytes arrive.
- The decoder rewrites the caller buffer in place and updates `*bufsz` to payload bytes retained in that slice.
- `total_decoded` counts payload bytes only.
- On completion, a non-negative return value means success and equals the number of undecoded trailing bytes.
- If `consume_trailer` is `false`, the terminal `CRLF` after the zero chunk and any following bytes remain in trailing data.
- If `consume_trailer` is `true`, trailer lines are consumed until the terminating empty line, then trailing bytes are reported.

## Fixed-Length Decoder

- `ihtp_fixed_decoder_init()` sets the expected payload length.
- `ihtp_decode_fixed()` is purely accounting: no copies, no framing rewrite, no hidden allocation.
- `remaining` and `total_decoded` advance monotonically.
- Passing more bytes than `remaining` is an error.
