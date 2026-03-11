#include <unity/unity.h>

#include <iohttpparser/ihtp_body.h>
#include <iohttpparser/ihtp_parser.h>
#include <iohttpparser/ihtp_semantics.h>

#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_iohttp_style_chunked_pipeline_reuses_parser_state(void)
{
    static const char next_request[] = "GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n";
    static const char wire[] = "POST /upload HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "Trailer: Digest\r\n"
                               "\r\n"
                               "4\r\n"
                               "Wiki\r\n"
                               "5\r\n"
                               "pedia\r\n"
                               "0\r\n"
                               "Digest: sha-256=abc\r\n"
                               "\r\n"
                               "GET /next HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "\r\n";
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    size_t consumed = 17;
    size_t split = 0;
    size_t body_len = 0;
    const char *headers_end = strstr(wire, "\r\n\r\n");
    char tail[256];
    const char *next = nullptr;
    ihtp_status_t status;
    int trailing = 0;

    TEST_ASSERT_NOT_NULL(headers_end);
    split = (size_t)(headers_end + 3 - wire);

    memset(&req, 0, sizeof(req));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);

    status = ihtp_parse_request_stateful(&state, wire, split, &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_HEADERS, state.phase);
    TEST_ASSERT_EQUAL_UINT(0, consumed);

    status = ihtp_parse_request_stateful(&state, wire, strlen(wire), &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT((size_t)(headers_end + 4 - wire), consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_POST, req.method);
    TEST_ASSERT_EQUAL_STRING_LEN("/upload", req.path, req.path_len);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_request_apply_semantics(&req, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, req.body_mode);
    TEST_ASSERT_TRUE(req.keep_alive);
    TEST_ASSERT_TRUE(req.has_trailer_fields);
    TEST_ASSERT_FALSE(req.expects_continue);
    TEST_ASSERT_FALSE(req.protocol_upgrade);

    body_len = strlen(wire) - consumed;
    TEST_ASSERT_TRUE(body_len < sizeof(tail));
    memcpy(tail, wire + consumed, body_len + 1U);

    trailing = ihtp_decode_chunked(&dec, tail, &body_len);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, trailing);
    TEST_ASSERT_EQUAL_UINT(9, body_len);
    TEST_ASSERT_EQUAL_STRING_LEN("Wikipedia", tail, body_len);
    TEST_ASSERT_EQUAL_UINT(strlen(next_request), (size_t)trailing);

    next = tail + body_len;
    TEST_ASSERT_EQUAL_STRING_LEN(next_request, next, (size_t)trailing);

    ihtp_parser_state_reset(&state);
    memset(&req, 0, sizeof(req));
    consumed = 0;

    status = ihtp_parse_request_stateful(&state, next, (size_t)trailing, &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(strlen(next_request), consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_GET, req.method);
    TEST_ASSERT_EQUAL_STRING_LEN("/next", req.path, req.path_len);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_request_apply_semantics(&req, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_NONE, req.body_mode);
    TEST_ASSERT_TRUE(req.keep_alive);
    TEST_ASSERT_FALSE(req.has_trailer_fields);
}

