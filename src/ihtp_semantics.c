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
    uint64_t content_length = 0;

    for (size_t i = 0; i < req->num_headers; i++) {
        const ihtp_header_t *h = &req->headers[i];

        if (header_name_eq(h, "transfer-encoding", 17)) {
            has_te = true;
            /* Check if chunked is the final encoding */
            if (h->value_len >= 7) {
                const char *end = h->value + h->value_len - 7;
                if (memcmp(end, "chunked", 7) == 0) {
                    chunked = true;
                }
            }
        } else if (header_name_eq(h, "content-length", 14)) {
            has_cl = true;
            if (!parse_content_length(h->value, h->value_len, &content_length)) {
                return IHTP_ERROR;
            }
        } else if (header_name_eq(h, "connection", 10)) {
            /* Determine keep-alive from Connection header */
            if (h->value_len == 10 && memcmp(h->value, "keep-alive", 10) == 0) {
                req->keep_alive = true;
            } else if (h->value_len == 5 && memcmp(h->value, "close", 5) == 0) {
                req->keep_alive = false;
            }
        }
    }

    /* TE + CL conflict (RFC 9112 Section 6.1) */
    if (has_te && has_cl && policy->reject_te_cl) {
        return IHTP_ERROR;
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
    if (!has_cl && !has_te) {
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
    uint64_t content_length = 0;

    for (size_t i = 0; i < resp->num_headers; i++) {
        const ihtp_header_t *h = &resp->headers[i];

        if (header_name_eq(h, "transfer-encoding", 17)) {
            has_te = true;
            if (h->value_len >= 7) {
                const char *end = h->value + h->value_len - 7;
                if (memcmp(end, "chunked", 7) == 0) {
                    chunked = true;
                }
            }
        } else if (header_name_eq(h, "content-length", 14)) {
            has_cl = true;
            if (!parse_content_length(h->value, h->value_len, &content_length)) {
                return IHTP_ERROR;
            }
        }
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
    if (resp->version == IHTP_HTTP_11) {
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
