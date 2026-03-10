/*
 * iohttpparser — Internal parser-state tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>
#include "ihtp_internal.h"

#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_request_state_advances_from_start_line_to_headers(void)
{
    static const char wire[] = "GET /alpha HTTP/1.1\r\nHost: example.com\r\n\r\n";
    ihtp_request_t req;
    ihtp_parser_state_t state;
    size_t consumed = 99;
    ihtp_status_t status;

    memset(&req, 0, sizeof(req));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);

    status = ihtp_parse_request_stateful(&state, wire, 28, &req, nullptr, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_HEADERS, state.phase);
    TEST_ASSERT_EQUAL_UINT(21, state.cursor);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_GET, req.method);
    TEST_ASSERT_EQUAL_STRING_LEN("/alpha", req.path, req.path_len);
    TEST_ASSERT_EQUAL_UINT(0, req.num_headers);

    status = ihtp_parse_request_stateful(&state, wire, strlen(wire), &req, nullptr, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_DONE, state.phase);
    TEST_ASSERT_EQUAL_UINT(strlen(wire), consumed);
    TEST_ASSERT_EQUAL_UINT(1, req.num_headers);
    TEST_ASSERT_EQUAL_STRING_LEN("Host", req.headers[0].name, req.headers[0].name_len);
    TEST_ASSERT_EQUAL_STRING_LEN("example.com", req.headers[0].value, req.headers[0].value_len);
}

void test_response_state_advances_from_status_line_to_headers(void)
{
    static const char wire[] = "HTTP/1.1 204 OK\r\nX-Test: yes\r\n\r\n";
    ihtp_response_t resp;
    ihtp_parser_state_t state;
    size_t consumed = 77;
    ihtp_status_t status;

    memset(&resp, 0, sizeof(resp));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);

    status = ihtp_parse_response_stateful(&state, wire, 26, &resp, nullptr, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_HEADERS, state.phase);
    TEST_ASSERT_EQUAL_UINT(17, state.cursor);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
    TEST_ASSERT_EQUAL_INT(204, resp.status_code);
    TEST_ASSERT_EQUAL_UINT(0, resp.num_headers);

    status = ihtp_parse_response_stateful(&state, wire, strlen(wire), &resp, nullptr, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_DONE, state.phase);
    TEST_ASSERT_EQUAL_UINT(strlen(wire), consumed);
    TEST_ASSERT_EQUAL_UINT(1, resp.num_headers);
    TEST_ASSERT_EQUAL_STRING_LEN("yes", resp.headers[0].value, resp.headers[0].value_len);
}

void test_headers_state_preserves_progress_across_calls(void)
{
    static const char wire[] = "Host: example.com\r\nX-Test: yes\r\n\r\n";
    ihtp_header_t headers[4];
    ihtp_parser_state_t state;
    size_t num_headers = 0;
    size_t consumed = 55;
    ihtp_status_t status;

    memset(headers, 0, sizeof(headers));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_HEADERS);

    status =
        ihtp_parse_headers_stateful(&state, wire, 22, headers, &num_headers, 4, nullptr, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_HEADERS, state.phase);
    TEST_ASSERT_EQUAL_UINT(19, state.cursor);
    TEST_ASSERT_EQUAL_UINT(1, num_headers);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
    TEST_ASSERT_EQUAL_STRING_LEN("Host", headers[0].name, headers[0].name_len);

    status = ihtp_parse_headers_stateful(&state, wire, strlen(wire), headers, &num_headers, 4,
                                         nullptr, &consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_DONE, state.phase);
    TEST_ASSERT_EQUAL_UINT(strlen(wire), consumed);
    TEST_ASSERT_EQUAL_UINT(2, num_headers);
    TEST_ASSERT_EQUAL_STRING_LEN("X-Test", headers[1].name, headers[1].name_len);
    TEST_ASSERT_EQUAL_STRING_LEN("yes", headers[1].value, headers[1].value_len);
}

void test_request_state_latches_error_phase(void)
{
    static const char wire[] = "GET / HTTP/1.1\r\nHost : example.com\r\n\r\n";
    ihtp_request_t req;
    ihtp_parser_state_t state;
    size_t consumed = 11;

    memset(&req, 0, sizeof(req));
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_parse_request_stateful(&state, wire, strlen(wire), &req,
                                                                  nullptr, &consumed));
    TEST_ASSERT_EQUAL_INT(IHTP_PARSER_PHASE_ERROR, state.phase);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_parse_request_stateful(&state, wire, strlen(wire), &req,
                                                                  nullptr, &consumed));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_request_state_advances_from_start_line_to_headers);
    RUN_TEST(test_response_state_advances_from_status_line_to_headers);
    RUN_TEST(test_headers_state_preserves_progress_across_calls);
    RUN_TEST(test_request_state_latches_error_phase);
    return UNITY_END();
}
