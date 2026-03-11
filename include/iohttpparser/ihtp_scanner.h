/*
 * iohttpparser — Scanner API (Layer 1: byte classification, SIMD)
 *
 * Advanced API for direct scanner access. Most users should use
 * ihtp_parser.h instead, which calls the scanner internally.
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_SCANNER_H
#define IOHTTPPARSER_IHTP_SCANNER_H

#include <stddef.h>

/** @defgroup ihtp_scanner Scanner API
 *  @ingroup ihtp_api
 *  @brief Low-level byte classification and delimiter search helpers.
 *  @{
 */

/**
 * @brief Find first occurrence of a character from a character class.
 * @ingroup ihtp_scanner
 *
 * SIMD-accelerated: uses SSE4.2 / AVX2 when available, with runtime dispatch.
 *
 * @param buf     Input buffer.
 * @param len     Length of input.
 * @param delims  NUL-terminated string of delimiter characters to find.
 * @return Pointer to first delimiter found, or buf+len if none found.
 */
[[nodiscard]] const char *ihtp_scan_find_char(const char *buf, size_t len, const char *delims);

/**
 * @brief Validate that a range contains only valid token characters (RFC 9110 token).
 * @ingroup ihtp_scanner
 *
 * @param buf Input buffer.
 * @param len Length of input.
 * @return true if all bytes are valid token characters.
 */
[[nodiscard]] bool ihtp_scan_is_token(const char *buf, size_t len);

/**
 * @brief Advance past linear whitespace (SP / HTAB).
 * @ingroup ihtp_scanner
 *
 * @param buf Input buffer.
 * @param len Length of input.
 * @return Number of whitespace bytes skipped.
 */
[[nodiscard]] size_t ihtp_scan_skip_lws(const char *buf, size_t len);

/**
 * @brief Detect SIMD support level at runtime.
 * @ingroup ihtp_scanner
 *
 * @return Bitmask: 0x01 = SSE4.2, 0x02 = AVX2, 0x04 = AVX-512.
 */
[[nodiscard]] int ihtp_scanner_simd_level(void);

/** @} */

#endif /* IOHTTPPARSER_IHTP_SCANNER_H */