void test_iohttp_style_expect_continue_leaves_trailers_to_consumer(void)
{
    static const char next_request[] = "GET /final HTTP/1.1\r\nHost: example.com\r\n\r\n";
    static const char trailer_block[] = "Digest: sha-256=demo\r\n\r\n";
    static const char wire[] = "POST /upload HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "Expect: 100-continue\r\n"
                               "Trailer: Digest\r\n"
                               "\r\n"
                               "4\r\n"
                               "Wiki\r\n"
                               "5\r\n"
                               "pedia\r\n"
                               "0\r\n"
                               "Digest: sha-256=demo\r\n"
                               "\r\n"
                               "GET /final HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "\r\n";
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    ihtp_chunked_decoder_t dec = {.consume_trailer = false};
    size_t consumed = 0;
    size_t body_len = 0;
    const char *headers_end = strstr(wire, "\r\n\r\n");
    char tail[320];
    const char *trailer_and_next = nullptr;
    const char *next = nullptr;
    const char *trailer_end = nullptr;
    ihtp_status_t status;
    int trailing = 0;

    TEST_ASSERT_NOT_NULL(headers_end);

    memset(&req, 0, sizeof(req));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);

    status = ihtp_parse_request_stateful(&state, wire, strlen(wire), &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT((size_t)(headers_end + 4 - wire), consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_request_apply_semantics(&req, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, req.body_mode);
    TEST_ASSERT_TRUE(req.keep_alive);
    TEST_ASSERT_TRUE(req.expects_continue);
    TEST_ASSERT_TRUE(req.has_trailer_fields);

    body_len = strlen(wire) - consumed;
    TEST_ASSERT_TRUE(body_len < sizeof(tail));
    memcpy(tail, wire + consumed, body_len + 1U);

    trailing = ihtp_decode_chunked(&dec, tail, &body_len);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, trailing);
    TEST_ASSERT_EQUAL_UINT(9, body_len);
    TEST_ASSERT_EQUAL_STRING_LEN("Wikipedia", tail, body_len);

    trailer_and_next = tail + body_len;
    TEST_ASSERT_EQUAL_STRING_LEN(trailer_block, trailer_and_next, strlen(trailer_block));
    TEST_ASSERT_EQUAL_UINT(strlen(trailer_block) + strlen(next_request), (size_t)trailing);

    trailer_end = strstr(trailer_and_next, "\r\n\r\n");
    TEST_ASSERT_NOT_NULL(trailer_end);
    next = trailer_end + 4;
    TEST_ASSERT_EQUAL_STRING_LEN(next_request, next, strlen(next_request));

    ihtp_parser_state_reset(&state);
    memset(&req, 0, sizeof(req));
    consumed = 0;

    status =
        ihtp_parse_request_stateful(&state, next, strlen(next_request), &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(strlen(next_request), consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_GET, req.method);
    TEST_ASSERT_EQUAL_STRING_LEN("/final", req.path, req.path_len);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_request_apply_semantics(&req, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_NONE, req.body_mode);
    TEST_ASSERT_TRUE(req.keep_alive);
    TEST_ASSERT_FALSE(req.expects_continue);
    TEST_ASSERT_FALSE(req.has_trailer_fields);
}

void test_iohttp_style_response_upgrade_handoff_leaves_protocol_bytes(void)
{
    static const char upgraded_bytes[] = "\x81\x05hello";
    static const char wire[] = "HTTP/1.1 101 Switching Protocols\r\n"
                               "Connection: Upgrade\r\n"
                               "Upgrade: websocket\r\n"
                               "\r\n"
                               "\x81\x05hello";
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_response_t resp;
    ihtp_parser_state_t state;
    size_t consumed = 99;
    const char *headers_end = strstr(wire, "\r\n\r\n");
    const char *next = nullptr;
    ihtp_status_t status;

    TEST_ASSERT_NOT_NULL(headers_end);

    memset(&resp, 0, sizeof(resp));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);

    status = ihtp_parse_response_stateful(&state, wire, strlen(wire), &resp, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT((size_t)(headers_end + 4 - wire), consumed);
    TEST_ASSERT_EQUAL_INT(101, resp.status_code);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_response_apply_semantics(&resp, &policy));
    TEST_ASSERT_TRUE(resp.protocol_upgrade);
    TEST_ASSERT_FALSE(resp.keep_alive);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_NONE, resp.body_mode);
    TEST_ASSERT_FALSE(resp.has_trailer_fields);

    next = wire + consumed;
    TEST_ASSERT_EQUAL_STRING_LEN(upgraded_bytes, next, sizeof(upgraded_bytes) - 1U);
}

