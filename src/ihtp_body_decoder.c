/*
 * iohttpparser — Body decoder (Layer 4: chunked + fixed-length)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#include <iohttpparser/ihtp_body.h>

#include <string.h>

/* ─── Chunked decoder states ──────────────────────────────────────────── */

enum {
    CHUNK_SIZE = 0,    /* Reading chunk-size hex digits */
    CHUNK_EXT = 1,     /* Reading chunk-ext (after ';') */
    CHUNK_SIZE_LF = 2, /* Expecting LF after chunk-size CR */
    CHUNK_DATA = 3,    /* Reading chunk-data */
    CHUNK_DATA_CR = 4, /* Expecting CR after chunk-data */
    CHUNK_DATA_LF = 5, /* Expecting LF after chunk-data CR */
    CHUNK_TRAILER = 6, /* Reading trailer section */
    CHUNK_DONE = 7,    /* Complete */
};

enum {
    TRAILER_LINE_START = 0, /* At start of a trailer line */
    TRAILER_DONE_LF = 1,    /* Saw CR for the terminating empty line */
    TRAILER_LINE = 2,       /* Inside a non-empty trailer line */
    TRAILER_LINE_LF = 3,    /* Saw CR inside a trailer line */
};

static uint64_t ihtp_hex_digit_to_u64(char c)
{
    if (c >= '0' && c <= '9') {
        return (uint64_t)(unsigned char)c - (uint64_t)'0';
    }
    if (c >= 'a' && c <= 'f') {
        return ((uint64_t)(unsigned char)c - (uint64_t)'a') + 10u;
    }
    return ((uint64_t)(unsigned char)c - (uint64_t)'A') + 10u;
}

/* ─── Chunked decoder ─────────────────────────────────────────────────── */

ihtp_status_t ihtp_decode_chunked(ihtp_chunked_decoder_t *decoder, char *buf, size_t *bufsz)
{
    if (decoder == nullptr || buf == nullptr || bufsz == nullptr) {
        return IHTP_ERROR;
    }

    size_t src = 0; /* Read position in buf */
    size_t dst = 0; /* Write position in buf (decoded data) */
    size_t avail = *bufsz;

    while (src < avail) {
        switch (decoder->state) {
        case CHUNK_SIZE: {
            char c = buf[src];
            uint64_t digit;
            if (c >= '0' && c <= '9') {
                digit = ihtp_hex_digit_to_u64(c);
            } else if (c >= 'a' && c <= 'f') {
                digit = ihtp_hex_digit_to_u64(c);
            } else if (c >= 'A' && c <= 'F') {
                digit = ihtp_hex_digit_to_u64(c);
            } else if (c == ';') {
                decoder->state = CHUNK_EXT;
                src++;
                continue;
            } else if (c == '\r') {
                if (decoder->hex_count == 0) {
                    return IHTP_ERROR;
                }
                decoder->state = CHUNK_SIZE_LF;
                src++;
                continue;
            } else {
                return IHTP_ERROR;
            }

            if (decoder->hex_count >= 16) {
                return IHTP_ERROR; /* Chunk size too large */
            }
            decoder->bytes_left_in_chunk = decoder->bytes_left_in_chunk * 16 + digit;
            decoder->hex_count++;
            src++;
            break;
        }

        case CHUNK_EXT:
            /* Skip chunk-ext until CR */
            if (buf[src] == '\r') {
                decoder->state = CHUNK_SIZE_LF;
            } else if (buf[src] == '\n') {
                return IHTP_ERROR;
            }
            src++;
            break;

        case CHUNK_SIZE_LF:
            if (buf[src] != '\n') {
                return IHTP_ERROR;
            }
            src++;
            if (decoder->bytes_left_in_chunk == 0) {
                /* Last chunk — go to trailer */
                if (decoder->consume_trailer) {
                    decoder->hex_count = TRAILER_LINE_START;
                    decoder->state = CHUNK_TRAILER;
                } else {
                    decoder->state = CHUNK_DONE;
                    *bufsz = dst;
                    return (ihtp_status_t)(avail - src);
                }
            } else {
                decoder->state = CHUNK_DATA;
            }
            break;

        case CHUNK_DATA: {
            size_t to_copy = avail - src;
            if (to_copy > decoder->bytes_left_in_chunk) {
                to_copy = (size_t)decoder->bytes_left_in_chunk;
            }
            /* In-place copy (dst <= src always) */
            memmove(buf + dst, buf + src, to_copy);
            dst += to_copy;
            src += to_copy;
            decoder->bytes_left_in_chunk -= to_copy;
            decoder->total_decoded += to_copy;
            if (decoder->bytes_left_in_chunk == 0) {
                decoder->state = CHUNK_DATA_CR;
            }
            break;
        }

        case CHUNK_DATA_CR:
            if (buf[src] != '\r') {
                return IHTP_ERROR;
            }
            src++;
            decoder->state = CHUNK_DATA_LF;
            break;

        case CHUNK_DATA_LF:
            if (buf[src] != '\n') {
                return IHTP_ERROR;
            }
            src++;
            /* Reset for next chunk */
            decoder->hex_count = 0;
            decoder->bytes_left_in_chunk = 0;
            decoder->state = CHUNK_SIZE;
            break;

        case CHUNK_TRAILER:
            if (decoder->hex_count == TRAILER_LINE_START) {
                if (buf[src] == '\r') {
                    decoder->hex_count = TRAILER_DONE_LF;
                    src++;
                    break;
                }
                if (buf[src] == '\n') {
                    return IHTP_ERROR;
                }
                decoder->hex_count = TRAILER_LINE;
                src++;
                break;
            }

            if (decoder->hex_count == TRAILER_DONE_LF) {
                if (buf[src] != '\n') {
                    return IHTP_ERROR;
                }
                src++;
                decoder->state = CHUNK_DONE;
                *bufsz = dst;
                return (ihtp_status_t)(avail - src);
            }

            if (decoder->hex_count == TRAILER_LINE) {
                if (buf[src] == '\r') {
                    decoder->hex_count = TRAILER_LINE_LF;
                    src++;
                    break;
                }
                if (buf[src] == '\n') {
                    return IHTP_ERROR;
                }
                src++;
                break;
            }

            if (decoder->hex_count == TRAILER_LINE_LF) {
                if (buf[src] != '\n') {
                    return IHTP_ERROR;
                }
                src++;
                decoder->hex_count = TRAILER_LINE_START;
                break;
            }

            return IHTP_ERROR;

        case CHUNK_DONE:
            *bufsz = dst;
            return (ihtp_status_t)(avail - src);
        }
    }

    *bufsz = dst;
    return IHTP_INCOMPLETE;
}

/* ─── Fixed-length body decoder ───────────────────────────────────────── */

void ihtp_fixed_decoder_init(ihtp_fixed_decoder_t *decoder, uint64_t content_length)
{
    if (decoder != nullptr) {
        decoder->remaining = content_length;
        decoder->total_decoded = 0;
    }
}

ihtp_status_t ihtp_decode_fixed(ihtp_fixed_decoder_t *decoder, size_t len)
{
    if (decoder == nullptr) {
        return IHTP_ERROR;
    }

    if (len > decoder->remaining) {
        return IHTP_ERROR; /* More data than expected */
    }

    decoder->remaining -= len;
    decoder->total_decoded += len;

    if (decoder->remaining == 0) {
        return IHTP_OK;
    }
    return IHTP_INCOMPLETE;
}
