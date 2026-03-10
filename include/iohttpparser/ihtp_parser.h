/*
 * iohttpparser — Parser API (Layer 2: request-line + headers)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_PARSER_H
#define IOHTTPPARSER_IHTP_PARSER_H

#include <iohttpparser/ihtp_types.h>

/**
 * @brief Parse an HTTP request (request-line + headers).
 *
 * Pull-based incremental API: call repeatedly with accumulated data.
 * On IHTP_OK, bytes_consumed is set to the number of bytes parsed.
 *
 * @param buf          Input buffer (accumulated bytes).
 * @param len          Length of input buffer.
 * @param req          Output: parsed request (zero-copy pointers into buf).
 * @param policy       Parsing policy (strict/lenient). NULL = strict.
 * @param bytes_consumed Output: number of bytes consumed on IHTP_OK.
 * @return IHTP_OK on success, IHTP_INCOMPLETE if more data needed, IHTP_ERROR on failure.
 */
[[nodiscard]] ihtp_status_t ihtp_parse_request(const char *buf, size_t len, ihtp_request_t *req,
                                               const ihtp_policy_t *policy, size_t *bytes_consumed);

/**
 * @brief Parse an HTTP response (status-line + headers).
 *
 * @param buf          Input buffer.
 * @param len          Length of input buffer.
 * @param resp         Output: parsed response.
 * @param policy       Parsing policy. NULL = strict.
 * @param bytes_consumed Output: number of bytes consumed on IHTP_OK.
 * @return IHTP_OK on success, IHTP_INCOMPLETE if more data needed, IHTP_ERROR on failure.
 */
[[nodiscard]] ihtp_status_t ihtp_parse_response(const char *buf, size_t len, ihtp_response_t *resp,
                                                const ihtp_policy_t *policy,
                                                size_t *bytes_consumed);

/**
 * @brief Parse headers only (for trailer sections or standalone header parsing).
 *
 * @param buf          Input buffer.
 * @param len          Length of input buffer.
 * @param headers      Output: array of headers.
 * @param num_headers  Input: max headers; Output: actual count.
 * @param policy       Parsing policy. NULL = strict.
 * @param bytes_consumed Output: number of bytes consumed on IHTP_OK.
 * @return IHTP_OK on success, IHTP_INCOMPLETE if more data needed, IHTP_ERROR on failure.
 */
[[nodiscard]] ihtp_status_t ihtp_parse_headers(const char *buf, size_t len, ihtp_header_t *headers,
                                               size_t *num_headers, const ihtp_policy_t *policy,
                                               size_t *bytes_consumed);

/**
 * @brief Resolve method string to enum.
 *
 * @param method     Method string (not NUL-terminated).
 * @param method_len Length of method string.
 * @return ihtp_method_t enum value, or IHTP_METHOD_UNKNOWN.
 */
[[nodiscard]] ihtp_method_t ihtp_method_from_str(const char *method, size_t method_len);

/**
 * @brief Return method name as NUL-terminated string.
 *
 * @param method Method enum.
 * @return Static string, or "UNKNOWN".
 */
[[nodiscard]] const char *ihtp_method_to_str(ihtp_method_t method);

#endif /* IOHTTPPARSER_IHTP_PARSER_H */
