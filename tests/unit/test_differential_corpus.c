/*
 * iohttpparser — Differential corpus tests
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <unity/unity.h>

#include <iohttpparser/ihtp_parser.h>
#include <llhttp.h>
#include <picohttpparser.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    DIFF_REFERENCE_PICO,
    DIFF_REFERENCE_LLHTTP,
} diff_reference_t;

typedef enum {
    DIFF_KIND_REQUEST,
    DIFF_KIND_RESPONSE,
} diff_kind_t;

typedef enum {
    DIFF_EXPECT_OK,
    DIFF_EXPECT_INCOMPLETE,
    DIFF_EXPECT_ERROR,
} diff_expect_t;

static const char *kReferencePico = "pico";
static const char *kReferenceLlhttp = "llhttp";
static const char *kRequest = "request";
static const char *kResponse = "response";
static const char *kOk = "ok";
static const char *kIncomplete = "incomplete";
static const char *kError = "error";
static const char *kStrict = "strict";
static const char *kLenient = "lenient";

typedef struct {
    ihtp_status_t status;
    size_t consumed;
    const char *method;
    size_t method_len;
    const char *path;
    size_t path_len;
    ihtp_http_version_t version;
    size_t num_headers;
} pico_request_result_t;

typedef struct {
    ihtp_status_t status;
    size_t consumed;
    int status_code;
    const char *reason;
    size_t reason_len;
    ihtp_http_version_t version;
    size_t num_headers;
} pico_response_result_t;

typedef struct {
    ihtp_status_t status;
    size_t consumed;
    const char *method;
    size_t method_len;
    const char *path;
    size_t path_len;
    ihtp_http_version_t version;
    size_t num_headers;
} llhttp_request_result_t;

typedef struct {
    ihtp_status_t status;
    size_t consumed;
    int status_code;
    const char *reason;
    size_t reason_len;
    ihtp_http_version_t version;
    size_t num_headers;
} llhttp_response_result_t;

typedef struct {
    const char *method;
    size_t method_len;
    const char *path;
    size_t path_len;
    const char *reason;
    size_t reason_len;
    size_t num_headers;
    bool message_complete;
} llhttp_capture_t;

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

static diff_reference_t parse_reference(const char *value, const char *path)
{
    if (str_eq(value, kReferencePico)) {
        return DIFF_REFERENCE_PICO;
    }
    if (str_eq(value, kReferenceLlhttp)) {
        return DIFF_REFERENCE_LLHTTP;
    }

    TEST_FAIL_MESSAGE(path);
    return DIFF_REFERENCE_PICO;
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
    if (str_eq(value, kIncomplete)) {
        return DIFF_EXPECT_INCOMPLETE;
    }
    if (str_eq(value, kError)) {
        return DIFF_EXPECT_ERROR;
    }

    TEST_FAIL_MESSAGE(path);
    return DIFF_EXPECT_ERROR;
}

static ihtp_status_t expect_to_status(diff_expect_t expect)
{
    switch (expect) {
    case DIFF_EXPECT_OK:
        return IHTP_OK;
    case DIFF_EXPECT_INCOMPLETE:
        return IHTP_INCOMPLETE;
    case DIFF_EXPECT_ERROR:
        return IHTP_ERROR;
    }

    return IHTP_ERROR;
}

static ihtp_http_version_t parse_version(const char *value, const char *path)
{
    if (str_eq(value, "1.0")) {
        return IHTP_HTTP_10;
    }
    if (str_eq(value, "1.1")) {
        return IHTP_HTTP_11;
    }

    TEST_FAIL_MESSAGE(path);
    return IHTP_HTTP_10;
}

static void run_pico_request(const char *buf, size_t len, pico_request_result_t *result)
{
    struct phr_header headers[IHTP_MAX_HEADERS];
    size_t num_headers = IHTP_MAX_HEADERS;
    const char *method = nullptr;
    size_t method_len = 0;
    const char *path = nullptr;
    size_t path_len = 0;
    int minor_version = -1;
    int rc = phr_parse_request(buf, len, &method, &method_len, &path, &path_len, &minor_version,
                               headers, &num_headers, 0);

    result->status = rc >= 0 ? IHTP_OK : (rc == -2 ? IHTP_INCOMPLETE : IHTP_ERROR);
    result->consumed = rc >= 0 ? (size_t)rc : 0;
    result->method = method;
    result->method_len = method_len;
    result->path = path;
    result->path_len = path_len;
    result->version = minor_version == 0 ? IHTP_HTTP_10 : IHTP_HTTP_11;
    result->num_headers = num_headers;
}

static void run_pico_response(const char *buf, size_t len, pico_response_result_t *result)
{
    struct phr_header headers[IHTP_MAX_HEADERS];
    size_t num_headers = IHTP_MAX_HEADERS;
    int minor_version = -1;
    int status_code = 0;
    const char *reason = nullptr;
    size_t reason_len = 0;
    int rc = phr_parse_response(buf, len, &minor_version, &status_code, &reason, &reason_len,
                                headers, &num_headers, 0);

    result->status = rc >= 0 ? IHTP_OK : (rc == -2 ? IHTP_INCOMPLETE : IHTP_ERROR);
    result->consumed = rc >= 0 ? (size_t)rc : 0;
    result->status_code = status_code;
    result->reason = reason;
    result->reason_len = reason_len;
    result->version = minor_version == 0 ? IHTP_HTTP_10 : IHTP_HTTP_11;
    result->num_headers = num_headers;
}

static int llhttp_on_method(llhttp_t *parser, const char *at, size_t length)
{
    llhttp_capture_t *capture = parser->data;

    if (capture->method == nullptr) {
        capture->method = at;
    }
    capture->method_len += length;
    return 0;
}

static int llhttp_on_url(llhttp_t *parser, const char *at, size_t length)
{
    llhttp_capture_t *capture = parser->data;

    if (capture->path == nullptr) {
        capture->path = at;
    }
    capture->path_len += length;
    return 0;
}

static int llhttp_on_status(llhttp_t *parser, const char *at, size_t length)
{
    llhttp_capture_t *capture = parser->data;

    if (capture->reason == nullptr) {
        capture->reason = at;
    }
    capture->reason_len += length;
    return 0;
}

static int llhttp_on_header_field_complete(llhttp_t *parser)
{
    llhttp_capture_t *capture = parser->data;

    capture->num_headers++;
    (void)parser;
    return 0;
}

static int llhttp_on_message_complete(llhttp_t *parser)
{
    llhttp_capture_t *capture = parser->data;

    capture->message_complete = true;
    return 0;
}

static ihtp_status_t map_llhttp_status(llhttp_errno_t execute_err, llhttp_errno_t finish_err,
                                       bool message_complete)
{
    if (execute_err == HPE_PAUSED_UPGRADE) {
        return IHTP_OK;
    }
    if (execute_err != HPE_OK) {
        return IHTP_ERROR;
    }
    if (message_complete) {
        return IHTP_OK;
    }
    if (finish_err == HPE_OK) {
        return IHTP_OK;
    }
    if (finish_err == HPE_INVALID_EOF_STATE) {
        return IHTP_INCOMPLETE;
    }
    return IHTP_ERROR;
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
}

static void run_llhttp_request(const char *buf, size_t len, const ihtp_policy_t *policy,
                               llhttp_request_result_t *result)
{
    llhttp_t parser;
    llhttp_settings_t settings;
    llhttp_capture_t capture = {0};
    llhttp_errno_t execute_err;
    llhttp_errno_t finish_err = HPE_OK;

    llhttp_settings_init(&settings);
    settings.on_method = llhttp_on_method;
    settings.on_url = llhttp_on_url;
    settings.on_header_field_complete = llhttp_on_header_field_complete;
    settings.on_message_complete = llhttp_on_message_complete;

    llhttp_init(&parser, HTTP_REQUEST, &settings);
    parser.data = &capture;
    configure_llhttp_policy(&parser, policy);

    execute_err = llhttp_execute(&parser, buf, len);
    if (execute_err == HPE_OK && !capture.message_complete) {
        finish_err = llhttp_finish(&parser);
    }

    result->status = map_llhttp_status(execute_err, finish_err, capture.message_complete);
    if (result->status == IHTP_OK) {
        result->consumed =
            execute_err == HPE_PAUSED_UPGRADE ? (size_t)(llhttp_get_error_pos(&parser) - buf) : len;
    } else {
        result->consumed = 0;
    }
    result->method = capture.method;
    result->method_len = capture.method_len;
    result->path = capture.path;
    result->path_len = capture.path_len;
    result->version = llhttp_get_http_minor(&parser) == 0 ? IHTP_HTTP_10 : IHTP_HTTP_11;
    result->num_headers = capture.num_headers;
}

static void run_llhttp_response(const char *buf, size_t len, const ihtp_policy_t *policy,
                                llhttp_response_result_t *result)
{
    llhttp_t parser;
    llhttp_settings_t settings;
    llhttp_capture_t capture = {0};
    llhttp_errno_t execute_err;
    llhttp_errno_t finish_err = HPE_OK;

    llhttp_settings_init(&settings);
    settings.on_status = llhttp_on_status;
    settings.on_header_field_complete = llhttp_on_header_field_complete;
    settings.on_message_complete = llhttp_on_message_complete;

    llhttp_init(&parser, HTTP_RESPONSE, &settings);
    parser.data = &capture;
    configure_llhttp_policy(&parser, policy);

    execute_err = llhttp_execute(&parser, buf, len);
    if (execute_err == HPE_OK && !capture.message_complete) {
        finish_err = llhttp_finish(&parser);
    }

    result->status = map_llhttp_status(execute_err, finish_err, capture.message_complete);
    if (result->status == IHTP_OK) {
        result->consumed =
            execute_err == HPE_PAUSED_UPGRADE ? (size_t)(llhttp_get_error_pos(&parser) - buf) : len;
    } else {
        result->consumed = 0;
    }
    result->status_code = llhttp_get_status_code(&parser);
    result->reason = capture.reason;
    result->reason_len = capture.reason_len;
    result->version = llhttp_get_http_minor(&parser) == 0 ? IHTP_HTTP_10 : IHTP_HTTP_11;
    result->num_headers = capture.num_headers;
}

static void run_corpus_case(const char *path)
{
    char line[4096];
    char wire_line[8192] = {0};
    char wire[8192] = {0};
    char method[32] = {0};
    char req_path[512] = {0};
    char reason[256] = {0};
    FILE *fp = fopen(path, "r");
    diff_reference_t reference = DIFF_REFERENCE_PICO;
    diff_kind_t kind = DIFF_KIND_REQUEST;
    diff_expect_t reference_expect = DIFF_EXPECT_OK;
    diff_expect_t ihtp_expect = DIFF_EXPECT_OK;
    const ihtp_policy_t *policy = nullptr;
    ihtp_http_version_t version = IHTP_HTTP_11;
    unsigned long long consumed_expect = 0;
    unsigned long long status_code = 0;
    unsigned long long num_headers = 0;
    bool has_reference = false;
    bool has_kind = false;
    bool has_reference_expect = false;
    bool has_ihtp_expect = false;
    bool has_policy = false;
    bool has_wire = false;
    bool has_version = false;
    bool has_consumed = false;
    bool has_method = false;
    bool has_path = false;
    bool has_status_code = false;
    bool has_reason = false;
    bool has_num_headers = false;

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
            reference = parse_reference(value, path);
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
        } else if (str_eq(key, "version")) {
            version = parse_version(value, path);
            has_version = true;
        } else if (str_eq(key, "consumed")) {
            consumed_expect = parse_ull(value, path);
            has_consumed = true;
        } else if (str_eq(key, "method")) {
            strncpy(method, value, sizeof(method) - 1);
            has_method = true;
        } else if (str_eq(key, "path")) {
            strncpy(req_path, value, sizeof(req_path) - 1);
            has_path = true;
        } else if (str_eq(key, "status_code")) {
            status_code = parse_ull(value, path);
            has_status_code = true;
        } else if (str_eq(key, "reason")) {
            strncpy(reason, value, sizeof(reason) - 1);
            has_reason = true;
        } else if (str_eq(key, "num_headers")) {
            num_headers = parse_ull(value, path);
            has_num_headers = true;
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
    TEST_ASSERT_TRUE_MESSAGE(has_version, path);

    (void)reference;

    size_t wire_len = decode_wire(wire_line, wire, sizeof(wire), path);

    if (kind == DIFF_KIND_REQUEST) {
        ihtp_request_t req;
        pico_request_result_t pico;
        llhttp_request_result_t llhttp;
        size_t consumed = 0;
        ihtp_status_t status = ihtp_parse_request(wire, wire_len, &req, policy, &consumed);

        if (reference == DIFF_REFERENCE_PICO) {
            run_pico_request(wire, wire_len, &pico);
            TEST_ASSERT_EQUAL_INT_MESSAGE(expect_to_status(reference_expect), pico.status, path);
        } else if (reference == DIFF_REFERENCE_LLHTTP) {
            run_llhttp_request(wire, wire_len, policy, &llhttp);
            TEST_ASSERT_EQUAL_INT_MESSAGE(expect_to_status(reference_expect), llhttp.status, path);
        }

        TEST_ASSERT_EQUAL_INT_MESSAGE(expect_to_status(ihtp_expect), status, path);

        if (ihtp_expect == DIFF_EXPECT_OK) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_OK, status, path);
            if (reference == DIFF_REFERENCE_PICO) {
                if (reference_expect == DIFF_EXPECT_OK) {
                    TEST_ASSERT_EQUAL_UINT_MESSAGE(pico.consumed, consumed, path);
                    TEST_ASSERT_EQUAL_INT_MESSAGE(pico.version, req.version, path);
                    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(pico.method, req.method_str,
                                                         pico.method_len, path);
                    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(pico.path, req.path, pico.path_len, path);
                    TEST_ASSERT_EQUAL_UINT_MESSAGE(pico.num_headers, req.num_headers, path);
                }
            } else if (reference == DIFF_REFERENCE_LLHTTP) {
                if (reference_expect == DIFF_EXPECT_OK) {
                    TEST_ASSERT_EQUAL_UINT_MESSAGE(llhttp.consumed, consumed, path);
                    TEST_ASSERT_EQUAL_INT_MESSAGE(llhttp.version, req.version, path);
                    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(llhttp.method, req.method_str,
                                                         llhttp.method_len, path);
                    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(llhttp.path, req.path, llhttp.path_len,
                                                         path);
                    TEST_ASSERT_EQUAL_UINT_MESSAGE(llhttp.num_headers, req.num_headers, path);
                }
            }
            TEST_ASSERT_TRUE_MESSAGE(has_consumed, path);
            TEST_ASSERT_TRUE_MESSAGE(has_method, path);
            TEST_ASSERT_TRUE_MESSAGE(has_path, path);
            TEST_ASSERT_TRUE_MESSAGE(has_num_headers, path);
            TEST_ASSERT_EQUAL_UINT_MESSAGE(consumed_expect, consumed, path);
            TEST_ASSERT_EQUAL_INT_MESSAGE(version, req.version, path);
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(method, req.method_str, strlen(method), path);
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(req_path, req.path, strlen(req_path), path);
            TEST_ASSERT_EQUAL_UINT_MESSAGE(num_headers, req.num_headers, path);
            return;
        }

        if (ihtp_expect == DIFF_EXPECT_INCOMPLETE) {
            TEST_ASSERT_EQUAL_UINT_MESSAGE(0, consumed, path);
            return;
        }

        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, consumed, path);
        return;
    }

    ihtp_response_t resp;
    pico_response_result_t pico;
    llhttp_response_result_t llhttp;
    size_t consumed = 0;
    ihtp_status_t status = ihtp_parse_response(wire, wire_len, &resp, policy, &consumed);

    if (reference == DIFF_REFERENCE_PICO) {
        run_pico_response(wire, wire_len, &pico);
        TEST_ASSERT_EQUAL_INT_MESSAGE(expect_to_status(reference_expect), pico.status, path);
    } else if (reference == DIFF_REFERENCE_LLHTTP) {
        run_llhttp_response(wire, wire_len, policy, &llhttp);
        TEST_ASSERT_EQUAL_INT_MESSAGE(expect_to_status(reference_expect), llhttp.status, path);
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(expect_to_status(ihtp_expect), status, path);

    if (ihtp_expect == DIFF_EXPECT_OK) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(IHTP_OK, status, path);
        if (reference == DIFF_REFERENCE_PICO) {
            if (reference_expect == DIFF_EXPECT_OK) {
                TEST_ASSERT_EQUAL_UINT_MESSAGE(pico.consumed, consumed, path);
                TEST_ASSERT_EQUAL_INT_MESSAGE(pico.version, resp.version, path);
                TEST_ASSERT_EQUAL_INT_MESSAGE(pico.status_code, resp.status_code, path);
                TEST_ASSERT_EQUAL_UINT_MESSAGE(pico.num_headers, resp.num_headers, path);
                if (has_reason) {
                    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(pico.reason, resp.reason, pico.reason_len,
                                                         path);
                }
            }
        } else if (reference == DIFF_REFERENCE_LLHTTP) {
            if (reference_expect == DIFF_EXPECT_OK) {
                TEST_ASSERT_EQUAL_UINT_MESSAGE(llhttp.consumed, consumed, path);
                TEST_ASSERT_EQUAL_INT_MESSAGE(llhttp.version, resp.version, path);
                TEST_ASSERT_EQUAL_INT_MESSAGE(llhttp.status_code, resp.status_code, path);
                TEST_ASSERT_EQUAL_UINT_MESSAGE(llhttp.num_headers, resp.num_headers, path);
                if (has_reason) {
                    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(llhttp.reason, resp.reason,
                                                         llhttp.reason_len, path);
                }
            }
        }
        TEST_ASSERT_TRUE_MESSAGE(has_consumed, path);
        TEST_ASSERT_TRUE_MESSAGE(has_status_code, path);
        TEST_ASSERT_TRUE_MESSAGE(has_num_headers, path);
        TEST_ASSERT_EQUAL_UINT_MESSAGE(consumed_expect, consumed, path);
        TEST_ASSERT_EQUAL_INT_MESSAGE(version, resp.version, path);
        TEST_ASSERT_EQUAL_INT_MESSAGE(status_code, resp.status_code, path);
        TEST_ASSERT_EQUAL_UINT_MESSAGE(num_headers, resp.num_headers, path);
        if (has_reason) {
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(reason, resp.reason, strlen(reason), path);
        }
        return;
    }

    if (ihtp_expect == DIFF_EXPECT_INCOMPLETE) {
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, consumed, path);
        return;
    }

    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, consumed, path);
}

void test_differential_corpus_cases(void)
{
    static const char *cases[] = {
        "tests/corpus/differential/pico_request_ok_simple_get.case",
        "tests/corpus/differential/pico_request_ok_connect_authority.case",
        "tests/corpus/differential/pico_request_incomplete_headers.case",
        "tests/corpus/differential/pico_request_strict_reject_bare_lf.case",
        "tests/corpus/differential/pico_request_lenient_accept_bare_lf.case",
        "tests/corpus/differential/pico_response_ok_simple.case",
        "tests/corpus/differential/pico_response_ok_switching_protocols.case",
        "tests/corpus/differential/pico_response_incomplete_headers.case",
        "tests/corpus/differential/pico_response_strict_reject_bare_lf.case",
        "tests/corpus/differential/pico_response_lenient_accept_bare_lf.case",
        "tests/corpus/differential/llhttp_request_ok_simple_get.case",
        "tests/corpus/differential/llhttp_request_ok_connect_authority.case",
        "tests/corpus/differential/llhttp_request_incomplete_headers.case",
        "tests/corpus/differential/llhttp_request_strict_reject_bare_lf.case",
        "tests/corpus/differential/llhttp_request_lenient_accept_bare_lf.case",
        "tests/corpus/differential/llhttp_response_ok_simple.case",
        "tests/corpus/differential/llhttp_response_ok_switching_protocols.case",
        "tests/corpus/differential/llhttp_response_incomplete_headers.case",
        "tests/corpus/differential/llhttp_response_strict_reject_bare_lf.case",
        "tests/corpus/differential/llhttp_response_lenient_accept_bare_lf.case",
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char full_path[1024];
        int written = snprintf(full_path, sizeof(full_path), "%s/%s", IHTP_SOURCE_DIR, cases[i]);

        TEST_ASSERT_GREATER_THAN_INT_MESSAGE(0, written, cases[i]);
        TEST_ASSERT_LESS_THAN_INT_MESSAGE((int)sizeof(full_path), written, cases[i]);
        run_corpus_case(full_path);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_differential_corpus_cases);
    return UNITY_END();
}
