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

static ihtp_status_t parse_req_with_semantics_policy(const char *raw, ihtp_request_t *req,
                                                     const ihtp_policy_t *policy)
{
    size_t consumed = 0;
    ihtp_status_t s = ihtp_parse_request(raw, strlen(raw), req, policy, &consumed);
    if (s != IHTP_OK) {
        return s;
    }
    return ihtp_request_apply_semantics(req, policy);
}

static ihtp_status_t parse_resp_with_semantics_policy(const char *raw, ihtp_response_t *resp,
                                                      const ihtp_policy_t *policy)
{
    size_t consumed = 0;
    ihtp_status_t s = ihtp_parse_response(raw, strlen(raw), resp, policy, &consumed);
    if (s != IHTP_OK) {
        return s;
    }
    return ihtp_response_apply_semantics(resp, policy);
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

void test_semantics_request_rejects_http11_without_host(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\n\r\n", &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_request_allows_http10_without_host(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.0\r\n\r\n", &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
}

void test_semantics_request_rejects_duplicate_host(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Host: duplicate.example\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_request_rejects_empty_host(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\n"
                                               "Host:\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
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

void test_semantics_request_accepts_transfer_encoding_chain_ending_in_chunked(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Transfer-Encoding: gzip, chunked\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, req.body_mode);
}

void test_semantics_request_accepts_case_insensitive_chunked(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Transfer-Encoding: ChUnKeD\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, req.body_mode);
}

void test_semantics_request_rejects_transfer_encoding_not_ending_in_chunked(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Transfer-Encoding: gzip\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_request_rejects_malformed_transfer_encoding_list(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Transfer-Encoding: chunked,\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_request_rejects_te_cl_in_strict_mode(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Transfer-Encoding: chunked\r\n"
                                               "Content-Length: 42\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_request_allows_te_cl_in_lenient_mode(void)
{
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics_policy("POST /data HTTP/1.1\r\n"
                                                      "Host: example.com\r\n"
                                                      "Transfer-Encoding: chunked\r\n"
                                                      "Content-Length: 42\r\n"
                                                      "\r\n",
                                                      &req, &policy);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, req.body_mode);
}

void test_semantics_request_rejects_conflicting_content_length(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Content-Length: 42\r\n"
                                               "Content-Length: 43\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_request_accepts_identical_duplicate_content_length(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("POST /data HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Content-Length: 42\r\n"
                                               "Content-Length: 42\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_FIXED, req.body_mode);
    TEST_ASSERT_EQUAL_UINT64(42, req.content_length);
}

void test_semantics_response_rejects_te_cl_in_strict_mode(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Transfer-Encoding: chunked\r\n"
                                                       "Content-Length: 42\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_response_allows_te_cl_in_lenient_mode(void)
{
    const ihtp_policy_t policy = IHTP_POLICY_LENIENT;
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Transfer-Encoding: chunked\r\n"
                                                       "Content-Length: 42\r\n"
                                                       "\r\n",
                                                       &resp, &policy);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, resp.body_mode);
}

void test_semantics_response_uses_eof_for_transfer_encoding_not_ending_in_chunked(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Transfer-Encoding: gzip\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_EOF, resp.body_mode);
}

void test_semantics_response_accepts_case_insensitive_chunked(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Transfer-Encoding: ChUnKeD\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_CHUNKED, resp.body_mode);
}

