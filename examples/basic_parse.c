/*
 * iohttpparser — Basic usage example
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/iohttpparser.h>
#include <iohttpparser/ihtp_scanner.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("iohttpparser %s (SIMD level: 0x%02x)\n\n", ihtp_version(), ihtp_scanner_simd_level());

    /* ── Parse a request incrementally and apply semantics ───────────── */

    const char *full = "POST /upload HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Transfer-Encoding: chunked\r\n"
                       "Trailer: Digest\r\n"
                       "Expect: 100-continue\r\n"
                       "\r\n"
                       "4\r\nWiki\r\n"
                       "5\r\npedia\r\n"
                       "0\r\n"
                       "Digest: sha-256=demo\r\n"
                       "\r\n";
    const size_t split = strlen(full) / 2U;
    const ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_request_t req = {0};
    ihtp_parser_state_t st;
    size_t consumed = 0;
    size_t body_len = 0;
    char body_buf[256];
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};

    ihtp_parser_state_init(&st, IHTP_PARSER_MODE_REQUEST);

    ihtp_status_t status =
        ihtp_parse_request_stateful(&st, full, split, &req, &policy, &consumed);
    if (status != IHTP_INCOMPLETE) {
        fprintf(stderr, "Expected incremental parse to be incomplete, got %d\n", status);
        return 1;
    }

    status = ihtp_parse_request_stateful(&st, full, strlen(full), &req, &policy, &consumed);
    if (status != IHTP_OK) {
        fprintf(stderr, "Parse error: %d\n", status);
        return 1;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        fprintf(stderr, "Semantics error\n");
        return 1;
    }

    printf("Method:  %s\n", ihtp_method_to_str(req.method));
    printf("Path:    %.*s\n", (int)req.path_len, req.path);
    printf("Version: HTTP/1.%d\n", req.version);
    printf("Headers: %zu\n", req.num_headers);
    printf("Body:    %d\n", req.body_mode);
    printf("Expect:  %s\n", req.expects_continue ? "100-continue" : "none");
    printf("Upgrade: %s\n", req.protocol_upgrade ? "yes" : "no");
    printf("Trailer: %s\n", req.has_trailer_fields ? "advertised" : "none");

    for (size_t i = 0; i < req.num_headers; i++) {
        printf("  %.*s: %.*s\n", (int)req.headers[i].name_len, req.headers[i].name,
               (int)req.headers[i].value_len, req.headers[i].value);
    }

    printf("Consumed: %zu bytes\n", consumed);
    if (req.body_mode == IHTP_BODY_CHUNKED) {
        body_len = strlen(full) - consumed;
        memcpy(body_buf, full + consumed, body_len + 1U);
        int trailing = ihtp_decode_chunked(&dec, body_buf, &body_len);
        if (trailing < 0) {
            fprintf(stderr, "Chunked decode error: %d\n", trailing);
            return 1;
        }
        printf("Decoded body: %.*s\n", (int)body_len, body_buf);
        printf("Trailing bytes after chunked decode: %d\n", trailing);
    }

    return 0;
}
