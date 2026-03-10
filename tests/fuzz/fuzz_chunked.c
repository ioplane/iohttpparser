/*
 * iohttpparser — LibFuzzer target for chunked decoder
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/ihtp_body.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size > 65536) {
        return 0;
    }

    /* ihtp_decode_chunked modifies buffer in-place, so copy */
    char buf[65536];
    memcpy(buf, data, size);
    size_t bufsz = size;

    ihtp_chunked_decoder_t dec = {0};
    ihtp_decode_chunked(&dec, buf, &bufsz);

    return 0;
}