void test_semantics_response_rejects_malformed_transfer_encoding_list(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Transfer-Encoding: gzip,,chunked\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_response_rejects_conflicting_content_length(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Content-Length: 42\r\n"
                                                       "Content-Length: 43\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, s);
}

void test_semantics_response_accepts_identical_duplicate_content_length(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Content-Length: 42\r\n"
                                                       "Content-Length: 42\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_EQUAL_INT(IHTP_BODY_FIXED, resp.body_mode);
    TEST_ASSERT_EQUAL_UINT64(42, resp.content_length);
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

void test_semantics_request_connection_close_overrides_http11_default(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Connection: close\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_FALSE(req.keep_alive);
}

void test_semantics_request_connection_keep_alive_overrides_http10_default(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.0\r\n"
                                               "Connection: keep-alive\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_TRUE(req.keep_alive);
}

void test_semantics_request_connection_token_list_close_wins(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.1\r\n"
                                               "Host: example.com\r\n"
                                               "Connection: keep-alive, upgrade, close\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_FALSE(req.keep_alive);
}

void test_semantics_request_connection_is_case_insensitive(void)
{
    ihtp_request_t req;
    ihtp_status_t s = parse_req_with_semantics("GET / HTTP/1.0\r\n"
                                               "Connection: KeEp-AlIvE\r\n"
                                               "\r\n",
                                               &req);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_TRUE(req.keep_alive);
}

void test_semantics_response_connection_close_overrides_http11_default(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Connection: close\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_FALSE(resp.keep_alive);
}

void test_semantics_response_connection_keep_alive_preserved_for_http10(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.0 200 OK\r\n"
                                                       "Connection: keep-alive\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_TRUE(resp.keep_alive);
}

void test_semantics_response_connection_token_list_close_wins(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.1 200 OK\r\n"
                                                       "Connection: keep-alive, close\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_FALSE(resp.keep_alive);
}

void test_semantics_response_connection_is_case_insensitive(void)
{
    ihtp_response_t resp;
    ihtp_status_t s = parse_resp_with_semantics_policy("HTTP/1.0 200 OK\r\n"
                                                       "Connection: ClOsE\r\n"
                                                       "\r\n",
                                                       &resp, nullptr);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, s);
    TEST_ASSERT_FALSE(resp.keep_alive);
}

/* ─── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_semantics_no_body);
    RUN_TEST(test_semantics_content_length);
    RUN_TEST(test_semantics_request_rejects_http11_without_host);
    RUN_TEST(test_semantics_request_allows_http10_without_host);
    RUN_TEST(test_semantics_request_rejects_duplicate_host);
    RUN_TEST(test_semantics_request_rejects_empty_host);
    RUN_TEST(test_semantics_chunked);
    RUN_TEST(test_semantics_request_accepts_transfer_encoding_chain_ending_in_chunked);
    RUN_TEST(test_semantics_request_accepts_case_insensitive_chunked);
    RUN_TEST(test_semantics_request_rejects_transfer_encoding_not_ending_in_chunked);
    RUN_TEST(test_semantics_request_rejects_malformed_transfer_encoding_list);
    RUN_TEST(test_semantics_request_rejects_te_cl_in_strict_mode);
    RUN_TEST(test_semantics_request_allows_te_cl_in_lenient_mode);
    RUN_TEST(test_semantics_request_rejects_conflicting_content_length);
    RUN_TEST(test_semantics_request_accepts_identical_duplicate_content_length);
    RUN_TEST(test_semantics_response_rejects_te_cl_in_strict_mode);
    RUN_TEST(test_semantics_response_allows_te_cl_in_lenient_mode);
    RUN_TEST(test_semantics_response_uses_eof_for_transfer_encoding_not_ending_in_chunked);
    RUN_TEST(test_semantics_response_accepts_case_insensitive_chunked);
    RUN_TEST(test_semantics_response_rejects_malformed_transfer_encoding_list);
    RUN_TEST(test_semantics_response_rejects_conflicting_content_length);
    RUN_TEST(test_semantics_response_accepts_identical_duplicate_content_length);
    RUN_TEST(test_semantics_keepalive_http11);
    RUN_TEST(test_semantics_keepalive_http10);
    RUN_TEST(test_semantics_request_connection_close_overrides_http11_default);
    RUN_TEST(test_semantics_request_connection_keep_alive_overrides_http10_default);
    RUN_TEST(test_semantics_request_connection_token_list_close_wins);
    RUN_TEST(test_semantics_request_connection_is_case_insensitive);
    RUN_TEST(test_semantics_response_connection_close_overrides_http11_default);
    RUN_TEST(test_semantics_response_connection_keep_alive_preserved_for_http10);
    RUN_TEST(test_semantics_response_connection_token_list_close_wins);
    RUN_TEST(test_semantics_response_connection_is_case_insensitive);
    return UNITY_END();
}
