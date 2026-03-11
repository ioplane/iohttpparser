/*
 * iohttpparser — Parser (Layer 2: request-line + headers)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#include <iohttpparser/ihtp_parser.h>

#include <string.h>

/* ─── Method lookup ───────────────────────────────────────────────────── */

ihtp_method_t ihtp_method_from_str(const char *method, size_t method_len)
{
    switch (method_len) {
    case 3:
        if (memcmp(method, "GET", 3) == 0) {
            return IHTP_METHOD_GET;
        }
        if (memcmp(method, "PUT", 3) == 0) {
            return IHTP_METHOD_PUT;
        }
        break;
    case 4:
        if (memcmp(method, "POST", 4) == 0) {
            return IHTP_METHOD_POST;
        }
        if (memcmp(method, "HEAD", 4) == 0) {
            return IHTP_METHOD_HEAD;
        }
        break;
    case 5:
        if (memcmp(method, "PATCH", 5) == 0) {
            return IHTP_METHOD_PATCH;
        }
        if (memcmp(method, "TRACE", 5) == 0) {
            return IHTP_METHOD_TRACE;
        }
        break;
    case 6:
        if (memcmp(method, "DELETE", 6) == 0) {
            return IHTP_METHOD_DELETE;
        }
        break;
    case 7:
        if (memcmp(method, "OPTIONS", 7) == 0) {
            return IHTP_METHOD_OPTIONS;
        }
        if (memcmp(method, "CONNECT", 7) == 0) {
            return IHTP_METHOD_CONNECT;
        }
        break;
    default:
        break;
    }
    return IHTP_METHOD_UNKNOWN;
}

const char *ihtp_method_to_str(ihtp_method_t method)
{
    switch (method) {
    case IHTP_METHOD_GET:
        return "GET";
    case IHTP_METHOD_POST:
        return "POST";
    case IHTP_METHOD_PUT:
        return "PUT";
    case IHTP_METHOD_DELETE:
        return "DELETE";
    case IHTP_METHOD_HEAD:
        return "HEAD";
    case IHTP_METHOD_OPTIONS:
        return "OPTIONS";
    case IHTP_METHOD_PATCH:
        return "PATCH";
    case IHTP_METHOD_CONNECT:
        return "CONNECT";
    case IHTP_METHOD_TRACE:
        return "TRACE";
    default:
        break;
    }
    return "UNKNOWN";
}

void ihtp_parser_state_init(ihtp_parser_state_t *state, ihtp_parser_mode_t mode)
{
    if (state == nullptr) {
        return;
    }

    state->mode = mode;
    state->phase = mode == IHTP_PARSER_MODE_HEADERS ? IHTP_PARSER_PHASE_HEADERS
                                                    : IHTP_PARSER_PHASE_START_LINE;
    state->cursor = 0;
}

void ihtp_parser_state_reset(ihtp_parser_state_t *state)
{
    if (state == nullptr) {
        return;
    }

    state->phase = state->mode == IHTP_PARSER_MODE_HEADERS ? IHTP_PARSER_PHASE_HEADERS
                                                           : IHTP_PARSER_PHASE_START_LINE;
    state->cursor = 0;
}

/* ─── Internal: find line ending ──────────────────────────────────────── */

static ihtp_status_t find_line_end(const char *buf, size_t len, const ihtp_policy_t *policy,
                                   const char **line_end, size_t *line_ending_len)
{
    const char *lf = memchr(buf, '\n', len);

    if (lf == nullptr) {
        return IHTP_INCOMPLETE;
    }

    if (lf > buf && lf[-1] == '\r') {
        *line_end = lf - 1;
        *line_ending_len = 2;
        return IHTP_OK;
    }

    if (policy != nullptr && policy->reject_bare_lf) {
        return IHTP_ERROR;
    }

    *line_end = lf;
    *line_ending_len = 1;
    return IHTP_OK;
}

static bool find_char_offset(const char *buf, size_t len, int ch, size_t *offset)
{
    const char *pos = memchr(buf, ch, len);

    if (pos == nullptr) {
        return false;
    }

    *offset = (size_t)(pos - buf);
    return true;
}

static bool find_header_name_colon(const char *buf, size_t len, size_t *colon_offset)
{
    for (size_t i = 0; i < len; i++) {
        uint8_t c = (uint8_t)buf[i];

        if (c == ':') {
            if (i == 0) {
                return false;
            }
            *colon_offset = i;
            return true;
        }

        if (!ihtp_is_token_char(c)) {
            return false;
        }
    }

    return false;
}

