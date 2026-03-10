/*
 * iohttpparser — LibFuzzer target for scanner backend equivalence
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#include <iohttpparser/ihtp_scanner.h>

#include <stddef.h>
#include <stdint.h>

static void fuzz_fail(void)
{
    __builtin_trap();
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char delims[33] = {0};
    size_t delim_len = 0;
    int simd_level = ihtp_scanner_simd_level();
    const char *buf;
    size_t buf_len;
    const char *expected_find;
    bool expected_token;
    size_t requested_delim_len;

    if (size == 0) {
        return 0;
    }

    requested_delim_len = (size_t)(data[0] % 32U);
    if (requested_delim_len > size - 1) {
        requested_delim_len = size - 1;
    }

    for (size_t i = 0; i < requested_delim_len; i++) {
        uint8_t byte = data[i + 1];

        if (byte == 0) {
            continue;
        }
        delims[delim_len++] = (char)byte;
    }
    delims[delim_len] = '\0';

    buf = (const char *)(data + 1 + requested_delim_len);
    buf_len = size - 1 - requested_delim_len;

    expected_find = ihtp_scan_find_char_scalar(buf, buf_len, delims);
    if (ihtp_scan_find_char(buf, buf_len, delims) != expected_find) {
        fuzz_fail();
    }

#ifdef IOHTTPPARSER_HAVE_SSE42
    if ((simd_level & 0x01) != 0 &&
        ihtp_scan_find_char_sse42(buf, buf_len, delims) != expected_find) {
        fuzz_fail();
    }
#endif

#ifdef IOHTTPPARSER_HAVE_AVX2
    if ((simd_level & 0x02) != 0 &&
        ihtp_scan_find_char_avx2(buf, buf_len, delims) != expected_find) {
        fuzz_fail();
    }
#endif

    expected_token = ihtp_scan_is_token_scalar(buf, buf_len);
    if (ihtp_scan_is_token(buf, buf_len) != expected_token) {
        fuzz_fail();
    }

#ifdef IOHTTPPARSER_HAVE_SSE42
    if ((simd_level & 0x01) != 0 && ihtp_scan_is_token_sse42(buf, buf_len) != expected_token) {
        fuzz_fail();
    }
#endif

#ifdef IOHTTPPARSER_HAVE_AVX2
    if ((simd_level & 0x02) != 0 && ihtp_scan_is_token_avx2(buf, buf_len) != expected_token) {
        fuzz_fail();
    }
#endif

    return 0;
}
