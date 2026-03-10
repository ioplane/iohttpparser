/*
 * iohttpparser — Body decoder unit tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_body.h>

#include <string.h>

void setUp(void)
{
}
void tearDown(void)
{
}

/* ─── Chunked decoder ─────────────────────────────────────────────────── */

void test_chunked_simple(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5\r\nhello\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_TRUE(status >= 0); /* Complete */
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_multiple(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_TRUE(status >= 0);
    TEST_ASSERT_EQUAL_UINT(11, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello world", buf, 11);
}

void test_chunked_incomplete(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5\r\nhel";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(3, bufsz); /* "hel" decoded so far */
}

void test_chunked_null_args(void)
{
    ihtp_chunked_decoder_t dec = {0};
    size_t bufsz = 0;

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_decode_chunked(nullptr, nullptr, nullptr));
    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_decode_chunked(&dec, nullptr, &bufsz));
}

/* ─── Fixed-length decoder ────────────────────────────────────────────── */

void test_fixed_complete(void)
{
    ihtp_fixed_decoder_t dec;
    ihtp_fixed_decoder_init(&dec, 100);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, ihtp_decode_fixed(&dec, 50));
    TEST_ASSERT_EQUAL_UINT64(50, dec.remaining);
    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_decode_fixed(&dec, 50));
    TEST_ASSERT_EQUAL_UINT64(0, dec.remaining);
}

void test_fixed_overflow(void)
{
    ihtp_fixed_decoder_t dec;
    ihtp_fixed_decoder_init(&dec, 10);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_decode_fixed(&dec, 20));
}

/* ─── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_chunked_simple);
    RUN_TEST(test_chunked_multiple);
    RUN_TEST(test_chunked_incomplete);
    RUN_TEST(test_chunked_null_args);
    RUN_TEST(test_fixed_complete);
    RUN_TEST(test_fixed_overflow);
    return UNITY_END();
}
