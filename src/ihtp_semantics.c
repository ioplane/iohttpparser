/*
 * iohttpparser — Semantics layer (Layer 3)
 * Determines body mode, validates Host, detects TE/CL conflicts.
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#include <iohttpparser/ihtp_parser.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ─── Case-insensitive header name comparison ─────────────────────────── */

static bool header_name_eq(const ihtp_header_t *h, const char *name, size_t name_len)
{
    if (h->name_len != name_len) {
        return false;
    }
    for (size_t i = 0; i < name_len; i++) {
        char a = h->name[i];
        char b = name[i];
        /* Lowercase ASCII letters for comparison */
        if (a >= 'A' && a <= 'Z') {
            a = (char)(a + ('a' - 'A'));
        }
        if (b >= 'A' && b <= 'Z') {
            b = (char)(b + ('a' - 'A'));
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

static bool bytes_eq_ignore_case(const char *buf, size_t len, const char *lit, size_t lit_len)
{
    if (len != lit_len) {
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        char a = buf[i];
        char b = lit[i];

        if (a >= 'A' && a <= 'Z') {
            a = (char)(a + ('a' - 'A'));
        }
        if (b >= 'A' && b <= 'Z') {
            b = (char)(b + ('a' - 'A'));
        }
        if (a != b) {
            return false;
        }
    }

    return true;
}

/* ─── Parse Content-Length value ───────────────────────────────────────── */

static bool parse_content_length(const char *value, size_t len, uint64_t *out)
{
    if (len == 0) {
        return false;
    }

    uint64_t result = 0;
    for (size_t i = 0; i < len; i++) {
        if (!ihtp_is_digit((uint8_t)value[i])) {
            return false;
        }
        uint64_t digit = (uint64_t)(value[i] - '0');
        /* Overflow check */
        if (result > (UINT64_MAX - digit) / 10) {
            return false;
        }
        result = result * 10 + digit;
    }

    *out = result;
    return true;
}

static bool parse_transfer_encoding(const char *value, size_t len, bool *ends_with_chunked)
{
    size_t pos = 0;
    bool saw_coding = false;
    bool final_chunked = false;

    while (pos < len) {
        size_t part_start = pos;

        while (pos < len && value[pos] != ',') {
            pos++;
        }

        size_t part_len = pos - part_start;
        const char *part = value + part_start;

        while (part_len > 0 && ihtp_is_lws((uint8_t)*part)) {
            part++;
            part_len--;
        }
        while (part_len > 0 && ihtp_is_lws((uint8_t)part[part_len - 1])) {
            part_len--;
        }
        if (part_len == 0) {
            return false;
        }

        size_t coding_len = part_len;
        const char *params = memchr(part, ';', part_len);
        if (params != nullptr) {
            coding_len = (size_t)(params - part);
            while (coding_len > 0 && ihtp_is_lws((uint8_t)part[coding_len - 1])) {
                coding_len--;
            }
            if (coding_len == 0) {
                return false;
            }
        }
        if (!ihtp_scanner_get()->is_token(part, coding_len)) {
            return false;
        }

        final_chunked = bytes_eq_ignore_case(part, coding_len, "chunked", 7);
        saw_coding = true;

        if (pos < len && value[pos] == ',') {
            pos++;
            if (pos == len) {
                return false;
            }
        }
    }

    if (!saw_coding) {
        return false;
    }

    *ends_with_chunked = final_chunked;
    return true;
}

static bool parse_connection_header(const char *value, size_t len, bool *has_close,
                                    bool *has_keep_alive)
{
    size_t pos = 0;

    while (pos < len) {
        size_t part_start = pos;

        while (pos < len && value[pos] != ',') {
            pos++;
        }

        size_t part_len = pos - part_start;
        const char *part = value + part_start;

        while (part_len > 0 && ihtp_is_lws((uint8_t)*part)) {
            part++;
            part_len--;
        }
        while (part_len > 0 && ihtp_is_lws((uint8_t)part[part_len - 1])) {
            part_len--;
        }
        if (part_len == 0 || !ihtp_scanner_get()->is_token(part, part_len)) {
            return false;
        }

        if (bytes_eq_ignore_case(part, part_len, "close", 5)) {
            *has_close = true;
        } else if (bytes_eq_ignore_case(part, part_len, "keep-alive", 10)) {
            *has_keep_alive = true;
        }

        if (pos < len && value[pos] == ',') {
            pos++;
            if (pos == len) {
                return false;
            }
        }
    }

    return true;
}

/* ─── Apply semantics to parsed request ───────────────────────────────── */

ihtp_status_t ihtp_request_apply_semantics(ihtp_request_t *req, const ihtp_policy_t *policy)
{
    if (req == nullptr) {
        return IHTP_ERROR;
    }

    static const ihtp_policy_t strict = IHTP_POLICY_STRICT;
    if (policy == nullptr) {
        policy = &strict;
    }

    bool has_te = false;
    bool has_cl = false;
    bool chunked = false;
    bool has_connection = false;
    bool has_host = false;
    bool connection_close = false;
    bool connection_keep_alive = false;
    uint64_t content_length = 0;

    for (size_t i = 0; i < req->num_headers; i++) {
        const ihtp_header_t *h = &req->headers[i];

        if (header_name_eq(h, "transfer-encoding", 17)) {
            bool header_chunked = false;

            has_te = true;
            if (!parse_transfer_encoding(h->value, h->value_len, &header_chunked)) {
                return IHTP_ERROR;
            }
            chunked = header_chunked;
        } else if (header_name_eq(h, "content-length", 14)) {
            uint64_t parsed_content_length = 0;

            if (!parse_content_length(h->value, h->value_len, &parsed_content_length)) {
                return IHTP_ERROR;
            }
            if (has_cl && parsed_content_length != content_length) {
                return IHTP_ERROR;
            }
            has_cl = true;
            content_length = parsed_content_length;
        } else if (header_name_eq(h, "connection", 10)) {
            if (!parse_connection_header(h->value, h->value_len, &connection_close,
                                         &connection_keep_alive)) {
                return IHTP_ERROR;
            }
        } else if (header_name_eq(h, "host", 4)) {
            if (has_host || h->value_len == 0) {
                return IHTP_ERROR;
            }
            has_host = true;
        }
    }

    /* TE + CL conflict (RFC 9112 Section 6.1) */
    if (has_te && has_cl && policy->reject_te_cl) {
        return IHTP_ERROR;
    }

    if (req->version == IHTP_HTTP_11 && !has_host) {
        return IHTP_ERROR;
    }
    if (has_te && !chunked) {
        return IHTP_ERROR;
    }
    if (connection_close) {
        req->keep_alive = false;
        has_connection = true;
    } else if (connection_keep_alive) {
        req->keep_alive = true;
        has_connection = true;
    }

    /* Determine body mode */
    if (has_te && chunked) {
        req->body_mode = IHTP_BODY_CHUNKED;
    } else if (has_cl) {
        req->body_mode = IHTP_BODY_FIXED;
        req->content_length = content_length;
    } else {
        req->body_mode = IHTP_BODY_NONE;
    }

    /* Default keep-alive based on HTTP version */
    if (!has_connection) {
        /* No explicit Connection header decision — use version default */
        if (req->version == IHTP_HTTP_11) {
            req->keep_alive = true;
        } else {
            req->keep_alive = false;
        }
    }

    return IHTP_OK;
}

/* ─── Apply semantics to parsed response ──────────────────────────────── */

ihtp_status_t ihtp_response_apply_semantics(ihtp_response_t *resp, const ihtp_policy_t *policy)
{
    if (resp == nullptr) {
        return IHTP_ERROR;
    }

    static const ihtp_policy_t strict = IHTP_POLICY_STRICT;
    if (policy == nullptr) {
        policy = &strict;
    }

    bool has_te = false;
    bool has_cl = false;
    bool chunked = false;
    bool has_connection = false;
    bool connection_close = false;
    bool connection_keep_alive = false;
    uint64_t content_length = 0;

    for (size_t i = 0; i < resp->num_headers; i++) {
        const ihtp_header_t *h = &resp->headers[i];

        if (header_name_eq(h, "transfer-encoding", 17)) {
            bool header_chunked = false;

            has_te = true;
            if (!parse_transfer_encoding(h->value, h->value_len, &header_chunked)) {
                return IHTP_ERROR;
            }
            chunked = header_chunked;
        } else if (header_name_eq(h, "content-length", 14)) {
            uint64_t parsed_content_length = 0;

            if (!parse_content_length(h->value, h->value_len, &parsed_content_length)) {
                return IHTP_ERROR;
            }
            if (has_cl && parsed_content_length != content_length) {
                return IHTP_ERROR;
            }
            has_cl = true;
            content_length = parsed_content_length;
        } else if (header_name_eq(h, "connection", 10)) {
            if (!parse_connection_header(h->value, h->value_len, &connection_close,
                                         &connection_keep_alive)) {
                return IHTP_ERROR;
            }
        }
    }

    if (has_te && has_cl && policy->reject_te_cl) {
        return IHTP_ERROR;
    }
    if (connection_close) {
        resp->keep_alive = false;
        has_connection = true;
    } else if (connection_keep_alive) {
        resp->keep_alive = true;
        has_connection = true;
    }

    /* 1xx, 204, 304 have no body */
    if ((resp->status_code >= 100 && resp->status_code < 200) || resp->status_code == 204 ||
        resp->status_code == 304) {
        resp->body_mode = IHTP_BODY_NONE;
    } else if (has_te && chunked) {
        resp->body_mode = IHTP_BODY_CHUNKED;
    } else if (has_cl) {
        resp->body_mode = IHTP_BODY_FIXED;
        resp->content_length = content_length;
    } else {
        resp->body_mode = IHTP_BODY_EOF;
    }

    /* Default keep-alive */
    if (!has_connection && resp->version == IHTP_HTTP_11) {
        resp->keep_alive = true;
    }

    return IHTP_OK;
}

/* ─── Version functions ───────────────────────────────────────────────── */

const char *ihtp_version(void)
{
    return IHTP_VERSION_STRING;
}

int ihtp_version_num(void)
{
    return (IHTP_VERSION_MAJOR << 16) | (IHTP_VERSION_MINOR << 8) | IHTP_VERSION_PATCH;
}
