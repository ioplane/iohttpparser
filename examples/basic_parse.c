/*
 * iohttpparser — Basic usage example
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/iohttpparser.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("iohttpparser %s (SIMD level: 0x%02x)\n\n", ihtp_version(), ihtp_scanner_simd_level());

    /* ── Parse a request ──────────────────────────────────────────────── */

    const char *raw = "GET /api/users?page=1 HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "Accept: application/json\r\n"
                      "Connection: keep-alive\r\n"
                      "\r\n";

    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(raw, strlen(raw), &req, nullptr, &consumed);
    if (status != IHTP_OK) {
        fprintf(stderr, "Parse error: %d\n", status);
        return 1;
    }

    printf("Method:  %s\n", ihtp_method_to_str(req.method));
    printf("Path:    %.*s\n", (int)req.path_len, req.path);
    printf("Version: HTTP/1.%d\n", req.version);
    printf("Headers: %zu\n", req.num_headers);

    for (size_t i = 0; i < req.num_headers; i++) {
        printf("  %.*s: %.*s\n", (int)req.headers[i].name_len, req.headers[i].name,
               (int)req.headers[i].value_len, req.headers[i].value);
    }

    printf("Consumed: %zu bytes\n", consumed);

    return 0;
}
