/*
 * iohttpparser — Body decoder corpus tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_body.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    CORPUS_KIND_CHUNKED,
    CORPUS_KIND_FIXED,
} corpus_kind_t;

typedef enum {
    CORPUS_EXPECT_OK,
    CORPUS_EXPECT_ERROR,
    CORPUS_EXPECT_INCOMPLETE,
} corpus_expect_t;

static const char *kChunked = "chunked";
static const char *kFixed = "fixed";
static const char *kOk = "ok";
static const char *kError = "error";
static const char *kIncomplete = "incomplete";
static const char *kTrue = "true";
static const char *kFalse = "false";

void setUp(void)
{
}

void tearDown(void)
{
}

static bool str_eq(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

static void trim_trailing_newline(char *line)
{
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

static size_t decode_wire(const char *src, char *dst, size_t dst_size)
{
    size_t out = 0;

    for (size_t i = 0; src[i] != '\0'; i++) {
        char ch = src[i];

        if (ch == '\\' && src[i + 1] != '\0') {
            i++;
            if (src[i] == 'r') {
                ch = '\r';
            } else if (src[i] == 'n') {
                ch = '\n';
            } else if (src[i] == 't') {
                ch = '\t';
            } else {
                ch = src[i];
            }
        }

        TEST_ASSERT_LESS_THAN_UINT_MESSAGE(dst_size, out + 1, "decoded wire overflow");
        dst[out++] = ch;
    }

    return out;
}

static unsigned long long parse_ull(const char *value, const char *path)
{
    char *end = nullptr;
    unsigned long long parsed = strtoull(value, &end, 10);

    TEST_ASSERT_NOT_NULL_MESSAGE(end, path);
    TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', *end, path);
    return parsed;
}

static void run_corpus_case(const char *path)
{
    char line[4096];
    char wire_line[8192] = {0};
    char decoded_line[8192] = {0};
    char wire[8192];
    FILE *fp = fopen(path, "r");
    corpus_kind_t kind = CORPUS_KIND_CHUNKED;
    corpus_expect_t expect = CORPUS_EXPECT_OK;
    bool consume_trailer = false;
    bool has_kind = false;
    bool has_expect = false;
    bool has_consume_trailer = false;
    bool has_wire = false;
    bool has_decoded = false;
    bool has_status_value = false;
    bool has_total_decoded = false;
    bool has_content_length = false;
    bool has_consume_len = false;
    bool has_remaining = false;
    int status_value = 0;
    uint64_t total_decoded = 0;
    uint64_t content_length = 0;
    size_t consume_len = 0;
    uint64_t remaining = 0;

    TEST_ASSERT_NOT_NULL_MESSAGE(fp, path);

    while (fgets(line, sizeof(line), fp) != nullptr) {
        trim_trailing_newline(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        char *eq = strchr(line, '=');
        TEST_ASSERT_NOT_NULL_MESSAGE(eq, path);
        *eq = '\0';

        const char *key = line;
        const char *value = eq + 1;

        if (str_eq(key, "kind")) {
            if (str_eq(value, kChunked)) {
                kind = CORPUS_KIND_CHUNKED;
            } else if (str_eq(value, kFixed)) {
                kind = CORPUS_KIND_FIXED;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_kind = true;
        } else if (str_eq(key, "expect")) {
            if (str_eq(value, kOk)) {
                expect = CORPUS_EXPECT_OK;
            } else if (str_eq(value, kError)) {
                expect = CORPUS_EXPECT_ERROR;
            } else if (str_eq(value, kIncomplete)) {
                expect = CORPUS_EXPECT_INCOMPLETE;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_expect = true;
        } else if (str_eq(key, "consume_trailer")) {
            if (str_eq(value, kTrue)) {
                consume_trailer = true;
            } else if (str_eq(value, kFalse)) {
                consume_trailer = false;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_consume_trailer = true;
        } else if (str_eq(key, "wire")) {
            TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', wire_line[0], path);
            strncpy(wire_line, value, sizeof(wire_line) - 1);
            has_wire = true;
        } else if (str_eq(key, "decoded")) {
            TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', decoded_line[0], path);
            strncpy(decoded_line, value, sizeof(decoded_line) - 1);
            has_decoded = true;
        } else if (str_eq(key, "status_value")) {
            status_value = (int)parse_ull(value, path);
            has_status_value = true;
        } else if (str_eq(key, "total_decoded")) {
            total_decoded = (uint64_t)parse_ull(value, path);
            has_total_decoded = true;
        } else if (str_eq(key, "content_length")) {
            content_length = (uint64_t)parse_ull(value, path);
            has_content_length = true;
        } else if (str_eq(key, "consume_len")) {
            consume_len = (size_t)parse_ull(value, path);
            has_consume_len = true;
        } else if (str_eq(key, "remaining")) {
            remaining = (uint64_t)parse_ull(value, path);
            has_remaining = true;
        } else {
            TEST_FAIL_MESSAGE(path);
        }
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fclose(fp), path);
    TEST_ASSERT_TRUE_MESSAGE(has_kind, path);
    TEST_ASSERT_TRUE_MESSAGE(has_expect, path);

    if (kind == CORPUS_KIND_CHUNKED) {
        size_t wire_len;
        ihtp_chunked_decoder_t dec;
        size_t bufsz;
        ihtp_status_t status;

        TEST_ASSERT_TRUE_MESSAGE(has_wire, path);
        TEST_ASSERT_TRUE_MESSAGE(has_consume_trailer, path);

        wire_len = decode_wire(wire_line, wire, sizeof(wire));
        bufsz = wire_len;
        dec = (ihtp_chunked_decoder_t){.consume_trailer = consume_trailer};
        status = ihtp_decode_chunked(&dec, wire, &bufsz);

        if (expect == CORPUS_EXPECT_ERROR) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_ERROR, status, path);
            return;
        }
        if (expect == CORPUS_EXPECT_INCOMPLETE) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_INCOMPLETE, status, path);
        } else {
            TEST_ASSERT_TRUE_MESSAGE(status >= 0, path);
            if (has_status_value) {
                TEST_ASSERT_EQUAL_INT_MESSAGE(status_value, status, path);
            }
        }

        if (has_decoded) {
            char decoded[8192];
            size_t decoded_len = decode_wire(decoded_line, decoded, sizeof(decoded));

            TEST_ASSERT_EQUAL_UINT_MESSAGE(decoded_len, bufsz, path);
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(decoded, wire, decoded_len, path);
        }
        if (has_total_decoded) {
            TEST_ASSERT_EQUAL_UINT64_MESSAGE(total_decoded, dec.total_decoded, path);
        }
        return;
    }

    TEST_ASSERT_TRUE_MESSAGE(has_content_length, path);
    TEST_ASSERT_TRUE_MESSAGE(has_consume_len, path);
    {
        ihtp_fixed_decoder_t dec;
        ihtp_status_t status;

        ihtp_fixed_decoder_init(&dec, content_length);
        status = ihtp_decode_fixed(&dec, consume_len);

        if (expect == CORPUS_EXPECT_ERROR) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_ERROR, status, path);
        } else if (expect == CORPUS_EXPECT_INCOMPLETE) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_INCOMPLETE, status, path);
        } else {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_OK, status, path);
        }

        if (has_total_decoded) {
            TEST_ASSERT_EQUAL_UINT64_MESSAGE(total_decoded, dec.total_decoded, path);
        }
        if (has_remaining) {
            TEST_ASSERT_EQUAL_UINT64_MESSAGE(remaining, dec.remaining, path);
        }
    }
}

void test_body_decoder_corpus_cases(void)
{
    static const char *cases[] = {
        IHTP_SOURCE_DIR "/tests/corpus/body/chunked_ok_simple.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/chunked_ok_trailer_consumed.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/chunked_ok_trailing_bytes.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/chunked_incomplete_trailer.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/chunked_error_missing_size_lf.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/chunked_error_bare_lf_trailer.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/fixed_ok_complete.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/fixed_incomplete_partial.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/fixed_error_overflow.case",
        IHTP_SOURCE_DIR "/tests/corpus/body/fixed_ok_zero_length.case",
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        run_corpus_case(cases[i]);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_body_decoder_corpus_cases);
    return UNITY_END();
}
