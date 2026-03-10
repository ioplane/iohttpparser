/*
 * iohttpparser — LibFuzzer target for request parser
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/iohttpparser.h>

#include <stddef.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    ihtp_request_t req;
    size_t consumed = 0;

    /* Fuzz with strict policy */
    ihtp_parse_request((const char *)data, size, &req, nullptr, &consumed);

    /* Fuzz with lenient policy */
    ihtp_policy_t lenient = IHTP_POLICY_LENIENT;
    ihtp_parse_request((const char *)data, size, &req, &lenient, &consumed);

    return 0;
}
