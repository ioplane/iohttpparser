/*
 * iohttpparser — Internal header (not part of public API)
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#ifndef IOHTTPPARSER_IHTP_INTERNAL_H
#define IOHTTPPARSER_IHTP_INTERNAL_H

#include <iohttpparser/ihtp_semantics.h>
#include <iohttpparser/ihtp_types.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ─── Version ─────────────────────────────────────────────────────────── */

#define IHTP_VERSION_MAJOR  0
#define IHTP_VERSION_MINOR  1
#define IHTP_VERSION_PATCH  0
#define IHTP_VERSION_STRING "0.1.0"

/* ─── Scanner function pointers (runtime SIMD dispatch) ───────────────── */

/**
 * @brief Scanner backend: find first delimiter in buffer.
 */
typedef const char *(*ihtp_scan_find_fn)(const char *buf, size_t len, const char *delims);

/**
 * @brief Scanner backend: validate token characters.
 */
typedef bool (*ihtp_scan_token_fn)(const char *buf, size_t len);

/**
 * @brief Global scanner dispatch table (set once at init).
 */
typedef struct {
    ihtp_scan_find_fn find_char;
    ihtp_scan_token_fn is_token;
} ihtp_scanner_vtable_t;

/**
 * @brief Get the active scanner vtable (auto-initialized on first call).
 */
const ihtp_scanner_vtable_t *ihtp_scanner_get(void);

/**
 * @brief Select scanner backend functions for a specific SIMD capability level.
 *
 * This is internal glue for runtime dispatch and deterministic tests. The
 * selection remains fail-safe: scalar baseline first, then SSE4.2, then AVX2.
 */
void ihtp_scanner_select_vtable(ihtp_scanner_vtable_t *vtable, int simd_level);

/**
 * @brief Map a SIMD capability level to the selected internal backend name.
 */
const char *ihtp_scanner_backend_name_for_level(int simd_level);

/**
 * @brief Report the currently active scanner backend name.
 */
const char *ihtp_scanner_active_backend_name(void);

/* ─── Optional parser perf tracing (benchmark/debug only) ─────────────── */

typedef struct {
    uint64_t find_line_end_calls;
    uint64_t find_line_end_bytes;

    uint64_t find_method_space_calls;
    uint64_t find_method_space_bytes;

    uint64_t request_target_calls;
    uint64_t request_target_bytes;
    uint64_t request_target_fast_bytes;
    uint64_t request_target_slow_bytes;

    uint64_t find_header_name_colon_calls;
    uint64_t find_header_name_colon_bytes;

    uint64_t trim_field_value_calls;
    uint64_t trim_field_value_bytes;

    uint64_t field_text_calls;
    uint64_t field_text_bytes;
    uint64_t field_text_fast_bytes;
    uint64_t field_text_slow_bytes;

    uint64_t parse_request_line_calls;
    uint64_t parse_header_block_calls;
} ihtp_perf_counters_t;

void ihtp_perf_counters_reset(void);
void ihtp_perf_counters_snapshot(ihtp_perf_counters_t *out);

/* ─── Scanner backends ────────────────────────────────────────────────── */

/* Scalar (always available) */
const char *ihtp_scan_find_char_scalar(const char *buf, size_t len, const char *delims);
bool ihtp_scan_is_token_scalar(const char *buf, size_t len);

/* SSE4.2 (x86-64, runtime check) */
#ifdef IOHTTPPARSER_HAVE_SSE42
const char *ihtp_scan_find_char_sse42(const char *buf, size_t len, const char *delims);
bool ihtp_scan_is_token_sse42(const char *buf, size_t len);
#endif

/* AVX2 (x86-64, runtime check) */
#ifdef IOHTTPPARSER_HAVE_AVX2
const char *ihtp_scan_find_char_avx2(const char *buf, size_t len, const char *delims);
bool ihtp_scan_is_token_avx2(const char *buf, size_t len);
#endif

/* ─── Version ─────────────────────────────────────────────────────────── */

const char *ihtp_version(void);
int ihtp_version_num(void);

/* ─── Character classification (RFC 9110 Section 5.6.2) ──────────────── */

/**
 * @brief Token character lookup table.
 *
 * token = 1*tchar
 * tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
 *         "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
 */
extern const uint8_t ihtp_token_table[256];

/**
 * @brief Check if byte is a valid token character.
 */
static inline bool ihtp_is_token_char(uint8_t c)
{
    return ihtp_token_table[c] != 0;
}

/**
 * @brief Check if byte is a digit (0-9).
 */
static inline bool ihtp_is_digit(uint8_t c)
{
    return c >= '0' && c <= '9';
}

/**
 * @brief Check if byte is horizontal whitespace (SP or HTAB).
 */
static inline bool ihtp_is_lws(uint8_t c)
{
    return c == ' ' || c == '\t';
}

#endif /* IOHTTPPARSER_IHTP_INTERNAL_H */
