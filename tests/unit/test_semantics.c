/*
 * iohttpparser — Semantics unit tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>

#include <string.h>

/* Forward declaration — semantics functions from ihtp_semantics.c */
extern ihtp_status_t ihtp_request_apply_semantics(ihtp_request_t *req, const ihtp_policy_t *policy);
extern ihtp_status_t ihtp_response_apply_semantics(ihtp_response_t *resp,
                                                   const ihtp_policy_t *policy);

void setUp(void)
{
}
void tearDown(void)
{
}

/* ─── Helper: parse + apply semantics ─────────────────────────────────── */

static ihtp_status_t parse_req_with_semantics(const char *raw, ihtp_request_t *req)
{
    size_t consumed = 0;
    ihtp_status_t s = ihtp_parse_request(raw, strlen(raw), req, nullptr, &consumed);
    if (s != IHTP_OK) {
        return s;
    }
    return ihtp_request_apply_semantics(req, nullptr);
}

/* ─── Body mode detection ─────────────────────────────────────────────── */

void test_semantics_no_body(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n", &req);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_NONE, req.body_mode);
}

void test_semantics_content_length(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Content-Length: 42\r\n"
                                               "\r\n",
                                               &req);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_FIXED, req.body_mode);
    TEST_ASSERT_EQUAL_UINT64(42, req.content_length);
}

void test_semantics_chunked(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Transfer-Encoding: chunked\r\n"
                                               "\r\n",
                                               &req);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, req.body_mode);
}

/* ─── Keep-alive ──────────────────────────────────────────────────────── */

void test_semantics_keepalive_http11(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n", &req);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_TRUE(req.keep_alive);
}

void test_semantics_keepalive_http10(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.0\r\n\r\n", &req);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_FALSE(req.keep_alive);
}

/* ─── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_semantics_no_body);
    RUN_TEST(test_semantics_content_length);
    RUN_TEST(test_semantics_chunked);
    RUN_TEST(test_semantics_keepalive_http11);
    RUN_TEST(test_semantics_keepalive_http10);
    return UNITY_END();
}
