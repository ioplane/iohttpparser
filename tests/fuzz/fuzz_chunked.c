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
    size_t bufsz = size;
    ihtp_status_t status;

    memcpy(buf, data, size);
    ihtp_chunked_decoder_t dec = {0};
    status = ihtp_decode_chunked(&dec, buf, &bufsz);
    (void)status;

    memcpy(buf, data, size);
    bufsz = size;
    dec = (ihtp_chunked_decoder_t){.consume_trailer = true};
    status = ihtp_decode_chunked(&dec, buf, &bufsz);
    (void)status;

    return 0;
}
