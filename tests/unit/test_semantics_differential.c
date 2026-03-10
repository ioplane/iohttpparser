/*
 * iohttpparser — Differential semantics tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>
#include <llhttp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern ihtp_status_t ihtp_request_apply_semantics(ihtp_request_t *req, const ihtp_policy_t *policy);
extern ihtp_status_t ihtp_response_apply_semantics(ihtp_response_t *resp,
                                                   const ihtp_policy_t *policy);

typedef enum {
    DIFF_KIND_REQUEST,
    DIFF_KIND_RESPONSE,
} diff_kind_t;

typedef enum {
    DIFF_EXPECT_OK,
    DIFF_EXPECT_ERROR,
} diff_expect_t;

typedef struct {
    ihtp_status_t status;
    ihtp_body_mode_t body_mode;
    uint64_t content_length;
    bool keep_alive;
    bool has_snapshot;
} llhttp_semantics_result_t;

typedef struct {
    ihtp_body_mode_t body_mode;
    uint64_t content_length;
    bool keep_alive;
    bool has_snapshot;
} llhttp_semantics_capture_t;

static const char *kLlhttp = "llhttp";
static const char *kRequest = "request";
static const char *kResponse = "response";
static const char *kStrict = "strict";
static const char *kLenient = "lenient";
static const char *kOk = "ok";
static const char *kError = "error";
static const char *kNone = "none";
static const char *kFixed = "fixed";
static const char *kChunked = "chunked";
static const char *kEof = "eof";
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

static size_t decode_wire(const char *src, char *dst, size_t dst_size, const char *path)
{
    size_t out = 0;

    for (size_t i = 0; src[i] != '\0'; i++) {
        char ch = src[i];

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
            default:
                ch = src[i];
                break;
            }
        }

        TEST_ASSERT_LESS_THAN_UINT_MESSAGE(dst_size, out + 1, path);
        dst[out++] = ch;
    }

    return out;
}

static diff_kind_t parse_kind(const char *value, const char *path)
{
    if (str_eq(value, kRequest)) {
        return DIFF_KIND_REQUEST;
    }
    if (str_eq(value, kResponse)) {
        return DIFF_KIND_RESPONSE;
    }

    TEST_FAIL_MESSAGE(path);
    return DIFF_KIND_REQUEST;
}

static diff_expect_t parse_expect(const char *value, const char *path)
{
    if (str_eq(value, kOk)) {
        return DIFF_EXPECT_OK;
    }
    if (str_eq(value, kError)) {
        return DIFF_EXPECT_ERROR;
    }

    TEST_FAIL_MESSAGE(path);
    return DIFF_EXPECT_ERROR;
}

static const ihtp_policy_t *parse_policy(const char *value, const char *path)
{
    static const ihtp_policy_t strict = IHTP_POLICY_STRICT;
    static const ihtp_policy_t lenient = IHTP_POLICY_LENIENT;

    if (str_eq(value, kStrict)) {
        return &strict;
    }
    if (str_eq(value, kLenient)) {
        return &lenient;
    }

    TEST_FAIL_MESSAGE(path);
    return nullptr;
}

static ihtp_body_mode_t parse_body_mode(const char *value, const char *path)
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

    TEST_FAIL_MESSAGE(path);
    return IHTP_BODY_NONE;
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

static unsigned long long parse_ull(const char *value, const char *path)
{
    char *end = nullptr;
    unsigned long long parsed = strtoull(value, &end, 10);

    TEST_ASSERT_NOT_NULL_MESSAGE(end, path);
    TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', *end, path);
    return parsed;
}

static void configure_llhttp_policy(llhttp_t *parser, const ihtp_policy_t *policy)
{
    if (policy == nullptr) {
        return;
    }
    if (!policy->reject_bare_lf) {
        llhttp_set_lenient_optional_cr_before_lf(parser, 1);
        llhttp_set_lenient_optional_lf_after_cr(parser, 1);
    }
    if (!policy->reject_te_cl) {
        llhttp_set_lenient_chunked_length(parser, 1);
    }
}

static ihtp_body_mode_t classify_llhttp_body_mode(const llhttp_t *parser)
{
    if (parser->type == HTTP_RESPONSE &&
        (parser->status_code / 100 == 1 || parser->status_code == 204 ||
         parser->status_code == 304 || (parser->flags & F_SKIPBODY) != 0)) {
        return IHTP_BODY_NONE;
    }

    if ((parser->flags & F_SKIPBODY) != 0) {
        return IHTP_BODY_NONE;
    }
    if ((parser->flags & F_CHUNKED) != 0) {
        return IHTP_BODY_CHUNKED;
    }
    if ((parser->flags & F_TRANSFER_ENCODING) != 0) {
        return IHTP_BODY_EOF;
    }
    if ((parser->flags & F_CONTENT_LENGTH) != 0) {
        return parser->content_length == 0 ? IHTP_BODY_NONE : IHTP_BODY_FIXED;
    }
    if (llhttp_message_needs_eof(parser) != 0) {
        return IHTP_BODY_EOF;
    }
    return IHTP_BODY_NONE;
}

static int llhttp_on_headers_complete(llhttp_t *parser)
{
    llhttp_semantics_capture_t *capture = parser->data;

    capture->body_mode = classify_llhttp_body_mode(parser);
    capture->content_length = parser->content_length;
    capture->keep_alive = llhttp_should_keep_alive(parser) != 0;
    capture->has_snapshot = true;
    return 0;
}

static ihtp_status_t map_llhttp_semantics_status(llhttp_errno_t execute_err)
{
    return execute_err == HPE_OK ? IHTP_OK : IHTP_ERROR;
}

static void run_llhttp_request_semantics(const char *buf, size_t len, const ihtp_policy_t *policy,
                                         llhttp_semantics_result_t *result)
{
    llhttp_t parser;
    llhttp_settings_t settings;
    llhttp_semantics_capture_t capture = {0};
    llhttp_errno_t execute_err;

    llhttp_settings_init(&settings);
    settings.on_headers_complete = llhttp_on_headers_complete;

    llhttp_init(&parser, HTTP_REQUEST, &settings);
    parser.data = &capture;
    configure_llhttp_policy(&parser, policy);

    execute_err = llhttp_execute(&parser, buf, len);

    result->status = map_llhttp_semantics_status(execute_err);
    result->body_mode = capture.body_mode;
    result->content_length = capture.content_length;
    result->keep_alive = capture.keep_alive;
    result->has_snapshot = capture.has_snapshot;
}

static void run_llhttp_response_semantics(const char *buf, size_t len, const ihtp_policy_t *policy,
                                          llhttp_semantics_result_t *result)
{
    llhttp_t parser;
    llhttp_settings_t settings;
    llhttp_semantics_capture_t capture = {0};
    llhttp_errno_t execute_err;

    llhttp_settings_init(&settings);
    settings.on_headers_complete = llhttp_on_headers_complete;

    llhttp_init(&parser, HTTP_RESPONSE, &settings);
    parser.data = &capture;
    configure_llhttp_policy(&parser, policy);

    execute_err = llhttp_execute(&parser, buf, len);

    result->status = map_llhttp_semantics_status(execute_err);
    result->body_mode = capture.body_mode;
    result->content_length = capture.content_length;
    result->keep_alive = capture.keep_alive;
    result->has_snapshot = capture.has_snapshot;
}

static void run_case(const char *path)
{
    char line[4096];
    char wire_line[8192] = {0};
    char wire[8192];
    FILE *fp = fopen(path, "r");
    diff_kind_t kind = DIFF_KIND_REQUEST;
    diff_expect_t reference_expect = DIFF_EXPECT_OK;
    diff_expect_t ihtp_expect = DIFF_EXPECT_OK;
    const ihtp_policy_t *policy = nullptr;
    ihtp_body_mode_t body_mode = IHTP_BODY_NONE;
    uint64_t content_length = 0;
    bool keep_alive = false;
    bool has_reference = false;
    bool has_kind = false;
    bool has_reference_expect = false;
    bool has_ihtp_expect = false;
    bool has_policy = false;
    bool has_wire = false;
    bool has_body_mode = false;
    bool has_content_length = false;
    bool has_keep_alive = false;

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

        if (str_eq(key, "reference")) {
            TEST_ASSERT_TRUE_MESSAGE(str_eq(value, kLlhttp), path);
            has_reference = true;
        } else if (str_eq(key, "kind")) {
            kind = parse_kind(value, path);
            has_kind = true;
        } else if (str_eq(key, "expect")) {
            reference_expect = parse_expect(value, path);
            ihtp_expect = reference_expect;
            has_reference_expect = true;
            has_ihtp_expect = true;
        } else if (str_eq(key, "reference_expect")) {
            reference_expect = parse_expect(value, path);
            has_reference_expect = true;
        } else if (str_eq(key, "ihtp_expect")) {
            ihtp_expect = parse_expect(value, path);
            has_ihtp_expect = true;
        } else if (str_eq(key, "policy")) {
            policy = parse_policy(value, path);
            has_policy = true;
        } else if (str_eq(key, "wire")) {
            strncpy(wire_line, value, sizeof(wire_line) - 1);
            has_wire = true;
        } else if (str_eq(key, "body_mode")) {
            body_mode = parse_body_mode(value, path);
            has_body_mode = true;
        } else if (str_eq(key, "content_length")) {
            content_length = (uint64_t)parse_ull(value, path);
            has_content_length = true;
        } else if (str_eq(key, "keep_alive")) {
            keep_alive = parse_bool(value, path);
            has_keep_alive = true;
        } else {
            TEST_FAIL_MESSAGE(path);
        }
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, fclose(fp), path);
    TEST_ASSERT_TRUE_MESSAGE(has_reference, path);
    TEST_ASSERT_TRUE_MESSAGE(has_kind, path);
    TEST_ASSERT_TRUE_MESSAGE(has_reference_expect, path);
    TEST_ASSERT_TRUE_MESSAGE(has_ihtp_expect, path);
    TEST_ASSERT_TRUE_MESSAGE(has_policy, path);
    TEST_ASSERT_TRUE_MESSAGE(has_wire, path);

    size_t wire_len = decode_wire(wire_line, wire, sizeof(wire), path);

    if (kind == DIFF_KIND_REQUEST) {
        ihtp_request_t req;
        llhttp_semantics_result_t reference;
        size_t consumed = 0;
        ihtp_status_t status = ihtp_parse_request(wire, wire_len, &req, policy, &consumed);

        if (status == IHTP_OK) {
            status = ihtp_request_apply_semantics(&req, policy);
        }

        run_llhttp_request_semantics(wire, wire_len, policy, &reference);

        TEST_ASSERT_EQUAL_INT_MESSAGE(reference_expect == DIFF_EXPECT_OK ? IHTP_OK : IHTP_ERROR,
                                      reference.status, path);
        TEST_ASSERT_EQUAL_INT_MESSAGE(ihtp_expect == DIFF_EXPECT_OK ? IHTP_OK : IHTP_ERROR, status,
                                      path);

        if (ihtp_expect == DIFF_EXPECT_OK) {
            if (has_body_mode) {
                TEST_ASSERT_EQUAL_INT_MESSAGE(body_mode, req.body_mode, path);
            }
            if (has_content_length) {
                TEST_ASSERT_EQUAL_UINT64_MESSAGE(content_length, req.content_length, path);
            }
            if (has_keep_alive) {
                TEST_ASSERT_EQUAL_INT_MESSAGE(keep_alive, req.keep_alive, path);
            }
        }
        if (reference_expect == DIFF_EXPECT_OK) {
            TEST_ASSERT_TRUE_MESSAGE(reference.has_snapshot, path);
            if (has_body_mode) {
                TEST_ASSERT_EQUAL_INT_MESSAGE(body_mode, reference.body_mode, path);
            }
            if (has_content_length) {
                TEST_ASSERT_EQUAL_UINT64_MESSAGE(content_length, reference.content_length, path);
            }
            if (has_keep_alive) {
                TEST_ASSERT_EQUAL_INT_MESSAGE(keep_alive, reference.keep_alive, path);
            }
        }
        return;
    }

    ihtp_response_t resp;
    llhttp_semantics_result_t reference;
    size_t consumed = 0;
    ihtp_status_t status = ihtp_parse_response(wire, wire_len, &resp, policy, &consumed);

    if (status == IHTP_OK) {
        status = ihtp_response_apply_semantics(&resp, policy);
    }

    run_llhttp_response_semantics(wire, wire_len, policy, &reference);

    TEST_ASSERT_EQUAL_INT_MESSAGE(reference_expect == DIFF_EXPECT_OK ? IHTP_OK : IHTP_ERROR,
                                  reference.status, path);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ihtp_expect == DIFF_EXPECT_OK ? IHTP_OK : IHTP_ERROR, status,
                                  path);

    if (ihtp_expect == DIFF_EXPECT_OK) {
        if (has_body_mode) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(body_mode, resp.body_mode, path);
        }
        if (has_content_length) {
            TEST_ASSERT_EQUAL_UINT64_MESSAGE(content_length, resp.content_length, path);
        }
        if (has_keep_alive) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(keep_alive, resp.keep_alive, path);
        }
    }
    if (reference_expect == DIFF_EXPECT_OK) {
        TEST_ASSERT_TRUE_MESSAGE(reference.has_snapshot, path);
        if (has_body_mode) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(body_mode, reference.body_mode, path);
        }
        if (has_content_length) {
            TEST_ASSERT_EQUAL_UINT64_MESSAGE(content_length, reference.content_length, path);
        }
        if (has_keep_alive) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(keep_alive, reference.keep_alive, path);
        }
    }
}

void test_semantics_differential_cases(void)
{
    static const char *cases[] = {
        "tests/corpus/semantics-differential/llhttp_request_content_length_fixed.case",
        "tests/corpus/semantics-differential/"
        "llhttp_request_identical_duplicate_content_length.case",
        "tests/corpus/semantics-differential/"
        "llhttp_request_reject_conflicting_content_length.case",
        "tests/corpus/semantics-differential/llhttp_request_reject_te_cl.case",
        "tests/corpus/semantics-differential/llhttp_request_lenient_te_cl.case",
        "tests/corpus/semantics-differential/llhttp_response_204_ignores_content_length.case",
        "tests/corpus/semantics-differential/llhttp_response_content_length_fixed.case",
        "tests/corpus/semantics-differential/llhttp_response_chunked.case",
        "tests/corpus/semantics-differential/llhttp_response_http11_eof_default_close.case",
        "tests/corpus/semantics-differential/"
        "llhttp_response_identical_duplicate_content_length.case",
        "tests/corpus/semantics-differential/"
        "llhttp_response_reject_conflicting_content_length.case",
        "tests/corpus/semantics-differential/llhttp_response_reject_te_cl.case",
        "tests/corpus/semantics-differential/llhttp_response_lenient_te_cl.case",
        "tests/corpus/semantics-differential/llhttp_response_eof_transfer_encoding.case",
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char full_path[1024];
        int written = snprintf(full_path, sizeof(full_path), "%s/%s", IHTP_SOURCE_DIR, cases[i]);

        TEST_ASSERT_GREATER_THAN_INT_MESSAGE(0, written, cases[i]);
        TEST_ASSERT_LESS_THAN_INT_MESSAGE((int)sizeof(full_path), written, cases[i]);
        run_case(full_path);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_semantics_differential_cases);
    return UNITY_END();
}
