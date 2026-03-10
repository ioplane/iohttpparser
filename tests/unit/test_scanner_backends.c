/*
 * iohttpparser — Scanner backend equivalence tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include "ihtp_internal.h"
#include <iohttpparser/ihtp_scanner.h>

#include <stdint.h>
#include <string.h>

typedef struct {
    const char *name;
    const char *buf;
    size_t len;
    const char *delims;
    size_t expected_offset;
} find_case_t;

typedef struct {
    const char *name;
    const char *buf;
    size_t len;
    bool expected;
} token_case_t;

void setUp(void)
{
}

void tearDown(void)
{
}

static void assert_find_case_eq(const char *backend_name, ihtp_scan_find_fn backend,
                                const find_case_t *test_case)
{
    const char *expected = ihtp_scan_find_char_scalar(test_case->buf, test_case->len,
                                                      test_case->delims);
    const char *actual = backend(test_case->buf, test_case->len, test_case->delims);

    TEST_ASSERT_EQUAL_PTR_MESSAGE(expected, actual, backend_name);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(test_case->buf + test_case->expected_offset, actual, backend_name);
}

static void assert_token_case_eq(const char *backend_name, ihtp_scan_token_fn backend,
                                 const token_case_t *test_case)
{
    bool expected = ihtp_scan_is_token_scalar(test_case->buf, test_case->len);
    bool actual = backend(test_case->buf, test_case->len);

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)test_case->expected, (int)actual, backend_name);
    TEST_ASSERT_EQUAL_INT_MESSAGE((int)expected, (int)actual, backend_name);
}

void test_scanner_scalar_backend_cases(void)
{
    static const char binary_buf[] = {'A', (char)0xFF, 'B', '\0'};
    static const find_case_t find_cases[] = {
        {.name = "empty", .buf = "", .len = 0, .delims = " ", .expected_offset = 0},
        {.name = "request space", .buf = "GET / HTTP/1.1", .len = 14, .delims = " ",
         .expected_offset = 3},
        {.name = "crlf", .buf = "Header: value\r\n", .len = 15, .delims = "\r\n",
         .expected_offset = 13},
        {.name = "multiple delimiters", .buf = "key=value&other", .len = 15, .delims = "=&",
         .expected_offset = 3},
        {.name = "not found", .buf = "token", .len = 5, .delims = ":", .expected_offset = 5},
        {.name = "high byte", .buf = binary_buf, .len = 3, .delims = "\xFF",
         .expected_offset = 1},
        {.name = "sse fallback >16", .buf = "0123456789abcdefq", .len = 17,
         .delims = "abcdefghijklmnopq", .expected_offset = 10},
    };
    static const char token_high_byte[] = {'O', 'K', (char)0xFF, '\0'};
    static const token_case_t token_cases[] = {
        {.name = "empty", .buf = "", .len = 0, .expected = true},
        {.name = "method", .buf = "GET", .len = 3, .expected = true},
        {.name = "header", .buf = "Content-Type", .len = 12, .expected = true},
        {.name = "space", .buf = "bad header", .len = 10, .expected = false},
        {.name = "colon", .buf = "bad:name", .len = 8, .expected = false},
        {.name = "high byte", .buf = token_high_byte, .len = 3, .expected = false},
    };

    for (size_t i = 0; i < sizeof(find_cases) / sizeof(find_cases[0]); i++) {
        const char *actual = ihtp_scan_find_char_scalar(find_cases[i].buf, find_cases[i].len,
                                                        find_cases[i].delims);
        TEST_ASSERT_EQUAL_PTR(find_cases[i].buf + find_cases[i].expected_offset, actual);
    }

    for (size_t i = 0; i < sizeof(token_cases) / sizeof(token_cases[0]); i++) {
        bool actual = ihtp_scan_is_token_scalar(token_cases[i].buf, token_cases[i].len);
        TEST_ASSERT_EQUAL_INT((int)token_cases[i].expected, (int)actual);
    }
}

void test_scanner_sse42_equivalence(void)
{
#ifdef IOHTTPPARSER_HAVE_SSE42
    static const char binary_buf[] = {'A', (char)0xFF, 'B', '\0'};
    static const char token_high_byte[] = {'O', 'K', (char)0xFF, '\0'};
    static const find_case_t find_cases[] = {
        {.name = "empty", .buf = "", .len = 0, .delims = " ", .expected_offset = 0},
        {.name = "request space", .buf = "GET / HTTP/1.1", .len = 14, .delims = " ",
         .expected_offset = 3},
        {.name = "crlf", .buf = "Header: value\r\n", .len = 15, .delims = "\r\n",
         .expected_offset = 13},
        {.name = "multiple delimiters", .buf = "key=value&other", .len = 15, .delims = "=&",
         .expected_offset = 3},
        {.name = "not found", .buf = "token", .len = 5, .delims = ":", .expected_offset = 5},
        {.name = "high byte", .buf = binary_buf, .len = 3, .delims = "\xFF",
         .expected_offset = 1},
        {.name = "sse fallback >16", .buf = "0123456789abcdefq", .len = 17,
         .delims = "abcdefghijklmnopq", .expected_offset = 10},
    };
    static const token_case_t token_cases[] = {
        {.name = "empty", .buf = "", .len = 0, .expected = true},
        {.name = "method", .buf = "GET", .len = 3, .expected = true},
        {.name = "header", .buf = "Content-Type", .len = 12, .expected = true},
        {.name = "space", .buf = "bad header", .len = 10, .expected = false},
        {.name = "colon", .buf = "bad:name", .len = 8, .expected = false},
        {.name = "high byte", .buf = token_high_byte, .len = 3, .expected = false},
    };

    if ((ihtp_scanner_simd_level() & 0x01) == 0) {
        return;
    }

    for (size_t i = 0; i < sizeof(find_cases) / sizeof(find_cases[0]); i++) {
        assert_find_case_eq("sse42", ihtp_scan_find_char_sse42, &find_cases[i]);
    }

    for (size_t i = 0; i < sizeof(token_cases) / sizeof(token_cases[0]); i++) {
        assert_token_case_eq("sse42", ihtp_scan_is_token_sse42, &token_cases[i]);
    }
#endif
}

void test_scanner_avx2_equivalence(void)
{
#ifdef IOHTTPPARSER_HAVE_AVX2
    static const char binary_buf[] = {'A', (char)0xFF, 'B', '\0'};
    static const char token_high_byte[] = {'O', 'K', (char)0xFF, '\0'};
    static const find_case_t find_cases[] = {
        {.name = "empty", .buf = "", .len = 0, .delims = " ", .expected_offset = 0},
        {.name = "request space", .buf = "GET / HTTP/1.1", .len = 14, .delims = " ",
         .expected_offset = 3},
        {.name = "crlf", .buf = "Header: value\r\n", .len = 15, .delims = "\r\n",
         .expected_offset = 13},
        {.name = "multiple delimiters", .buf = "key=value&other", .len = 15, .delims = "=&",
         .expected_offset = 3},
        {.name = "not found", .buf = "token", .len = 5, .delims = ":", .expected_offset = 5},
        {.name = "high byte", .buf = binary_buf, .len = 3, .delims = "\xFF",
         .expected_offset = 1},
        {.name = "wide block", .buf = "0123456789abcdefghijklmnopqrstuvwx?", .len = 35,
         .delims = "?", .expected_offset = 34},
    };
    static const token_case_t token_cases[] = {
        {.name = "empty", .buf = "", .len = 0, .expected = true},
        {.name = "method", .buf = "GET", .len = 3, .expected = true},
        {.name = "header", .buf = "Content-Type", .len = 12, .expected = true},
        {.name = "space", .buf = "bad header", .len = 10, .expected = false},
        {.name = "colon", .buf = "bad:name", .len = 8, .expected = false},
        {.name = "high byte", .buf = token_high_byte, .len = 3, .expected = false},
    };

    if ((ihtp_scanner_simd_level() & 0x02) == 0) {
        return;
    }

    for (size_t i = 0; i < sizeof(find_cases) / sizeof(find_cases[0]); i++) {
        assert_find_case_eq("avx2", ihtp_scan_find_char_avx2, &find_cases[i]);
    }

    for (size_t i = 0; i < sizeof(token_cases) / sizeof(token_cases[0]); i++) {
        assert_token_case_eq("avx2", ihtp_scan_is_token_avx2, &token_cases[i]);
    }
#endif
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scanner_scalar_backend_cases);
    RUN_TEST(test_scanner_sse42_equivalence);
    RUN_TEST(test_scanner_avx2_equivalence);
    return UNITY_END();
}
