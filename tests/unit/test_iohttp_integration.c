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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_iohttp_style_chunked_pipeline_reuses_parser_state);
    return UNITY_END();
}