static bool find_last_char_offset(const char *buf, size_t len, int ch, size_t *offset)
{
    const char *pos = memrchr(buf, ch, len);

    if (pos == nullptr) {
        return false;
    }

    *offset = (size_t)(pos - buf);
    return true;
}

static bool request_target_is_valid(const char *buf, size_t len, bool allow_spaces)
{
    for (size_t i = 0; i < len; i++) {
        uint8_t c = (uint8_t)buf[i];

        if (c == ' ' && allow_spaces) {
            continue;
        }

        if (c <= 0x20 || c == 0x7f) {
            return false;
        }
    }

    return true;
}

static bool field_text_is_valid(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        uint8_t c = (uint8_t)buf[i];

        if (c == '\t' || c == ' ') {
            continue;
        }

        if (c < 0x20 || c == 0x7f) {
            return false;
        }
    }

    return true;
}

static bool trim_and_validate_field_value(const char *buf, size_t len, const char **trimmed_start,
                                          size_t *trimmed_len)
{
    size_t first = len;
    size_t last = 0;

    for (size_t i = 0; i < len; i++) {
        uint8_t c = (uint8_t)buf[i];

        if (c == '\t' || c == ' ') {
            continue;
        }

        if (c < 0x20 || c == 0x7f) {
            return false;
        }

        if (first == len) {
            first = i;
        }
        last = i + 1;
    }

    if (first == len) {
        *trimmed_start = buf + len;
        *trimmed_len = 0;
        return true;
    }

    *trimmed_start = buf + first;
    *trimmed_len = last - first;
    return true;
}

static ihtp_status_t parse_status_line(const char *buf, size_t len, ihtp_response_t *resp,
                                       const ihtp_policy_t *policy, size_t *line_end)
{
    const char *line_break = nullptr;
    size_t line_ending_len = 0;
    ihtp_status_t line_status = find_line_end(buf, len, policy, &line_break, &line_ending_len);

    if (line_status != IHTP_OK) {
        return line_status;
    }

    size_t line_len = (size_t)(line_break - buf);

    if (line_len < 13 || memcmp(buf, "HTTP/1.", 7) != 0) {
        return IHTP_ERROR;
    }

    if (buf[7] == '1') {
        resp->version = IHTP_HTTP_11;
    } else if (buf[7] == '0') {
        resp->version = IHTP_HTTP_10;
    } else {
        return IHTP_ERROR;
    }

    if (buf[8] != ' ') {
        return IHTP_ERROR;
    }

    if (!ihtp_is_digit((uint8_t)buf[9]) || !ihtp_is_digit((uint8_t)buf[10]) ||
        !ihtp_is_digit((uint8_t)buf[11])) {
        return IHTP_ERROR;
    }
    resp->status_code = (buf[9] - '0') * 100 + (buf[10] - '0') * 10 + (buf[11] - '0');
    if (resp->status_code < 100 || resp->status_code > 599) {
        return IHTP_ERROR;
    }

    if (buf[12] != ' ') {
        return IHTP_ERROR;
    }

    if (line_len > 13) {
        resp->reason = buf + 13;
        resp->reason_len = (size_t)(line_break - (buf + 13));
        if (!field_text_is_valid(resp->reason, resp->reason_len)) {
            return IHTP_ERROR;
        }
    } else {
        resp->reason = buf + 13;
        resp->reason_len = 0;
    }

    *line_end = line_len + line_ending_len;
    return IHTP_OK;
}

/* ─── Parse request-line ──────────────────────────────────────────────── */

