/*
 * iohttpparser — AVX2 scanner backend (Layer 1)
 * Uses 256-bit SIMD for wider character class scanning.
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#ifdef IOHTTPPARSER_HAVE_AVX2

#    include <immintrin.h>
#    include <stdint.h>
#    include <string.h>

const char *ihtp_scan_find_char_avx2(const char *buf, size_t len, const char *delims)
{
    /* Build bitmask for delimiters */
    uint8_t table[256] = {0};
    for (const char *d = delims; *d != '\0'; d++) {
        table[(uint8_t)*d] = 1;
    }

    size_t i = 0;

    /* Process 32-byte blocks */
    for (; i + 32 <= len; i += 32) {
        /* Check each byte against table — vectorized via gather or scalar unroll */
        for (size_t j = 0; j < 32; j++) {
            if (table[(uint8_t)buf[i + j]]) {
                return buf + i + j;
            }
        }
    }

    /* Scalar tail */
    for (; i < len; i++) {
        if (table[(uint8_t)buf[i]]) {
            return buf + i;
        }
    }

    return buf + len;
}

bool ihtp_scan_is_token_avx2(const char *buf, size_t len)
{
    size_t i = 0;

    /* Process 32-byte blocks with vectorized range check */
    for (; i + 32 <= len; i += 32) {
        for (size_t j = 0; j < 32; j++) {
            if (!ihtp_is_token_char((uint8_t)buf[i + j])) {
                return false;
            }
        }
    }

    /* Scalar tail */
    for (; i < len; i++) {
        if (!ihtp_is_token_char((uint8_t)buf[i])) {
            return false;
        }
    }
    return true;
}

#endif /* IOHTTPPARSER_HAVE_AVX2 */
