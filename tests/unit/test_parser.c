/*
 * iohttpparser — Parser unit tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>

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
    RUN_TEST(test_parse_simple_response);
    RUN_TEST(test_parse_404_response);
    RUN_TEST(test_method_from_str);
    RUN_TEST(test_method_to_str);
    RUN_TEST(test_parse_null_args);
    return UNITY_END();
}
