/*
 * iohttpparser — Public type definitions
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_TYPES_H
#define IOHTTPPARSER_IHTP_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Maximum number of headers per request/response.
 *
 * Configurable at compile time via -DHP_MAX_HEADERS=N.
 */
#ifndef IHTP_MAX_HEADERS
#    define IHTP_MAX_HEADERS 64
#endif

/**
 * @brief Maximum length of a single header line (name + ": " + value).
 */
#ifndef IHTP_MAX_HEADER_LINE
#    define IHTP_MAX_HEADER_LINE 8192
#endif

/**
 * @brief Maximum length of the request-line (method + SP + target + SP + version + CRLF).
 */
#ifndef IHTP_MAX_REQUEST_LINE
#    define IHTP_MAX_REQUEST_LINE 8192
#endif

/**
 * @brief Parser return status.
 */
typedef enum {
    IHTP_OK = 0,                      /**< Parsing complete, result available */
    IHTP_INCOMPLETE = -2,             /**< Need more data (call again with more bytes) */
    IHTP_ERROR = -1,                  /**< Parse error (malformed input) */
    IHTP_ERROR_TOO_LONG = -3,         /**< Request-line or header exceeds size limit */
    IHTP_ERROR_TOO_MANY_HEADERS = -4, /**< Header count exceeds IHTP_MAX_HEADERS */
} ihtp_status_t;

/**
 * @brief HTTP method enum (common methods for fast comparison).
 */
typedef enum {
    IHTP_METHOD_UNKNOWN = 0,
    IHTP_METHOD_GET,
    IHTP_METHOD_POST,
    IHTP_METHOD_PUT,
    IHTP_METHOD_DELETE,
    IHTP_METHOD_HEAD,
    IHTP_METHOD_OPTIONS,
    IHTP_METHOD_PATCH,
    IHTP_METHOD_CONNECT,
    IHTP_METHOD_TRACE,
} ihtp_method_t;

/**
 * @brief HTTP version.
 */
typedef enum {
    IHTP_HTTP_10 = 0,
    IHTP_HTTP_11 = 1,
} ihtp_http_version_t;

/**
 * @brief Stateful parser mode.
 */
typedef enum {
    IHTP_PARSER_MODE_REQUEST = 0,
    IHTP_PARSER_MODE_RESPONSE = 1,
    IHTP_PARSER_MODE_HEADERS = 2,
} ihtp_parser_mode_t;

/**
 * @brief Public parser phase for stateful incremental parsing.
 */
typedef enum {
    IHTP_PARSER_PHASE_START_LINE = 0,
    IHTP_PARSER_PHASE_HEADERS = 1,
    IHTP_PARSER_PHASE_DONE = 2,
    IHTP_PARSER_PHASE_ERROR = 3,
} ihtp_parser_phase_t;

/**
 * @brief Stateful parser object for incremental request/response/header parsing.
 *
 * Zero-initialize before first use or call ihtp_parser_state_init().
 */
typedef struct {
    ihtp_parser_mode_t mode;   /**< Parser mode: request, response, or headers */
    ihtp_parser_phase_t phase; /**< Current parser phase */
    size_t cursor;             /**< Bytes consumed inside the accumulated input buffer */
} ihtp_parser_state_t;

/**
 * @brief A single HTTP header (name-value pair).
 *
 * Pointers reference the original input buffer (zero-copy).
 * Valid only while the input buffer is alive.
 */
typedef struct {
    const char *name;  /**< Header name (not NUL-terminated) */
    size_t name_len;   /**< Length of name */
    const char *value; /**< Header value (not NUL-terminated) */
    size_t value_len;  /**< Length of value */
} ihtp_header_t;

/**
 * @brief Body transfer mode (determined by semantics layer).
 */
typedef enum {
    IHTP_BODY_NONE = 0,    /**< No body (HEAD response, 204, 304, etc.) */
    IHTP_BODY_FIXED = 1,   /**< Content-Length present */
    IHTP_BODY_CHUNKED = 2, /**< Transfer-Encoding: chunked */
    IHTP_BODY_EOF = 3,     /**< Read until connection close (HTTP/1.0) */
} ihtp_body_mode_t;

/**
 * @brief Parsed HTTP request (Layer 2 output).
 *
 * All string pointers reference the original input buffer.
 */
typedef struct {
    ihtp_method_t method;   /**< Parsed method enum */
    const char *method_str; /**< Raw method string */
    size_t method_len;      /**< Length of method string */

    const char *path; /**< Request target (path) */
    size_t path_len;  /**< Length of path */

    ihtp_http_version_t version; /**< HTTP version (1.0 or 1.1) */

    ihtp_header_t headers[IHTP_MAX_HEADERS]; /**< Parsed headers */
    size_t num_headers;                      /**< Number of parsed headers */

    ihtp_body_mode_t body_mode; /**< Body mode (set by semantics layer) */
    uint64_t content_length;    /**< Content-Length value (if body_mode == IHTP_BODY_FIXED) */
    bool keep_alive;            /**< Connection persistence */
    bool protocol_upgrade;      /**< Connection switches protocols after headers */
    bool expects_continue;      /**< Request carries Expect: 100-continue */
    bool has_trailer_fields;    /**< Trailer header advertises trailing fields */
} ihtp_request_t;

/**
 * @brief Parsed HTTP response (Layer 2 output).
 */
typedef struct {
    ihtp_http_version_t version; /**< HTTP version */
    int status_code;             /**< Status code (100-599) */
    const char *reason;          /**< Reason phrase */
    size_t reason_len;           /**< Length of reason phrase */

    ihtp_header_t headers[IHTP_MAX_HEADERS]; /**< Parsed headers */
    size_t num_headers;                      /**< Number of parsed headers */

    ihtp_body_mode_t body_mode; /**< Body mode (set by semantics layer) */
    uint64_t content_length;    /**< Content-Length value (if body_mode == IHTP_BODY_FIXED) */
    bool keep_alive;            /**< Connection persistence */
    bool protocol_upgrade;      /**< Response switches protocols after headers */
    bool has_trailer_fields;    /**< Trailer header advertises trailing fields */
} ihtp_response_t;

/**
 * @brief Parser policy (strict vs lenient).
 */
typedef struct {
    bool reject_obs_fold;     /**< Reject obs-fold in header values (default: true) */
    bool reject_bare_lf;      /**< Reject bare LF without CR (default: true) */
    bool reject_te_cl;        /**< Reject requests with both TE and CL (default: true) */
    bool allow_spaces_in_uri; /**< Allow unencoded spaces in URI (default: false) */
} ihtp_policy_t;

/**
 * @brief Default strict policy (RFC 9112 compliant).
 */
#define IHTP_POLICY_STRICT ((ihtp_policy_t){true, true, true, false})

/**
 * @brief Lenient policy (for proxying legacy clients).
 */
#define IHTP_POLICY_LENIENT ((ihtp_policy_t){false, false, false, true})

/**
 * @brief Default interoperable server-side preset for iohttp.
 *
 * This currently maps to the strict RFC profile. The named preset makes the
 * intended consumer contract explicit and allows future divergence without
 * changing integration call sites.
 */
#define IHTP_POLICY_IOHTTP IHTP_POLICY_STRICT

/**
 * @brief Default fail-closed preset for ioguard.
 *
 * This currently maps to the strict RFC profile and is named separately so
 * higher-layer integrations can depend on an explicit security-oriented preset.
 */
#define IHTP_POLICY_IOGUARD IHTP_POLICY_STRICT

#endif /* IOHTTPPARSER_IHTP_TYPES_H */
