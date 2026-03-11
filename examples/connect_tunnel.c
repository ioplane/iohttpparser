/*
 * iohttpparser — CONNECT tunnel handoff example
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/iohttpparser.h>

#include <stdio.h>
#include <string.h>

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

    if (ihtp_parse_request_stateful(&st, wire, strlen(wire), &req, &policy, &consumed) !=
        IHTP_OK) {
        fprintf(stderr, "Failed to parse CONNECT request\n");
        return 1;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        fprintf(stderr, "Failed to apply semantics\n");
        return 1;
    }

    printf("Method:      %s\n", ihtp_method_to_str(req.method));
    printf("Authority:   %.*s\n", (int)req.path_len, req.path);
    printf("Body mode:   %d\n", req.body_mode);
    printf("Keep-alive:  %s\n", req.keep_alive ? "yes" : "no");
    printf("Upgrade:     %s\n", req.protocol_upgrade ? "yes" : "no");
    printf("Expect:      %s\n", req.expects_continue ? "100-continue" : "none");
    printf("Consumed:    %zu\n", consumed);
    printf("\n");
    printf("Consumer handoff:\n");
    printf("- CONNECT stays visible through req.method == IHTP_METHOD_CONNECT\n");
    printf("- req.path is the authority-form target for tunnel setup\n");
    printf("- no redundant CONNECT boolean is needed in the parser API\n");

    return 0;
}