static ihtp_status_t parse_request_line(const char *buf, size_t len, ihtp_request_t *req,
                                        const ihtp_policy_t *policy, size_t *line_end)
{
    const ihtp_scanner_vtable_t *scanner = ihtp_scanner_get();
    const char *line_break = nullptr;
    size_t line_ending_len = 0;

    /* Find end of request-line */
    ihtp_status_t line_status = find_line_end(buf, len, policy, &line_break, &line_ending_len);
    if (line_status == IHTP_INCOMPLETE) {
        if (len > IHTP_MAX_REQUEST_LINE) {
            return IHTP_ERROR_TOO_LONG;
        }
        return IHTP_INCOMPLETE;
    }
    if (line_status != IHTP_OK) {
        return line_status;
    }

    size_t line_len = (size_t)(line_break - buf);
    if (line_len > IHTP_MAX_REQUEST_LINE) {
        return IHTP_ERROR_TOO_LONG;
    }

    /* Parse: METHOD SP request-target SP HTTP-version */
    size_t sp1_offset = 0;
    if (!find_char_offset(buf, line_len, ' ', &sp1_offset)) {
        return IHTP_ERROR;
    }
    const char *sp1 = buf + sp1_offset;

    req->method_str = buf;
    req->method_len = sp1_offset;
    if (req->method_len == 0) {
        return IHTP_ERROR;
    }

    /* Validate method is token */
    if (!scanner->is_token(req->method_str, req->method_len)) {
        return IHTP_ERROR;
    }
    req->method = ihtp_method_from_str(req->method_str, req->method_len);

    /* Request-target */
    const char *path_start = sp1 + 1;
    size_t sp2_offset = 0;
    size_t remaining = line_len - req->method_len - 1;
    if (!find_last_char_offset(path_start, remaining, ' ', &sp2_offset)) {
        return IHTP_ERROR;
    }
    const char *sp2 = path_start + sp2_offset;

    req->path = path_start;
    req->path_len = sp2_offset;
    if (req->path_len == 0) {
        return IHTP_ERROR;
    }
    if (!request_target_is_valid(req->path, req->path_len,
                                 policy != nullptr && policy->allow_spaces_in_uri)) {
        return IHTP_ERROR;
    }

    /* HTTP-version: "HTTP/1.0" or "HTTP/1.1" */
    const char *ver = sp2 + 1;
    size_t ver_len = (size_t)(line_break - ver);
    if (ver_len != 8 || memcmp(ver, "HTTP/1.", 7) != 0) {
        return IHTP_ERROR;
    }
    if (ver[7] == '1') {
        req->version = IHTP_HTTP_11;
    } else if (ver[7] == '0') {
        req->version = IHTP_HTTP_10;
    } else {
        return IHTP_ERROR;
    }

    *line_end = line_len + line_ending_len;
    return IHTP_OK;
}

/* ─── Parse headers ───────────────────────────────────────────────────── */

static ihtp_status_t parse_header_block(const char *buf, size_t len, ihtp_header_t *headers,
                                        size_t initial_count, size_t *num_headers,
                                        size_t max_headers, const ihtp_policy_t *policy,
                                        size_t *block_end)
{
    size_t count = initial_count;
    size_t pos = 0;

    while (pos < len) {
        bool is_continuation = ihtp_is_lws((uint8_t)buf[pos]);
        if (policy != nullptr && policy->reject_obs_fold && is_continuation) {
            return IHTP_ERROR;
        }

        /* Find line ending for this header line */
        const char *line_break = nullptr;
        size_t line_ending_len = 0;
        ihtp_status_t line_status =
            find_line_end(buf + pos, len - pos, policy, &line_break, &line_ending_len);
        if (line_status == IHTP_INCOMPLETE) {
            if (len - pos > IHTP_MAX_HEADER_LINE) {
                return IHTP_ERROR_TOO_LONG;
            }
            *num_headers = count;
            *block_end = pos;
            return IHTP_INCOMPLETE;
        }
        if (line_status != IHTP_OK) {
            return line_status;
        }

        size_t line_len = (size_t)(line_break - (buf + pos));
        if (line_len > IHTP_MAX_HEADER_LINE) {
            return IHTP_ERROR_TOO_LONG;
        }
        if (line_len == 0) {
            *block_end = pos + line_ending_len;
            *num_headers = count;
            return IHTP_OK;
        }

        if (is_continuation) {
            if (count == 0) {
                return IHTP_ERROR;
            }
            if (!field_text_is_valid(buf + pos, line_len)) {
                return IHTP_ERROR;
            }

            headers[count - 1].value_len = (size_t)(line_break - headers[count - 1].value);
            pos += line_len + line_ending_len;
            continue;
        }

        /* Parse: field-name ":" OWS field-value OWS */
        const char *line_start = buf + pos;
        size_t colon_offset = 0;
        if (!find_header_name_colon(line_start, line_len, &colon_offset)) {
            return IHTP_ERROR;
        }
        const char *colon = line_start + colon_offset;

        if (count >= max_headers) {
            return IHTP_ERROR_TOO_MANY_HEADERS;
        }

        headers[count].name = line_start;
        headers[count].name_len = colon_offset;

        /* Skip ": " and trim OWS from value */
        const char *val_start = colon + 1;
        size_t val_len = (size_t)(line_break - val_start);
        if (!trim_and_validate_field_value(val_start, val_len, &val_start, &val_len)) {
            return IHTP_ERROR;
        }

        headers[count].value = val_start;
        headers[count].value_len = val_len;
        count++;

        pos += line_len + line_ending_len;
    }

    *num_headers = count;
    *block_end = pos;
    return IHTP_INCOMPLETE;
}

