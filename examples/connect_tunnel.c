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
    const char *wire = "CONNECT vpn.example.com:443 HTTP/1.1\r\n"
                       "Host: vpn.example.com:443\r\n"
                       "Proxy-Connection: keep-alive\r\n"
                       "\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_IOGUARD;
    ihtp_request_t req = {0};
    ihtp_parser_state_t st;
    size_t consumed = 0;

    ihtp_parser_state_init(&st, IHTP_PARSER_MODE_REQUEST);

    if (ihtp_parse_request_stateful(&st, wire, strlen(wire), &req, &policy, &consumed) != IHTP_OK) {
        CHECKED_EPRINTF("Failed to parse CONNECT request\n");
        return 1;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        CHECKED_EPRINTF("Failed to apply semantics\n");
        return 1;
    }

    CHECKED_PRINTF("Method:      %s\n", ihtp_method_to_str(req.method));
    CHECKED_PRINTF("Authority:   %.*s\n", (int)req.path_len, req.path);
    CHECKED_PRINTF("Body mode:   %d\n", req.body_mode);
    CHECKED_PRINTF("Keep-alive:  %s\n", req.keep_alive ? "yes" : "no");
    CHECKED_PRINTF("Upgrade:     %s\n", req.protocol_upgrade ? "yes" : "no");
    CHECKED_PRINTF("Expect:      %s\n", req.expects_continue ? "100-continue" : "none");
    CHECKED_PRINTF("Consumed:    %zu\n", consumed);
    CHECKED_PRINTF("\n");
    CHECKED_PRINTF("Consumer handoff:\n");
    CHECKED_PRINTF("- CONNECT stays visible through req.method == IHTP_METHOD_CONNECT\n");
    CHECKED_PRINTF("- req.path is the authority-form target for tunnel setup\n");
    CHECKED_PRINTF("- no redundant CONNECT boolean is needed in the parser API\n");

    return 0;
}
