/*
 * iohttpparser — Scanner unit tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_scanner.h>

#include <string.h>

void setUp(void)
{
}
void tearDown(void)
{
}

/* ─── ihtp_scan_find_char ───────────────────────────────────────────────── */

void test_scanner_find_char_basic(void)
{
    const char *buf = "GET /path HTTP/1.1\r\n";
    const char *result = ihtp_scan_find_char(buf, strlen(buf), " ");
    TEST_ASSERT_EQUAL_PTR(buf + 3, result);
}

void test_scanner_find_char_not_found(void)
{
    const char *buf = "GETPATH";
    const char *result = ihtp_scan_find_char(buf, strlen(buf), ":");
    TEST_ASSERT_EQUAL_PTR(buf + strlen(buf), result);
}

void test_scanner_find_char_crlf(void)
{
    const char *buf = "Header: value\r\n";
    const char *result = ihtp_scan_find_char(buf, strlen(buf), "\r\n");
    TEST_ASSERT_EQUAL_PTR(buf + 13, result);
}

void test_scanner_find_char_multiple_delims(void)
{
    const char *buf = "key=value&other";
    const char *result = ihtp_scan_find_char(buf, strlen(buf), "=&");
    TEST_ASSERT_EQUAL_PTR(buf + 3, result);
}

/* ─── ihtp_scan_is_token ────────────────────────────────────────────────── */

void test_scanner_is_token_valid(void)
{
    TEST_ASSERT_TRUE(ihtp_scan_is_token("GET", 3));
    TEST_ASSERT_TRUE(ihtp_scan_is_token("Content-Type", 12));
    TEST_ASSERT_TRUE(ihtp_scan_is_token("X-Custom-Header", 15));
}

void test_scanner_is_token_invalid(void)
{
    TEST_ASSERT_FALSE(ihtp_scan_is_token("bad header", 10));
    TEST_ASSERT_FALSE(ihtp_scan_is_token("bad:name", 8));
    TEST_ASSERT_FALSE(ihtp_scan_is_token("bad\0name", 8));
}

/* ─── ihtp_scan_skip_lws ────────────────────────────────────────────────── */

void test_scanner_skip_lws(void)
{
    TEST_ASSERT_EQUAL_UINT(3, ihtp_scan_skip_lws("   hello", 8));
    TEST_ASSERT_EQUAL_UINT(0, ihtp_scan_skip_lws("hello", 5));
    TEST_ASSERT_EQUAL_UINT(2, ihtp_scan_skip_lws("\t hello", 7));
}

/* ─── SIMD level ──────────────────────────────────────────────────────── */

void test_scanner_simd_level(void)
{
    int level = ihtp_scanner_simd_level();
    /* Just verify it returns something reasonable */
    TEST_ASSERT_TRUE(level >= 0);
}

/* ─── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scanner_find_char_basic);
    RUN_TEST(test_scanner_find_char_not_found);
    RUN_TEST(test_scanner_find_char_crlf);
    RUN_TEST(test_scanner_find_char_multiple_delims);
    RUN_TEST(test_scanner_is_token_valid);
    RUN_TEST(test_scanner_is_token_invalid);
    RUN_TEST(test_scanner_skip_lws);
    RUN_TEST(test_scanner_simd_level);
    return UNITY_END();
}