void test_iohttp_style_fixed_length_response_pipelines_next_response(void)
{
    static const char next_response[] = "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";
    static const char wire[] = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: 5\r\n"
                               "\r\n"
                               "hello"
                               "HTTP/1.1 204 No Content\r\n"
                               "Connection: close\r\n"
                               "\r\n";
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_response_t resp;
    ihtp_parser_state_t state;
    ihtp_fixed_decoder_t dec;
    size_t consumed = 0;
    const char *headers_end = strstr(wire, "\r\n\r\n");
    const char *body = nullptr;
    const char *next = nullptr;
    ihtp_status_t status;

    TEST_ASSERT_NOT_NULL(headers_end);

    memset(&resp, 0, sizeof(resp));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);

    status = ihtp_parse_response_stateful(&state, wire, strlen(wire), &resp, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT((size_t)(headers_end + 4 - wire), consumed);
    TEST_ASSERT_EQUAL_INT(200, resp.status_code);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_response_apply_semantics(&resp, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_FIXED, resp.body_mode);
    TEST_ASSERT_EQUAL_UINT64(5, resp.content_length);
    TEST_ASSERT_TRUE(resp.keep_alive);

    body = wire + consumed;
    TEST_ASSERT_EQUAL_STRING_LEN("hello", body, 5);

    ihtp_fixed_decoder_init(&dec, resp.content_length);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_decode_fixed(&dec, 5));

    next = body + 5;
    TEST_ASSERT_EQUAL_STRING_LEN(next_response, next, strlen(next_response));

    ihtp_parser_state_reset(&state);
    memset(&resp, 0, sizeof(resp));
    consumed = 0;

    status = ihtp_parse_response_stateful(&state, next, strlen(next_response), &resp, &policy,
                                          &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(strlen(next_response), consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_response_apply_semantics(&resp, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_NONE, resp.body_mode);
    TEST_ASSERT_FALSE(resp.keep_alive);
}

void test_iohttp_style_eof_response_hands_remaining_bytes_to_consumer(void)
{
    static const char payload[] = "compressed-stream";
    static const char wire[] = "HTTP/1.1 200 OK\r\n"
                               "Transfer-Encoding: gzip\r\n"
                               "\r\n"
                               "compressed-stream";
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_response_t resp;
    ihtp_parser_state_t state;
    size_t consumed = 0;
    const char *headers_end = strstr(wire, "\r\n\r\n");
    const char *body = nullptr;
    ihtp_status_t status;

    TEST_ASSERT_NOT_NULL(headers_end);

    memset(&resp, 0, sizeof(resp));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);

    status = ihtp_parse_response_stateful(&state, wire, strlen(wire), &resp, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT((size_t)(headers_end + 4 - wire), consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_response_apply_semantics(&resp, &policy));
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_EOF, resp.body_mode);
    TEST_ASSERT_FALSE(resp.keep_alive);
    TEST_ASSERT_FALSE(resp.protocol_upgrade);

    body = wire + consumed;
    TEST_ASSERT_EQUAL_STRING_LEN(payload, body, strlen(payload));
}

void test_ioguard_style_rejects_ambiguous_te_cl_request(void)
{
    static const char wire[] = "POST /blocked HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "Content-Length: 12\r\n"
                               "\r\n";
    ihtp_policy_t policy = IHTP_POLICY_IOGUARD;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    size_t consumed = 0;
    ihtp_status_t status;

    memset(&req, 0, sizeof(req));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);

    status = ihtp_parse_request_stateful(&state, wire, strlen(wire), &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_POST, req.method);
    TEST_ASSERT_EQUAL_STRING_LEN("/blocked", req.path, req.path_len);

    status = ihtp_request_apply_semantics(&req, &policy);
    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_ioguard_style_connect_request_hands_authority_to_consumer(void)
{
    static const char wire[] = "CONNECT vpn.example.com:443 HTTP/1.1\r\n"
                               "Host: vpn.example.com:443\r\n"
                               "Proxy-Connection: keep-alive\r\n"
                               "\r\n";
    ihtp_policy_t policy = IHTP_POLICY_IOGUARD;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    size_t consumed = 0;
    ihtp_status_t status;

    memset(&req, 0, sizeof(req));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);

    status = ihtp_parse_request_stateful(&state, wire, strlen(wire), &req, &policy, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(strlen(wire), consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_CONNECT, req.method);
    TEST_ASSERT_EQUAL_STRING_LEN("vpn.example.com:443", req.path, req.path_len);

    status = ihtp_request_apply_semantics(&req, &policy);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_NONE, req.body_mode);
    TEST_ASSERT_TRUE(req.keep_alive);
    TEST_ASSERT_FALSE(req.protocol_upgrade);
    TEST_ASSERT_FALSE(req.expects_continue);
    TEST_ASSERT_FALSE(req.has_trailer_fields);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_iohttp_style_chunked_pipeline_reuses_parser_state);
    RUN_TEST(test_iohttp_style_expect_continue_leaves_trailers_to_consumer);
    RUN_TEST(test_iohttp_style_response_upgrade_handoff_leaves_protocol_bytes);
    RUN_TEST(test_iohttp_style_fixed_length_response_pipelines_next_response);
    RUN_TEST(test_iohttp_style_eof_response_hands_remaining_bytes_to_consumer);
    RUN_TEST(test_ioguard_style_rejects_ambiguous_te_cl_request);
    RUN_TEST(test_ioguard_style_connect_request_hands_authority_to_consumer);
    return UNITY_END();
}
