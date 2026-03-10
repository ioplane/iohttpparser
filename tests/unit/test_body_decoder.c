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

void test_chunked_incremental_size_crlf_across_buffers(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf1[] = "5\r";
    char buf2[] = "\nhello\r\n0\r\n\r\n";
    size_t bufsz1 = strlen(buf1);
    size_t bufsz2 = strlen(buf2);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf1, &bufsz1);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(0, bufsz1);
    TEST_ASSERT_EQUAL_UINT64(0, dec.total_decoded);

    status = ihtp_decode_chunked(&dec, buf2, &bufsz2);

    TEST_ASSERT_EQUAL_INT(2, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz2);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf2, 5);
    TEST_ASSERT_EQUAL_UINT64(5, dec.total_decoded);
}

void test_chunked_incremental_data_crlf_across_buffers(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf1[] = "5\r\nhello\r";
    char buf2[] = "\n0\r\n\r\n";
    size_t bufsz1 = strlen(buf1);
    size_t bufsz2 = strlen(buf2);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf1, &bufsz1);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz1);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf1, 5);
    TEST_ASSERT_EQUAL_UINT64(5, dec.total_decoded);

    status = ihtp_decode_chunked(&dec, buf2, &bufsz2);

    TEST_ASSERT_EQUAL_INT(2, status);
    TEST_ASSERT_EQUAL_UINT(0, bufsz2);
    TEST_ASSERT_EQUAL_UINT64(5, dec.total_decoded);
}

void test_chunked_returns_trailing_bytes_without_trailer_consumption(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5\r\nhello\r\n0\r\n\r\nXYZ";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(5, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_returns_trailing_bytes_after_trailer_consumption(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf[] = "5\r\nhello\r\n0\r\nX-Test: ok\r\n\r\nXYZ";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(3, status);
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_accepts_chunk_extension(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5;foo=bar\r\nhello\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_TRUE(status >= 0);
    TEST_ASSERT_EQUAL_UINT(5, bufsz);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", buf, 5);
}

void test_chunked_incomplete_chunk_extension(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5;foo=bar";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, status);
    TEST_ASSERT_EQUAL_UINT(0, bufsz);
}

void test_chunked_rejects_bare_lf_in_chunk_extension(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5;foo=bar\nhello\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_chunked_rejects_invalid_hex_digit(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "g\r\nhello\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_chunked_rejects_missing_lf_after_size(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5\rhello\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_chunked_rejects_missing_cr_after_data(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "5\r\nhelloX\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_chunked_rejects_oversized_chunk_size(void)
{
    ihtp_chunked_decoder_t dec = {0};
    char buf[] = "10000000000000000\r\nx\r\n0\r\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_chunked_rejects_bare_lf_in_trailer_line(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf[] = "5\r\nhello\r\n0\r\nX-Test: ok\n\r\n";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
}

void test_chunked_rejects_missing_lf_after_trailer_cr(void)
{
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char buf[] = "5\r\nhello\r\n0\r\nX-Test: ok\rX";
    size_t bufsz = strlen(buf);

    ihtp_status_t status = ihtp_decode_chunked(&dec, buf, &bufsz);

    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, status);
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

void test_fixed_zero_length_body_is_immediately_complete(void)
{
    ihtp_fixed_decoder_t dec;
    ihtp_fixed_decoder_init(&dec, 0);

    TEST_ASSERT_EQUAL_INT(IHTP_OK, ihtp_decode_fixed(&dec, 0));
    TEST_ASSERT_EQUAL_UINT64(0, dec.remaining);
    TEST_ASSERT_EQUAL_UINT64(0, dec.total_decoded);
}

void test_fixed_zero_length_consume_is_noop_while_incomplete(void)
{
    ihtp_fixed_decoder_t dec;
    ihtp_fixed_decoder_init(&dec, 5);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, ihtp_decode_fixed(&dec, 0));
    TEST_ASSERT_EQUAL_UINT64(5, dec.remaining);
    TEST_ASSERT_EQUAL_UINT64(0, dec.total_decoded);
}

void test_fixed_overflow_after_partial_consume_preserves_state(void)
{
    ihtp_fixed_decoder_t dec;
    ihtp_fixed_decoder_init(&dec, 10);

    TEST_ASSERT_EQUAL_INT(IHTP_INCOMPLETE, ihtp_decode_fixed(&dec, 6));
    TEST_ASSERT_EQUAL_INT(IHTP_ERROR, ihtp_decode_fixed(&dec, 5));
    TEST_ASSERT_EQUAL_UINT64(4, dec.remaining);
    TEST_ASSERT_EQUAL_UINT64(6, dec.total_decoded);
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
    RUN_TEST(test_chunked_incremental_size_crlf_across_buffers);
    RUN_TEST(test_chunked_incremental_data_crlf_across_buffers);
    RUN_TEST(test_chunked_returns_trailing_bytes_without_trailer_consumption);
    RUN_TEST(test_chunked_returns_trailing_bytes_after_trailer_consumption);
    RUN_TEST(test_chunked_accepts_chunk_extension);
    RUN_TEST(test_chunked_incomplete_chunk_extension);
    RUN_TEST(test_chunked_rejects_bare_lf_in_chunk_extension);
    RUN_TEST(test_chunked_rejects_invalid_hex_digit);
    RUN_TEST(test_chunked_rejects_missing_lf_after_size);
    RUN_TEST(test_chunked_rejects_missing_cr_after_data);
    RUN_TEST(test_chunked_rejects_oversized_chunk_size);
    RUN_TEST(test_chunked_rejects_bare_lf_in_trailer_line);
    RUN_TEST(test_chunked_rejects_missing_lf_after_trailer_cr);
    RUN_TEST(test_fixed_complete);
    RUN_TEST(test_fixed_overflow);
    RUN_TEST(test_fixed_zero_length_body_is_immediately_complete);
    RUN_TEST(test_fixed_zero_length_consume_is_noop_while_incomplete);
    RUN_TEST(test_fixed_overflow_after_partial_consume_preserves_state);
    return UNITY_END();
}
