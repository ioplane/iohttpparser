/*
 * iohttpparser — Parser unit tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>

#include <stdio.h>
#include <string.h>

void setUp(void)
{
}
void tearDown(void)
{
}

/* ─── Request parsing ─────────────────────────────────────────────────── */

void test_parse_simple_get(void)
{
    const char *req_str = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_GET, req.method);
    TEST_ASSERT_EQUAL_UINT(1, req.path_len);
    TEST_ASSERT_EQUAL_STRING_LEN("/", req.path, 1);
    TEST_ASSERT_EQUAL_INT(IHTP_HTTP_11, req.version);
    TEST_ASSERT_EQUAL_UINT(1, req.num_headers);
    TEST_ASSERT_EQUAL_UINT(strlen(req_str), consumed);
}

void test_parse_post_with_headers(void)
{
    const char *req_str = "POST /api/data HTTP/1.1\r\n"
                          "Host: example.com\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 13\r\n"
                          "\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_POST, req.method);
    TEST_ASSERT_EQUAL_UINT(3, req.num_headers);
}

void test_parse_incomplete_request(void)
{
    const char *req_str = "GET / HTTP/1.1\r\nHost: exam";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
}

void test_parse_malformed_method(void)
{
    const char *req_str = "G E T / HTTP/1.1\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_http10(void)
{
    const char *req_str = "GET / HTTP/1.0\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(IHTP_HTTP_10, req.version);
}

void test_parse_invalid_version(void)
{
    const char *req_str = "GET / HTTP/2.0\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_leading_lws_before_first_header(void)
{
    const char *req_str = "GET / HTTP/1.1\r\n Host: example.com\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_empty_header_name(void)
{
    const char *req_str = "GET / HTTP/1.1\r\n: example.com\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_whitespace_before_header_colon(void)
{
    const char *req_str = "GET / HTTP/1.1\r\nHost : example.com\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_tab_in_request_target(void)
{
    const char req_str[] = "GET /\t HTTP/1.1\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_del_in_request_target(void)
{
    const char req_str[] = "GET /\x7f HTTP/1.1\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_space_in_request_target_in_strict_mode(void)
{
    const char *req_str = "GET /with space HTTP/1.1\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_accepts_space_in_request_target_in_lenient_mode(void)
{
    const char *req_str = "GET /with space HTTP/1.1\r\n\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(11, req.path_len);
    TEST_ASSERT_EQUAL_STRING_LEN("/with space", req.path, req.path_len);
    TEST_ASSERT_EQUAL_UINT(strlen(req_str), consumed);
}

void test_parse_lenient_mode_still_rejects_tab_in_request_target(void)
{
    const char req_str[] = "GET /\t HTTP/1.1\r\n\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_lenient_mode_still_rejects_del_in_request_target(void)
{
    const char req_str[] = "GET /\x7f HTTP/1.1\r\n\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_bare_lf_in_strict_mode(void)
{
    const char *req_str = "GET / HTTP/1.1\nHost: example.com\n\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_accepts_bare_lf_in_lenient_mode(void)
{
    const char *req_str = "GET / HTTP/1.1\nHost: example.com\n\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(1, req.num_headers);
    TEST_ASSERT_EQUAL_UINT(strlen(req_str), consumed);
}

void test_parse_rejects_nul_in_header_value(void)
{
    const char req_str[] = "GET / HTTP/1.1\r\nHost: ex\000ample.com\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_rejects_ctl_in_header_value(void)
{
    const char req_str[] = "GET / HTTP/1.1\r\nHost: ex\vample.com\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_accepts_htab_in_header_value(void)
{
    const char req_str[] = "GET / HTTP/1.1\r\nX-Test: alpha\tbeta\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(1, req.num_headers);
}

void test_parse_rejects_obs_fold_in_strict_mode(void)
{
    const char *req_str = "GET / HTTP/1.1\r\nX-Test: one\r\n two\r\n\r\n";
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_accepts_obs_fold_in_lenient_mode(void)
{
    const char *req_str = "GET / HTTP/1.1\r\nX-Test: one\r\n two\r\n\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(1, req.num_headers);
    TEST_ASSERT_EQUAL_UINT(9, req.headers[0].value_len);
    TEST_ASSERT_EQUAL_STRING_LEN("one\r\n two", req.headers[0].value, req.headers[0].value_len);
}

void test_parse_lenient_obs_fold_still_rejects_ctl_in_continuation(void)
{
    const char req_str[] = "GET / HTTP/1.1\r\nX-Test: one\r\n \vtwo\r\n\r\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_request_resets_consumed_on_incomplete_input(void)
{
    const char *req_str = "GET / HTTP/1.1\r\nHost: exam";
    ihtp_request_t req;
    size_t consumed = 99;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

void test_parse_request_resets_consumed_on_error(void)
{
    const char *req_str = "GET / HTTP/1.1\nHost: example.com\n\n";
    ihtp_request_t req;
    size_t consumed = 99;

    ihtp_status_t status = ihtp_parse_request(req_str, strlen(req_str), &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

void test_parse_rejects_request_line_over_limit(void)
{
    char req_str[IHTP_MAX_REQUEST_LINE + 4];
    ihtp_request_t req;
    size_t consumed = 0;

    memset(req_str, 'A', sizeof(req_str));
    req_str[IHTP_MAX_REQUEST_LINE + 1] = '\r';
    req_str[IHTP_MAX_REQUEST_LINE + 2] = '\n';
    req_str[IHTP_MAX_REQUEST_LINE + 3] = '\0';

    ihtp_status_t status =
        ihtp_parse_request(req_str, sizeof(req_str) - 1, &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR_TOO_LONG, status);
}

void test_parse_rejects_header_line_over_limit(void)
{
    char req_str[32 + IHTP_MAX_HEADER_LINE + 16];
    const char *prefix = "GET / HTTP/1.1\r\nX:";
    size_t prefix_len = strlen(prefix);
    ihtp_request_t req;
    size_t consumed = 0;

    memcpy(req_str, prefix, prefix_len);
    memset(req_str + prefix_len, 'A', IHTP_MAX_HEADER_LINE + 1);
    req_str[prefix_len + IHTP_MAX_HEADER_LINE + 1] = '\r';
    req_str[prefix_len + IHTP_MAX_HEADER_LINE + 2] = '\n';
    req_str[prefix_len + IHTP_MAX_HEADER_LINE + 3] = '\r';
    req_str[prefix_len + IHTP_MAX_HEADER_LINE + 4] = '\n';
    req_str[prefix_len + IHTP_MAX_HEADER_LINE + 5] = '\0';

    ihtp_status_t status = ihtp_parse_request(req_str, prefix_len + IHTP_MAX_HEADER_LINE + 5, &req,
                                              nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR_TOO_LONG, status);
}

/* ─── Response parsing ────────────────────────────────────────────────── */

void test_parse_simple_response(void)
{
    const char *resp_str = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(200, resp.status_code);
    TEST_ASSERT_EQUAL_INT(IHTP_HTTP_11, resp.version);
}

void test_parse_404_response(void)
{
    const char *resp_str = "HTTP/1.1 404 Not Found\r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(404, resp.status_code);
}

void test_parse_response_with_empty_reason_phrase(void)
{
    const char *resp_str = "HTTP/1.1 204 \r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_INT(204, resp.status_code);
    TEST_ASSERT_EQUAL_UINT(0, resp.reason_len);
}

void test_parse_response_rejects_missing_reason_separator(void)
{
    const char *resp_str = "HTTP/1.1 204\r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_response_rejects_bare_lf_in_strict_mode(void)
{
    const char *resp_str = "HTTP/1.1 204 OK\n\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_response_accepts_bare_lf_in_lenient_mode(void)
{
    const char *resp_str = "HTTP/1.1 204 OK\nX-Test: yes\n\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(1, resp.num_headers);
    TEST_ASSERT_EQUAL_UINT(strlen(resp_str), consumed);
}

void test_parse_response_rejects_status_code_below_100(void)
{
    const char *resp_str = "HTTP/1.1 099 Low\r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_response_rejects_status_code_above_599(void)
{
    const char *resp_str = "HTTP/1.1 600 High\r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_response_rejects_ctl_in_reason_phrase(void)
{
    const char resp_str[] = "HTTP/1.1 200 O\vK\r\n\r\n";
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, sizeof(resp_str) - 1, &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_parse_response_resets_consumed_on_incomplete_input(void)
{
    const char *resp_str = "HTTP/1.1 200 OK\r\nContent-Length: 0";
    ihtp_response_t resp;
    size_t consumed = 99;

    ihtp_status_t status =
        ihtp_parse_response(resp_str, strlen(resp_str), &resp, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

void test_parse_headers_resets_consumed_on_incomplete_input(void)
{
    const char *hdr_str = "Host: example.com\r\nContent-Length: 12";
    ihtp_header_t headers[4];
    size_t num_headers = 4;
    size_t consumed = 99;

    ihtp_status_t status =
        ihtp_parse_headers(hdr_str, strlen(hdr_str), headers, &num_headers, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

void test_parse_headers_rejects_too_many_headers(void)
{
    const char *hdr_str = "A: 1\r\nB: 2\r\n\r\n";
    ihtp_header_t headers[1];
    size_t num_headers = 1;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_headers(hdr_str, strlen(hdr_str), headers, &num_headers, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR_TOO_MANY_HEADERS, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

void test_parse_headers_rejects_bare_lf_in_strict_mode(void)
{
    const char *hdr_str = "Host: example.com\n\n";
    ihtp_header_t headers[4];
    size_t num_headers = 4;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_headers(hdr_str, strlen(hdr_str), headers, &num_headers, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

void test_parse_headers_accepts_bare_lf_in_lenient_mode(void)
{
    const char *hdr_str = "Host: example.com\nX-Test: yes\n\n";
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_header_t headers[4];
    size_t num_headers = 4;
    size_t consumed = 0;

    ihtp_status_t status =
        ihtp_parse_headers(hdr_str, strlen(hdr_str), headers, &num_headers, &policy, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, status);
    TEST_ASSERT_EQUAL_UINT(2, num_headers);
    TEST_ASSERT_EQUAL_UINT(strlen(hdr_str), consumed);
}

void test_parse_request_rejects_too_many_headers(void)
{
    char req_str[4096];
    size_t pos = 0;
    ihtp_request_t req;
    size_t consumed = 0;

    pos += (size_t)snprintf(req_str + pos, sizeof(req_str) - pos, "GET / HTTP/1.1\r\n");
    for (size_t i = 0; i < IHTP_MAX_HEADERS + 1; i++) {
        pos += (size_t)snprintf(req_str + pos, sizeof(req_str) - pos, "X-%zu: 1\r\n", i);
    }
    pos += (size_t)snprintf(req_str + pos, sizeof(req_str) - pos, "\r\n");

    ihtp_status_t status = ihtp_parse_request(req_str, pos, &req, nullptr, &consumed);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR_TOO_MANY_HEADERS, status);
    TEST_ASSERT_EQUAL_UINT(0, consumed);
}

/* ─── Method helpers ──────────────────────────────────────────────────── */

void test_method_from_str(void)
{
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_GET, ihtp_method_from_str("GET", 3));
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_POST, ihtp_method_from_str("POST", 4));
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_DELETE, ihtp_method_from_str("DELETE", 6));
    TEST_ASSERT_EQUAL_INT(IHTP_METHOD_UNKNOWN, ihtp_method_from_str("CUSTOM", 6));
}

void test_method_to_str(void)
{
    TEST_ASSERT_EQUAL_STRING("GET", ihtp_method_to_str(IHTP_METHOD_GET));
    TEST_ASSERT_EQUAL_STRING("POST", ihtp_method_to_str(IHTP_METHOD_POST));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", ihtp_method_to_str(IHTP_METHOD_UNKNOWN));
}

/* ─── Null safety ─────────────────────────────────────────────────────── */

void test_parse_null_args(void)
{
    ihtp_request_t req;
    size_t consumed = 0;

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_parse_request(nullptr, 0, &req, nullptr, &consumed));
    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_parse_request("GET", 3, nullptr, nullptr, &consumed));
}

/* ─── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_simple_get);
    RUN_TEST(test_parse_post_with_headers);
    RUN_TEST(test_parse_incomplete_request);
    RUN_TEST(test_parse_malformed_method);
    RUN_TEST(test_parse_http10);
    RUN_TEST(test_parse_invalid_version);
    RUN_TEST(test_parse_rejects_leading_lws_before_first_header);
    RUN_TEST(test_parse_rejects_empty_header_name);
    RUN_TEST(test_parse_rejects_whitespace_before_header_colon);
    RUN_TEST(test_parse_rejects_tab_in_request_target);
    RUN_TEST(test_parse_rejects_del_in_request_target);
    RUN_TEST(test_parse_rejects_space_in_request_target_in_strict_mode);
    RUN_TEST(test_parse_accepts_space_in_request_target_in_lenient_mode);
    RUN_TEST(test_parse_lenient_mode_still_rejects_tab_in_request_target);
    RUN_TEST(test_parse_lenient_mode_still_rejects_del_in_request_target);
    RUN_TEST(test_parse_rejects_bare_lf_in_strict_mode);
    RUN_TEST(test_parse_accepts_bare_lf_in_lenient_mode);
    RUN_TEST(test_parse_rejects_nul_in_header_value);
    RUN_TEST(test_parse_rejects_ctl_in_header_value);
    RUN_TEST(test_parse_accepts_htab_in_header_value);
    RUN_TEST(test_parse_rejects_obs_fold_in_strict_mode);
    RUN_TEST(test_parse_accepts_obs_fold_in_lenient_mode);
    RUN_TEST(test_parse_lenient_obs_fold_still_rejects_ctl_in_continuation);
    RUN_TEST(test_parse_request_resets_consumed_on_incomplete_input);
    RUN_TEST(test_parse_request_resets_consumed_on_error);
    RUN_TEST(test_parse_rejects_request_line_over_limit);
    RUN_TEST(test_parse_rejects_header_line_over_limit);
    RUN_TEST(test_parse_simple_response);
    RUN_TEST(test_parse_404_response);
    RUN_TEST(test_parse_response_with_empty_reason_phrase);
    RUN_TEST(test_parse_response_rejects_missing_reason_separator);
    RUN_TEST(test_parse_response_rejects_bare_lf_in_strict_mode);
    RUN_TEST(test_parse_response_accepts_bare_lf_in_lenient_mode);
    RUN_TEST(test_parse_response_rejects_status_code_below_100);
    RUN_TEST(test_parse_response_rejects_status_code_above_599);
    RUN_TEST(test_parse_response_rejects_ctl_in_reason_phrase);
    RUN_TEST(test_parse_response_resets_consumed_on_incomplete_input);
    RUN_TEST(test_parse_headers_resets_consumed_on_incomplete_input);
    RUN_TEST(test_parse_headers_rejects_too_many_headers);
    RUN_TEST(test_parse_headers_rejects_bare_lf_in_strict_mode);
    RUN_TEST(test_parse_headers_accepts_bare_lf_in_lenient_mode);
    RUN_TEST(test_parse_request_rejects_too_many_headers);
    RUN_TEST(test_method_from_str);
    RUN_TEST(test_method_to_str);
    RUN_TEST(test_parse_null_args);
    return UNITY_END();
}
