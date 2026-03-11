#include <iohttpparser/iohttpparser.h>

#include <stdio.h>
#include <string.h>

#define CHECKED_PRINTF(...)                                                                     \
    do {                                                                                        \
        if (printf(__VA_ARGS__) < 0) {                                                          \
            return 1;                                                                           \
        }                                                                                       \
    } while (0)

#define CHECKED_EPRINTF(...)                                                                    \
    do {                                                                                        \
        if (fprintf(stderr, __VA_ARGS__) < 0) {                                                 \
            return 1;                                                                           \
        }                                                                                       \
    } while (0)

int main(void)
{
    const char *wire = "POST /upload HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Transfer-Encoding: chunked\r\n"
                       "Expect: 100-continue\r\n"
                       "Trailer: Digest\r\n"
                       "\r\n"
                       "4\r\nWiki\r\n"
                       "5\r\npedia\r\n"
                       "0\r\n"
                       "Digest: sha-256=demo\r\n"
                       "\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_request_t req = {0};
    ihtp_parser_state_t st;
    size_t consumed = 0;
    char body_buf[256];
    size_t body_len = 0;
    ihtp_chunked_decoder_t dec = {.consume_trailer = false};

    ihtp_parser_state_init(&st, IHTP_PARSER_MODE_REQUEST);

    if (ihtp_parse_request_stateful(&st, wire, strlen(wire), &req, &policy, &consumed) != IHTP_OK) {
        CHECKED_EPRINTF("Failed to parse request\n");
        return 1;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        CHECKED_EPRINTF("Failed to apply request semantics\n");
        return 1;
    }

    CHECKED_PRINTF("Method:   %s\n", ihtp_method_to_str(req.method));
    CHECKED_PRINTF("Expect:   %s\n", req.expects_continue ? "100-continue" : "none");
    CHECKED_PRINTF("Trailer:  %s\n", req.has_trailer_fields ? "advertised" : "none");
    CHECKED_PRINTF("Body:     %d\n", req.body_mode);
    CHECKED_PRINTF("\n");
    CHECKED_PRINTF("Consumer handoff:\n");
    CHECKED_PRINTF("- if expects_continue is set, the consumer may emit 100 Continue before reading the body\n");
    CHECKED_PRINTF("- has_trailer_fields tells the consumer that trailing fields may follow the chunked body\n");
    CHECKED_PRINTF("- this example leaves consume_trailer disabled to keep trailer ownership in consumer code\n");

    if (req.body_mode != IHTP_BODY_CHUNKED) {
        CHECKED_EPRINTF("Expected chunked body mode\n");
        return 1;
    }

    body_len = strlen(wire) - consumed;
    memcpy(body_buf, wire + consumed, body_len + 1U);

    int trailing = ihtp_decode_chunked(&dec, body_buf, &body_len);
    if (trailing < 0) {
        CHECKED_EPRINTF("Chunked decode failed: %d\n", trailing);
        return 1;
    }

    CHECKED_PRINTF("Decoded body prefix: %.*s\n", (int)body_len, body_buf);
    CHECKED_PRINTF("Trailing bytes returned to consumer: %d\n", trailing);
    CHECKED_PRINTF("Consumer note: trailer bytes stay outside parser semantics and remain consumer-owned.\n");

    return 0;
}
