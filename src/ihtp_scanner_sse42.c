/*
 * iohttpparser — SSE4.2 scanner backend (Layer 1)
 * Uses PCMPESTRI for fast character class scanning.
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#ifdef IOHTTPPARSER_HAVE_SSE42

#    include <nmmintrin.h>
#    include <stdint.h>
#    include <string.h>

const char *ihtp_scan_find_char_sse42(const char *buf, size_t len, const char *delims)
{
    size_t dlen = strlen(delims);
    if (dlen == 0 || dlen > 16) {
        return ihtp_scan_find_char_scalar(buf, len, delims);
    }

    __m128i needle = _mm_setzero_si128();
    memcpy(&needle, delims, dlen);
    int needle_len = (int)dlen;

    size_t i = 0;

    /* Process 16-byte blocks */
    for (; i + 16 <= len; i += 16) {
        __m128i haystack = _mm_loadu_si128((const __m128i *)(buf + i));
        int idx = _mm_cmpestri(needle, needle_len, haystack, 16,
                               _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT);
        if (idx < 16) {
            return buf + i + (size_t)idx;
        }
    }

    /* Handle remaining bytes with scalar fallback */
    for (; i < len; i++) {
        for (size_t d = 0; d < dlen; d++) {
            if (buf[i] == delims[d]) {
                return buf + i;
            }
        }
    }

    return buf + len;
}

bool ihtp_scan_is_token_sse42(const char *buf, size_t len)
{
    /* The token character class does not fit safely in a single PCMPESTRI range table. */
    return ihtp_scan_is_token_scalar(buf, len);
}

#endif /* IOHTTPPARSER_HAVE_SSE42 */