static const ihtp_policy_t *resolve_policy(const ihtp_policy_t *policy)
{
    static const ihtp_policy_t strict = IHTP_POLICY_STRICT;
    if (policy == nullptr) {
        return &strict;
    }

    return policy;
}

static ihtp_status_t parser_state_error(ihtp_parser_state_t *state)
{
    if (state != nullptr) {
        state->phase = IHTP_PARSER_PHASE_ERROR;
    }

    return IHTP_ERROR;
}

/* ─── Internal stateful parser entry points ───────────────────────────── */

ihtp_status_t ihtp_parse_request_stateful(ihtp_parser_state_t *state, const char *buf, size_t len,
                                          ihtp_request_t *req, const ihtp_policy_t *policy,
                                          size_t *bytes_consumed)
{
    if (state == nullptr || buf == nullptr || req == nullptr || bytes_consumed == nullptr ||
        state->mode != IHTP_PARSER_MODE_REQUEST) {
        return IHTP_ERROR;
    }

    policy = resolve_policy(policy);
    *bytes_consumed = 0;

    if (state->phase == IHTP_PARSER_PHASE_ERROR) {
        return IHTP_ERROR;
    }
    if (state->phase == IHTP_PARSER_PHASE_DONE) {
        *bytes_consumed = state->cursor;
        return IHTP_OK;
    }

    if (state->phase == IHTP_PARSER_PHASE_START_LINE) {
        size_t line_end = 0;
        ihtp_status_t status =
            parse_request_line(buf + state->cursor, len - state->cursor, req, policy, &line_end);
        if (status != IHTP_OK) {
            if (status == IHTP_ERROR || status == IHTP_ERROR_TOO_LONG) {
                state->phase = IHTP_PARSER_PHASE_ERROR;
            }
            return status;
        }

        state->cursor += line_end;
        state->phase = IHTP_PARSER_PHASE_HEADERS;
    }

    if (state->phase == IHTP_PARSER_PHASE_HEADERS) {
        size_t block_end = 0;
        ihtp_status_t status = parse_header_block(buf + state->cursor, len - state->cursor,
                                                  req->headers, req->num_headers, &req->num_headers,
                                                  IHTP_MAX_HEADERS, policy, &block_end);
        state->cursor += block_end;
        if (status == IHTP_INCOMPLETE) {
            return status;
        }
        if (status != IHTP_OK) {
            state->phase = IHTP_PARSER_PHASE_ERROR;
            return status;
        }

        state->phase = IHTP_PARSER_PHASE_DONE;
    }

    *bytes_consumed = state->cursor;
    return IHTP_OK;
}

ihtp_status_t ihtp_parse_response_stateful(ihtp_parser_state_t *state, const char *buf, size_t len,
                                           ihtp_response_t *resp, const ihtp_policy_t *policy,
                                           size_t *bytes_consumed)
{
    if (state == nullptr || buf == nullptr || resp == nullptr || bytes_consumed == nullptr ||
        state->mode != IHTP_PARSER_MODE_RESPONSE) {
        return IHTP_ERROR;
    }

    policy = resolve_policy(policy);
    *bytes_consumed = 0;

    if (state->phase == IHTP_PARSER_PHASE_ERROR) {
        return IHTP_ERROR;
    }
    if (state->phase == IHTP_PARSER_PHASE_DONE) {
        *bytes_consumed = state->cursor;
        return IHTP_OK;
    }

    if (state->phase == IHTP_PARSER_PHASE_START_LINE) {
        size_t line_end = 0;
        ihtp_status_t status =
            parse_status_line(buf + state->cursor, len - state->cursor, resp, policy, &line_end);
        if (status != IHTP_OK) {
            if (status == IHTP_ERROR) {
                state->phase = IHTP_PARSER_PHASE_ERROR;
            }
            return status;
        }

        state->cursor += line_end;
        state->phase = IHTP_PARSER_PHASE_HEADERS;
    }

    if (state->phase == IHTP_PARSER_PHASE_HEADERS) {
        size_t block_end = 0;
        ihtp_status_t status = parse_header_block(buf + state->cursor, len - state->cursor,
                                                  resp->headers, resp->num_headers,
                                                  &resp->num_headers, IHTP_MAX_HEADERS, policy,
                                                  &block_end);
        state->cursor += block_end;
        if (status == IHTP_INCOMPLETE) {
            return status;
        }
        if (status != IHTP_OK) {
            state->phase = IHTP_PARSER_PHASE_ERROR;
            return status;
        }

        state->phase = IHTP_PARSER_PHASE_DONE;
    }

    *bytes_consumed = state->cursor;
    return IHTP_OK;
}

