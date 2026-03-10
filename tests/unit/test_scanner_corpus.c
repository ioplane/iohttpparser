/*
 * iohttpparser — Scanner corpus tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_scanner.h>
#include "ihtp_internal.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    CORPUS_KIND_FIND,
    CORPUS_KIND_TOKEN,
} corpus_kind_t;

static const char *kFind = "find";
static const char *kToken = "token";
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

static uint8_t parse_hex_byte(char hi, char lo, const char *path)
{
    TEST_ASSERT_TRUE_MESSAGE(isxdigit((unsigned char)hi) != 0, path);
    TEST_ASSERT_TRUE_MESSAGE(isxdigit((unsigned char)lo) != 0, path);

    uint8_t value = 0;

    value = (uint8_t)(isdigit((unsigned char)hi)
                          ? hi - '0'
                          : (uint8_t)(tolower((unsigned char)hi) - 'a' + 10));
    value = (uint8_t)(value << 4);
    value |= (uint8_t)(isdigit((unsigned char)lo)
                           ? lo - '0'
                           : (uint8_t)(tolower((unsigned char)lo) - 'a' + 10));
    return value;
}

static size_t decode_wire(const char *src, char *dst, size_t dst_size, const char *path)
{
    size_t out = 0;

    for (size_t i = 0; src[i] != '\0'; i++) {
        uint8_t ch = (uint8_t)src[i];

        if (ch == '\\' && src[i + 1] != '\0') {
            i++;
            switch (src[i]) {
            case 'r':
                ch = '\r';
                break;
            case 'n':
                ch = '\n';
                break;
            case 't':
                ch = '\t';
                break;
            case '0':
                ch = '\0';
                break;
            case 'x':
                TEST_ASSERT_NOT_EQUAL_MESSAGE('\0', src[i + 1], path);
                TEST_ASSERT_NOT_EQUAL_MESSAGE('\0', src[i + 2], path);
                ch = parse_hex_byte(src[i + 1], src[i + 2], path);
                i += 2;
                break;
            default:
                ch = (uint8_t)src[i];
                break;
            }
        }

        TEST_ASSERT_LESS_THAN_UINT_MESSAGE(dst_size, out + 1, path);
        dst[out++] = (char)ch;
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

static bool parse_bool(const char *value, const char *path)
{
    if (str_eq(value, kTrue)) {
        return true;
    }
    if (str_eq(value, kFalse)) {
        return false;
    }

    TEST_FAIL_MESSAGE(path);
    return false;
}

static void assert_find_backend(const char *path, const char *backend_name,
                                ihtp_scan_find_fn find_fn, const char *buf, size_t len,
                                const char *delims, size_t expected_offset)
{
    const char *actual = find_fn(buf, len, delims);

    TEST_ASSERT_EQUAL_PTR_MESSAGE(buf + expected_offset, actual, backend_name);
    (void)path;
}

static void assert_token_backend(const char *backend_name, ihtp_scan_token_fn token_fn,
                                 const char *buf, size_t len, bool expected)
{
    bool actual = token_fn(buf, len);

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)expected, (int)actual, backend_name);
}

static void run_corpus_case(const char *path)
{
    char line[4096];
    char buf_line[8192] = {0};
    char delims_line[256] = {0};
    char buf[8192] = {0};
    char delims[256] = {0};
    FILE *fp = fopen(path, "r");
    corpus_kind_t kind = CORPUS_KIND_FIND;
    bool has_kind = false;
    bool has_buf = false;
    bool has_delims = false;
    bool has_expected_offset = false;
    bool has_expected_bool = false;
    bool expected_bool = false;
    size_t expected_offset = 0;
    size_t buf_len = 0;
    size_t delims_len = 0;

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
            if (str_eq(value, kFind)) {
                kind = CORPUS_KIND_FIND;
            } else if (str_eq(value, kToken)) {
                kind = CORPUS_KIND_TOKEN;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_kind = true;
        } else if (str_eq(key, "buf")) {
            strncpy(buf_line, value, sizeof(buf_line) - 1);
            has_buf = true;
        } else if (str_eq(key, "delims")) {
            strncpy(delims_line, value, sizeof(delims_line) - 1);
            has_delims = true;
        } else if (str_eq(key, "expected_offset")) {
            expected_offset = (size_t)parse_ull(value, path);
            has_expected_offset = true;
        } else if (str_eq(key, "expected")) {
            expected_bool = parse_bool(value, path);
            has_expected_bool = true;
        } else {
            TEST_FAIL_MESSAGE(path);
        }
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fclose(fp), path);
    TEST_ASSERT_TRUE_MESSAGE(has_kind, path);
    TEST_ASSERT_TRUE_MESSAGE(has_buf, path);

    buf_len = decode_wire(buf_line, buf, sizeof(buf), path);
    delims_len = decode_wire(delims_line, delims, sizeof(delims), path);
    TEST_ASSERT_LESS_THAN_UINT_MESSAGE(sizeof(delims), delims_len + 1, path);
    delims[delims_len] = '\0';

    if (kind == CORPUS_KIND_FIND) {
        const char *scalar = ihtp_scan_find_char_scalar(buf, buf_len, delims);

        TEST_ASSERT_TRUE_MESSAGE(has_delims, path);
        TEST_ASSERT_TRUE_MESSAGE(has_expected_offset, path);
        TEST_ASSERT_EQUAL_PTR_MESSAGE(buf + expected_offset, scalar, path);

        assert_find_backend(path, "dispatch", ihtp_scan_find_char, buf, buf_len, delims,
                            expected_offset);

#ifdef IOHTTPPARSER_HAVE_SSE42
        if ((ihtp_scanner_simd_level() & 0x01) != 0) {
            TEST_ASSERT_EQUAL_PTR_MESSAGE(scalar, ihtp_scan_find_char_sse42(buf, buf_len, delims),
                                          "sse42");
        }
#endif
#ifdef IOHTTPPARSER_HAVE_AVX2
        if ((ihtp_scanner_simd_level() & 0x02) != 0) {
            TEST_ASSERT_EQUAL_PTR_MESSAGE(scalar, ihtp_scan_find_char_avx2(buf, buf_len, delims),
                                          "avx2");
        }
#endif
        return;
    }

    TEST_ASSERT_TRUE_MESSAGE(has_expected_bool, path);
    TEST_ASSERT_EQUAL_INT_MESSAGE((int)expected_bool, (int)ihtp_scan_is_token_scalar(buf, buf_len),
                                  path);
    assert_token_backend("dispatch", ihtp_scan_is_token, buf, buf_len, expected_bool);

#ifdef IOHTTPPARSER_HAVE_SSE42
    if ((ihtp_scanner_simd_level() & 0x01) != 0) {
        TEST_ASSERT_EQUAL_INT_MESSAGE((int)expected_bool,
                                      (int)ihtp_scan_is_token_sse42(buf, buf_len), "sse42");
    }
#endif
#ifdef IOHTTPPARSER_HAVE_AVX2
    if ((ihtp_scanner_simd_level() & 0x02) != 0) {
        TEST_ASSERT_EQUAL_INT_MESSAGE((int)expected_bool,
                                      (int)ihtp_scan_is_token_avx2(buf, buf_len), "avx2");
    }
#endif
}

void test_scanner_corpus_cases(void)
{
    static const char *cases[] = {
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_request_space.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_crlf.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_high_byte.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_embedded_nul.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_empty_delims.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_long_delims_fallback.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/find_avx2_wide_block.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/token_valid_header.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/token_invalid_space.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/token_invalid_high_byte.case",
        IHTP_SOURCE_DIR "/tests/corpus/scanner/token_invalid_nul.case",
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        run_corpus_case(cases[i]);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scanner_corpus_cases);
    return UNITY_END();
}
