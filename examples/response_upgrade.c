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
    const char *wire = "HTTP/1.1 101 Switching Protocols\r\n"
                       "Connection: Upgrade\r\n"
                       "Upgrade: websocket\r\n"
                       "\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_response_t resp = {0};
    ihtp_parser_state_t st;
    size_t consumed = 0;

    ihtp_parser_state_init(&st, IHTP_PARSER_MODE_RESPONSE);

    if (ihtp_parse_response_stateful(&st, wire, strlen(wire), &resp, &policy, &consumed) != IHTP_OK) {
        CHECKED_EPRINTF("Failed to parse response\n");
        return 1;
    }
    if (ihtp_response_apply_semantics(&resp, &policy) != IHTP_OK) {
        CHECKED_EPRINTF("Failed to apply response semantics\n");
        return 1;
    }

    CHECKED_PRINTF("Status:    %d\n", resp.status_code);
    CHECKED_PRINTF("Upgrade:   %s\n", resp.protocol_upgrade ? "yes" : "no");
    CHECKED_PRINTF("Body mode: %d\n", resp.body_mode);
    CHECKED_PRINTF("Consumed:  %zu\n", consumed);
    CHECKED_PRINTF("\n");
    CHECKED_PRINTF("Consumer handoff:\n");
    CHECKED_PRINTF("- protocol_upgrade means the application should switch protocols after the header block\n");
    CHECKED_PRINTF("- iohttpparser does not interpret WebSocket or other upgraded bytes\n");
    CHECKED_PRINTF("- after a 101 response, framing ownership moves to the upgraded protocol handler\n");

    return 0;
}
