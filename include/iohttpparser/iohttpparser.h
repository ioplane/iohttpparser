/*
 * iohttpparser — High-performance HTTP/1.1 parser for C23
 * SIMD-accelerated · RFC 9112 · Pull-based incremental API
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IOHTTPPARSER_H
#define IOHTTPPARSER_IOHTTPPARSER_H

/**
 * @file iohttpparser.h
 * @brief Main public API header — includes all parser components.
 *
 * Usage:
 * @code{.c}
 * #include <iohttpparser/iohttpparser.h>
 * @endcode
 *
 * Or include individual headers:
 * @code{.c}
 * #include <iohttpparser/ihtp_types.h>
 * #include <iohttpparser/ihtp_parser.h>
 * #include <iohttpparser/ihtp_semantics.h>
 * #include <iohttpparser/ihtp_body.h>
 * @endcode
 */

#include <iohttpparser/ihtp_body.h>
#include <iohttpparser/ihtp_parser.h>
#include <iohttpparser/ihtp_semantics.h>
#include <iohttpparser/ihtp_types.h>

/**
 * @brief Return library version string.
 * @return NUL-terminated version string (e.g., "0.1.0").
 */
[[nodiscard]] const char *ihtp_version(void);

/**
 * @brief Return library version as packed integer.
 * @return Version as (major << 16 | minor << 8 | patch).
 */
[[nodiscard]] int ihtp_version_num(void);

#endif /* IOHTTPPARSER_IOHTTPPARSER_H */
