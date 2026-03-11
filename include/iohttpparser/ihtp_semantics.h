/*
 * iohttpparser — Semantics API (Layer 3: framing and connection decisions)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_SEMANTICS_H
#define IOHTTPPARSER_IHTP_SEMANTICS_H

#include <iohttpparser/ihtp_types.h>

/** @defgroup ihtp_semantics Semantics API
 *  @ingroup ihtp_api
 *  @brief RFC-driven framing, connection, and ownership decisions above syntax parsing.
 *  @{
 */

/**
 * @brief Apply RFC-driven semantics to a parsed HTTP request.
 * @ingroup ihtp_semantics
 *
 * This is the handoff between the syntax parser and the consumer.
 * The function resolves body framing, connection persistence, and request-side
 * rejection rules such as invalid Host handling or TE/CL ambiguity.
 * It also exposes consumer-facing ownership flags:
 * - protocol upgrades (`protocol_upgrade`)
 * - `Expect: 100-continue` (`expects_continue`)
 * - advertised trailer fields (`has_trailer_fields`)
 *
 * @param req    Parsed request from the parser layer.
 * @param policy Parsing policy. NULL = strict.
 * @return IHTP_OK on success, IHTP_ERROR on invalid or ambiguous framing.
 */
[[nodiscard]] ihtp_status_t ihtp_request_apply_semantics(ihtp_request_t *req,
                                                         const ihtp_policy_t *policy);

/**
 * @brief Apply RFC-driven semantics to a parsed HTTP response.
 * @ingroup ihtp_semantics
 *
 * This resolves response body framing and connection persistence so the
 * consumer can hand the result to a body decoder or connection-management
 * layer without re-parsing headers manually. It also marks protocol switches
 * and advertised trailer fields for higher layers.
 *
 * @param resp   Parsed response from the parser layer.
 * @param policy Parsing policy. NULL = strict.
 * @return IHTP_OK on success, IHTP_ERROR on invalid or ambiguous framing.
 */
[[nodiscard]] ihtp_status_t ihtp_response_apply_semantics(ihtp_response_t *resp,
                                                          const ihtp_policy_t *policy);

/** @} */

#endif /* IOHTTPPARSER_IHTP_SEMANTICS_H */