ihtp_status_t ihtp_parse_headers_stateful(ihtp_parser_state_t *state, const char *buf, size_t len,
                                          ihtp_header_t *headers, size_t *num_headers,
                                          size_t max_headers, const ihtp_policy_t *policy,
                                          size_t *bytes_consumed)
{
    if (state == nullptr || buf == nullptr || headers == nullptr || num_headers == nullptr ||
        bytes_consumed == nullptr || state->mode != IHTP_PARSER_MODE_HEADERS) {
        return IHTP_ERROR;
    }

    policy = resolve_policy(policy);
    *bytes_consumed = 0;

    if (state->phase == IHTP_PARSER_PHASE_ERROR) {
        return IHTP_ERROR;
    }
    if (state->phase == IHTP_PARSER_PHASE_DONE) {
        *bytes_consumed = state->cursor;
        return IHTP_OK;
    }

    if (state->phase != IHTP_PARSER_PHASE_HEADERS) {
        return parser_state_error(state);
    }

    size_t block_end = 0;
    ihtp_status_t status = parse_header_block(buf + state->cursor, len - state->cursor, headers,
                                              *num_headers, num_headers, max_headers, policy,
                                              &block_end);
    state->cursor += block_end;
    if (status == IHTP_INCOMPLETE) {
        return status;
    }
    if (status != IHTP_OK) {
        state->phase = IHTP_PARSER_PHASE_ERROR;
        return status;
    }

    state->phase = IHTP_PARSER_PHASE_DONE;
    *bytes_consumed = state->cursor;
    return IHTP_OK;
}

/* ─── Public: parse request ───────────────────────────────────────────── */

ihtp_status_t ihtp_parse_request(const char *buf, size_t len, ihtp_request_t *req,
                                 const ihtp_policy_t *policy, size_t *bytes_consumed)
{
    if (buf == nullptr || req == nullptr || bytes_consumed == nullptr) {
        return IHTP_ERROR;
    }

    memset(req, 0, sizeof(*req));

    ihtp_parser_state_t state;
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    return ihtp_parse_request_stateful(&state, buf, len, req, policy, bytes_consumed);
}

/* ─── Public: parse response ──────────────────────────────────────────── */

ihtp_status_t ihtp_parse_response(const char *buf, size_t len, ihtp_response_t *resp,
                                  const ihtp_policy_t *policy, size_t *bytes_consumed)
{
    if (buf == nullptr || resp == nullptr || bytes_consumed == nullptr) {
        return IHTP_ERROR;
    }

    memset(resp, 0, sizeof(*resp));

    ihtp_parser_state_t state;
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);
    return ihtp_parse_response_stateful(&state, buf, len, resp, policy, bytes_consumed);
}

/* ─── Public: parse headers only ──────────────────────────────────────── */

ihtp_status_t ihtp_parse_headers(const char *buf, size_t len, ihtp_header_t *headers,
                                 size_t *num_headers, const ihtp_policy_t *policy,
                                 size_t *bytes_consumed)
{
    if (buf == nullptr || headers == nullptr || num_headers == nullptr ||
        bytes_consumed == nullptr) {
        return IHTP_ERROR;
    }

    size_t max_headers = *num_headers;
    *num_headers = 0;

    ihtp_parser_state_t state;
    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_HEADERS);
    return ihtp_parse_headers_stateful(&state, buf, len, headers, num_headers, max_headers, policy,
                                       bytes_consumed);
}
