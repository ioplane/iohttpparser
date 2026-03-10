/*
 * iohttpparser — Semantics corpus tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern ihtp_status_t ihtp_request_apply_semantics(ihtp_request_t *req, const ihtp_policy_t *policy);
extern ihtp_status_t ihtp_response_apply_semantics(ihtp_response_t *resp,
                                                   const ihtp_policy_t *policy);

typedef enum {
    CORPUS_KIND_REQUEST,
    CORPUS_KIND_RESPONSE,
} corpus_kind_t;

typedef enum {
    CORPUS_EXPECT_OK,
    CORPUS_EXPECT_ERROR,
} corpus_expect_t;

typedef struct {
    const char *path;
    corpus_kind_t kind;
    const ihtp_policy_t *policy;
    corpus_expect_t expect;
    ihtp_body_mode_t body_mode;
    bool has_body_mode;
} corpus_case_t;

static const char *kStrict = "strict";
static const char *kLenient = "lenient";
static const char *kRequest = "request";
static const char *kResponse = "response";
static const char *kOk = "ok";
static const char *kError = "error";
static const char *kNone = "none";
static const char *kFixed = "fixed";
static const char *kChunked = "chunked";
static const char *kEof = "eof";

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

static ihtp_body_mode_t parse_body_mode(const char *value)
{
    if (str_eq(value, kNone)) {
        return IHTP_BODY_NONE;
    }
    if (str_eq(value, kFixed)) {
        return IHTP_BODY_FIXED;
    }
    if (str_eq(value, kChunked)) {
        return IHTP_BODY_CHUNKED;
    }
    if (str_eq(value, kEof)) {
        return IHTP_BODY_EOF;
    }

    TEST_FAIL_MESSAGE("unknown body_mode in corpus");
    return IHTP_BODY_NONE;
}

static void run_corpus_case(const char *path)
{
    char line[4096];
    char wire_line[8192] = {0};
    char wire[8192];
    FILE *fp = fopen(path, "r");
    corpus_kind_t kind = CORPUS_KIND_REQUEST;
    const ihtp_policy_t *policy = nullptr;
    corpus_expect_t expect = CORPUS_EXPECT_OK;
    ihtp_body_mode_t body_mode = IHTP_BODY_NONE;
    bool has_kind = false;
    bool has_mode = false;
    bool has_expect = false;
    bool has_body_mode = false;
    bool has_wire = false;

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
            if (str_eq(value, kRequest)) {
                kind = CORPUS_KIND_REQUEST;
            } else if (str_eq(value, kResponse)) {
                kind = CORPUS_KIND_RESPONSE;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_kind = true;
        } else if (str_eq(key, "mode")) {
            static const ihtp_policy_t strict = IHTP_POLICY_STRICT;
            static const ihtp_policy_t lenient = IHTP_POLICY_LENIENT;

            if (str_eq(value, kStrict)) {
                policy = &strict;
            } else if (str_eq(value, kLenient)) {
                policy = &lenient;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_mode = true;
        } else if (str_eq(key, "expect")) {
            if (str_eq(value, kOk)) {
                expect = CORPUS_EXPECT_OK;
            } else if (str_eq(value, kError)) {
                expect = CORPUS_EXPECT_ERROR;
            } else {
                TEST_FAIL_MESSAGE(path);
            }
            has_expect = true;
        } else if (str_eq(key, "body_mode")) {
            body_mode = parse_body_mode(value);
            has_body_mode = true;
        } else if (str_eq(key, "wire")) {
            TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', wire_line[0], path);
            strncpy(wire_line, value, sizeof(wire_line) - 1);
            has_wire = true;
        } else {
            TEST_FAIL_MESSAGE(path);
        }
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fclose(fp), path);

    TEST_ASSERT_TRUE_MESSAGE(has_kind, path);
    TEST_ASSERT_TRUE_MESSAGE(has_mode, path);
    TEST_ASSERT_TRUE_MESSAGE(has_expect, path);
    TEST_ASSERT_TRUE_MESSAGE(has_wire, path);

    size_t wire_len = decode_wire(wire_line, wire, sizeof(wire));

    if (kind == CORPUS_KIND_REQUEST) {
        ihtp_request_t req;
        size_t consumed = 0;
        ihtp_status_t status = ihtp_parse_request(wire, wire_len, &req, policy, &consumed);

        if (status == IHTP_OK) {
            status = ihtp_request_apply_semantics(&req, policy);
        }

        if (expect == CORPUS_EXPECT_ERROR) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_ERROR, status, path);
            return;
        }

        TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_OK, status, path);
        if (has_body_mode) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(body_mode, req.body_mode, path);
        }
        return;
    }

    ihtp_response_t resp;
    size_t consumed = 0;
    ihtp_status_t status = ihtp_parse_response(wire, wire_len, &resp, policy, &consumed);

    if (status == IHTP_OK) {
        status = ihtp_response_apply_semantics(&resp, policy);
    }

    if (expect == CORPUS_EXPECT_ERROR) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_ERROR, status, path);
        return;
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_OK, status, path);
    if (has_body_mode) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(body_mode, resp.body_mode, path);
    }
}

void test_semantics_corpus_cases(void)
{
    static const char *cases[] = {
        IHTP_SOURCE_DIR "/tests/corpus/semantics/request_reject_te_cl.case",
        IHTP_SOURCE_DIR "/tests/corpus/semantics/request_reject_duplicate_chunked.case",
        IHTP_SOURCE_DIR "/tests/corpus/semantics/request_reject_http11_missing_host.case",
        IHTP_SOURCE_DIR "/tests/corpus/semantics/request_ok_lenient_te_cl.case",
        IHTP_SOURCE_DIR "/tests/corpus/semantics/response_reject_te_cl.case",
        IHTP_SOURCE_DIR "/tests/corpus/semantics/response_no_body_204_ignores_te.case",
        IHTP_SOURCE_DIR "/tests/corpus/semantics/response_no_body_101_ignores_cl.case",
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        run_corpus_case(cases[i]);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_semantics_corpus_cases);
    return UNITY_END();
}
