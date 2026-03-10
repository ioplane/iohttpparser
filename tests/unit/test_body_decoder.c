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

void test_chunked_consume_empty_trailer(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf[] = "5\r\nhello\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_consume_non_empty_trailer(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf[] = "5\r\nhello\r\n0\r\nX-Test: ok\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_incomplete_trailer(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf[] = "5\r\nhello\r\n0\r\nX-Test: ok\r";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_incremental_across_buffers(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf1[] = "5\r\nhe";
    char buf2[] = "llo\r\n0\r\n\r\n";
    size_t bufsz1 = strlen(buf1);
    size_t bufsz2 = strlen(buf2);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf1, &bufsz1);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(2, bufsz1);
    TEST_ASSERT_EQUAL_STRING_LEN("he", buf1, 2);
    TEST_ASSERT_EQUAL_UINT64(2, dec.total_decoded);

    status = ihtp_decode_chunked(&dec, buf2, &bufsz2);

    TEST_ASSERT_TRUE(status >= 0);
    TEST_ASSERT_EQUAL_UINT(3, bufsz2);
    TEST_ASSERT_EQUAL_STRING_LEN("llo", buf2, 3);
    TEST_ASSERT_EQUAL_UINT64(5, dec.total_decoded);
}

void test_chunked_incremental_trailer_across_buffers(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf1[] = "5\r\nhello\r\n0\r\nX-Test:";
    char buf2[] = " ok\r\n\r\n";
    size_t bufsz1 = strlen(buf1);
    size_t bufsz2 = strlen(buf2);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf1, &bufsz1);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz1);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf1, 5);
    TEST_ASSERT_EQUAL_UINT64(5, dec.total_decoded);

    status = ihtp_decode_chunked(&dec, buf2, &bufsz2);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(0, bufsz2);
    TEST_ASSERT_EQUAL_UINT64(5, dec.total_decoded);
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
    RUN_TEST(test_chunked_consume_empty_trailer);
    RUN_TEST(test_chunked_consume_non_empty_trailer);
    RUN_TEST(test_chunked_incomplete_trailer);
    RUN_TEST(test_chunked_incremental_across_buffers);
    RUN_TEST(test_chunked_incremental_trailer_across_buffers);
    RUN_TEST(test_fixed_complete);
    RUN_TEST(test_fixed_overflow);
    return UNITY_END();
}
