/*
 * iohttpparser — Semantics API (Layer 3: framing and connection decisions)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_SEMANTICS_H
#define IOHTTPPARSER_IHTP_SEMANTICS_H

#include <iohttpparser/ihtp_types.h>

/**
 * @brief Apply RFC-driven semantics to a parsed HTTP request.
 *
 * This is the handoff between the syntax parser and the consumer.
 * The function resolves body framing, connection persistence, and request-side
 * rejection rules such as invalid Host handling or TE/CL ambiguity.
 *
 * @param req    Parsed request from the parser layer.
 * @param policy Parsing policy. NULL = strict.
 * @return IHTP_OK on success, IHTP_ERROR on invalid or ambiguous framing.
 */
[[nodiscard]] ihtp_status_t ihtp_request_apply_semantics(ihtp_request_t *req,
                                                         const ihtp_policy_t *policy);

/**
 * @brief Apply RFC-driven semantics to a parsed HTTP response.
 *
 * This resolves response body framing and connection persistence so the
 * consumer can hand the result to a body decoder or connection-management
 * layer without re-parsing headers manually.
 *
 * @param resp   Parsed response from the parser layer.
 * @param policy Parsing policy. NULL = strict.
 * @return IHTP_OK on success, IHTP_ERROR on invalid or ambiguous framing.
 */
[[nodiscard]] ihtp_status_t ihtp_response_apply_semantics(ihtp_response_t *resp,
                                                          const ihtp_policy_t *policy);

#endif /* IOHTTPPARSER_IHTP_SEMANTICS_H */
