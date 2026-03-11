/*
 * iohttpparser — Parser API (Layer 2: request-line + headers)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_PARSER_H
#define IOHTTPPARSER_IHTP_PARSER_H

#include <iohttpparser/ihtp_types.h>

/** @defgroup ihtp_parser Parser API
 *  @ingroup ihtp_api
 *  @brief Request, response, and headers-only parsing entry points.
 *  @{
 */

/**
 * @brief Initialize a parser state object for incremental parsing.
 * @ingroup ihtp_parser
 *
 * @param state Parser state object.
 * @param mode  Request, response, or headers-only parsing mode.
 */
void ihtp_parser_state_init(ihtp_parser_state_t *state, ihtp_parser_mode_t mode);

/**
 * @brief Reset an initialized parser state object to its initial phase.
 * @ingroup ihtp_parser
 *
 * The parser mode is preserved.
 * Previously produced output structs remain caller-owned and are not cleared.
 *
 * @param state Parser state object.
 */
void ihtp_parser_state_reset(ihtp_parser_state_t *state);

/**
 * @brief Parse an HTTP request (request-line + headers).
 * @ingroup ihtp_parser
 *
 * Pull-based incremental API: call repeatedly with accumulated data.
 * On IHTP_OK, bytes_consumed is set to the number of bytes parsed.
 * On IHTP_INCOMPLETE and IHTP_ERROR, bytes_consumed is reset to zero.
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
 * @brief Parse an HTTP request using explicit parser state.
 * @ingroup ihtp_parser
 *
 * Reuse the same state object across calls while the accumulated input buffer grows.
 * The output remains zero-copy and points into the caller-owned buffer.
 * Do not switch the state object between unrelated buffer contents without reset.
 *
 * @param state          Parser state initialized with IHTP_PARSER_MODE_REQUEST.
 * @param buf            Input buffer (accumulated bytes).
 * @param len            Length of input buffer.
 * @param req            Output: parsed request.
 * @param policy         Parsing policy. NULL = strict.
 * @param bytes_consumed Output: total bytes consumed on IHTP_OK.
 * @return IHTP_OK on success, IHTP_INCOMPLETE if more data is needed, IHTP_ERROR on failure.
 */
[[nodiscard]] ihtp_status_t ihtp_parse_request_stateful(ihtp_parser_state_t *state, const char *buf,
                                                        size_t len, ihtp_request_t *req,
                                                        const ihtp_policy_t *policy,
                                                        size_t *bytes_consumed);

/**
 * @brief Parse an HTTP response (status-line + headers).
 * @ingroup ihtp_parser
 *
 * On IHTP_INCOMPLETE and IHTP_ERROR, bytes_consumed is reset to zero.
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
 * @brief Parse an HTTP response using explicit parser state.
 * @ingroup ihtp_parser
 *
 * Reuse the same state object while the same accumulated response buffer grows.
 * Returned spans still point into the caller-owned buffer.
 *
 * @param state          Parser state initialized with IHTP_PARSER_MODE_RESPONSE.
 * @param buf            Input buffer.
 * @param len            Length of input buffer.
 * @param resp           Output: parsed response.
 * @param policy         Parsing policy. NULL = strict.
 * @param bytes_consumed Output: total bytes consumed on IHTP_OK.
 * @return IHTP_OK on success, IHTP_INCOMPLETE if more data is needed, IHTP_ERROR on failure.
 */
[[nodiscard]] ihtp_status_t ihtp_parse_response_stateful(ihtp_parser_state_t *state,
                                                         const char *buf, size_t len,
                                                         ihtp_response_t *resp,
                                                         const ihtp_policy_t *policy,
                                                         size_t *bytes_consumed);

/**
 * @brief Parse headers only (for trailer sections or standalone header parsing).
 * @ingroup ihtp_parser
 *
 * On IHTP_INCOMPLETE and IHTP_ERROR, bytes_consumed is reset to zero.
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
 * @brief Parse a standalone header block using explicit parser state.
 * @ingroup ihtp_parser
 *
 * This follows the same accumulated-buffer contract as the request and
 * response stateful entry points.
 *
 * @param state          Parser state initialized with IHTP_PARSER_MODE_HEADERS.
 * @param buf            Input buffer.
 * @param len            Length of input buffer.
 * @param headers        Output header array.
 * @param num_headers    Input/output header count.
 * @param max_headers    Maximum number of headers that fit in the array.
 * @param policy         Parsing policy. NULL = strict.
 * @param bytes_consumed Output: total bytes consumed on IHTP_OK.
 * @return IHTP_OK on success, IHTP_INCOMPLETE if more data is needed, IHTP_ERROR on failure.
 */
[[nodiscard]] ihtp_status_t ihtp_parse_headers_stateful(ihtp_parser_state_t *state, const char *buf,
                                                        size_t len, ihtp_header_t *headers,
                                                        size_t *num_headers, size_t max_headers,
                                                        const ihtp_policy_t *policy,
                                                        size_t *bytes_consumed);

/**
 * @brief Resolve method string to enum.
 * @ingroup ihtp_parser
 *
 * @param method     Method string (not NUL-terminated).
 * @param method_len Length of method string.
 * @return ihtp_method_t enum value, or IHTP_METHOD_UNKNOWN.
 */
[[nodiscard]] ihtp_method_t ihtp_method_from_str(const char *method, size_t method_len);

/**
 * @brief Return method name as NUL-terminated string.
 * @ingroup ihtp_parser
 *
 * @param method Method enum.
 * @return Static string, or "UNKNOWN".
 */
[[nodiscard]] const char *ihtp_method_to_str(ihtp_method_t method);

/** @} */

#endif /* IOHTTPPARSER_IHTP_PARSER_H */
