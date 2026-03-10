/*
 * iohttpparser — Body decoder API (Layer 4: chunked + fixed-length)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_BODY_H
#define IOHTTPPARSER_IHTP_BODY_H

#include <iohttpparser/ihtp_types.h>

/**
 * @brief Chunked transfer decoder state.
 *
 * Zero-initialize before first use.
 *
 * Contract:
 * - state is preserved across calls for incremental decoding
 * - total_decoded counts only payload bytes, never chunk framing or trailers
 * - if consume_trailer is false, completion leaves the terminal CRLF and any
 *   following bytes undecoded in the caller buffer
 * - if consume_trailer is true, trailer lines are consumed until the
 *   terminating empty line before reporting completion
 */
typedef struct {
    uint64_t bytes_left_in_chunk; /**< Remaining bytes in current chunk */
    uint64_t total_decoded;       /**< Total decoded body bytes */
    char hex_count;               /**< Internal: hex digits parsed */
    char state;                   /**< Internal: decoder state machine */
    bool consume_trailer;         /**< If true, consume trailing headers */
} ihtp_chunked_decoder_t;

/**
 * @brief Decode chunked transfer encoding (in-place).
 *
 * Rewrites buf in-place, removing chunk framing.
 * On return, *bufsz is updated to decoded data length.
 * The function is incremental: callers may invoke it repeatedly with
 * subsequent buffer slices using the same decoder state.
 *
 * @param decoder  Decoder state (zero-init before first call).
 * @param buf      Input/output buffer (modified in-place).
 * @param bufsz    Input: available bytes; Output: decoded bytes.
 * @return >= 0: complete (value = undecoded trailing bytes),
 *         IHTP_INCOMPLETE: need more data,
 *         IHTP_ERROR: malformed chunked data.
 */
[[nodiscard]] ihtp_status_t ihtp_decode_chunked(ihtp_chunked_decoder_t *decoder, char *buf,
                                                size_t *bufsz);

/**
 * @brief Fixed-length body decoder state.
 *
 * Initialize with ihtp_fixed_decoder_init().
 */
typedef struct {
    uint64_t remaining;     /**< Bytes remaining to read */
    uint64_t total_decoded; /**< Total decoded body bytes */
} ihtp_fixed_decoder_t;

/**
 * @brief Initialize fixed-length body decoder.
 *
 * @param decoder        Decoder state.
 * @param content_length Expected body length from Content-Length header.
 */
void ihtp_fixed_decoder_init(ihtp_fixed_decoder_t *decoder, uint64_t content_length);

/**
 * @brief Consume fixed-length body data.
 *
 * @param decoder Decoder state.
 * @param len     Number of bytes received.
 * @return IHTP_OK if all expected bytes consumed, IHTP_INCOMPLETE if more
 *         data is needed, IHTP_ERROR if len exceeds remaining bytes.
 */
[[nodiscard]] ihtp_status_t ihtp_decode_fixed(ihtp_fixed_decoder_t *decoder, size_t len);

#endif /* IOHTTPPARSER_IHTP_BODY_H */
