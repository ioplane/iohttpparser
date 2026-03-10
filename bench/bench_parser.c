/*
 * iohttpparser — Scanner benchmark harness
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#include <iohttpparser/ihtp_scanner.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    const char *name;
    const char *buf;
    size_t len;
    const char *delims;
} find_case_t;

typedef struct {
    const char *name;
    const char *buf;
    size_t len;
} token_case_t;

static volatile size_t find_sink;
static volatile unsigned token_sink;

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }

    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static void run_find_bench(const char *backend_name, ihtp_scan_find_fn fn,
                           const find_case_t *test_case, size_t iterations)
{
    volatile uintptr_t fn_bits = (uintptr_t)(const void *)fn;
    ihtp_scan_find_fn bench_fn = (ihtp_scan_find_fn)(const void *)fn_bits;
    uint64_t start_ns = monotonic_ns();

    for (size_t i = 0; i < iterations; i++) {
        const char *result = bench_fn(test_case->buf, test_case->len, test_case->delims);
        find_sink += (size_t)(result - test_case->buf);
    }

    uint64_t elapsed_ns = monotonic_ns() - start_ns;
    double ns_per_iter = iterations == 0 ? 0.0 : (double)elapsed_ns / (double)iterations;

    printf("%-8s find  %-18s len=%-3zu %12" PRIu64 " ns total  %10.2f ns/op\n", backend_name,
           test_case->name, test_case->len, elapsed_ns, ns_per_iter);
}

static void run_token_bench(const char *backend_name, ihtp_scan_token_fn fn,
                            const token_case_t *test_case, size_t iterations)
{
    volatile uintptr_t fn_bits = (uintptr_t)(const void *)fn;
    ihtp_scan_token_fn bench_fn = (ihtp_scan_token_fn)(const void *)fn_bits;
    uint64_t start_ns = monotonic_ns();

    for (size_t i = 0; i < iterations; i++) {
        token_sink += bench_fn(test_case->buf, test_case->len) ? 1U : 0U;
    }

    uint64_t elapsed_ns = monotonic_ns() - start_ns;
    double ns_per_iter = iterations == 0 ? 0.0 : (double)elapsed_ns / (double)iterations;

    printf("%-8s token %-18s len=%-3zu %12" PRIu64 " ns total  %10.2f ns/op\n", backend_name,
           test_case->name, test_case->len, elapsed_ns, ns_per_iter);
}

static void run_backend_group(const char *backend_name, ihtp_scan_find_fn find_fn,
                              ihtp_scan_token_fn token_fn, const find_case_t *find_cases,
                              size_t find_case_count, const token_case_t *token_cases,
                              size_t token_case_count, size_t iterations)
{
    for (size_t i = 0; i < find_case_count; i++) {
        run_find_bench(backend_name, find_fn, &find_cases[i], iterations);
    }

    for (size_t i = 0; i < token_case_count; i++) {
        run_token_bench(backend_name, token_fn, &token_cases[i], iterations);
    }
}

int main(int argc, char **argv)
{
    static const find_case_t find_cases[] = {
        {.name = "request-space", .buf = "GET /alpha/beta HTTP/1.1\r\n", .len = 26, .delims = " "},
        {.name = "header-crlf", .buf = "Content-Type: text/plain\r\n", .len = 26, .delims = "\r\n"},
        {.name = "query-delim", .buf = "/search?q=alpha&lang=en", .len = 23, .delims = "=&"},
        {.name = "not-found", .buf = "token-without-colon", .len = 19, .delims = ":"},
        {.name = "status-space", .buf = "HTTP/1.1 204 No Content\r\n", .len = 25, .delims = " "},
        {.name = "cookie-semi",
         .buf = "session=alpha; Path=/; HttpOnly; SameSite=Lax",
         .len = 46,
         .delims = ";"},
        {.name = "long-header-crlf",
         .buf = "X-Forwarded-For: 203.0.113.1, 198.51.100.2, 192.0.2.10\r\n",
         .len = 61,
         .delims = "\r\n"},
        {.name = "wide-not-found",
         .buf = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
         .len = 62,
         .delims = ":"},
    };
    static const token_case_t token_cases[] = {
        {.name = "method", .buf = "GET", .len = 3},
        {.name = "header", .buf = "Content-Type", .len = 12},
        {.name = "invalid-space", .buf = "bad header", .len = 10},
        {.name = "invalid-colon", .buf = "bad:name", .len = 8},
        {.name = "conn-token", .buf = "keep-alive", .len = 10},
        {.name = "long-header", .buf = "X-Forwarded-For", .len = 15},
        {.name = "wide-valid", .buf = "abcdefghijklmnopqrstuvwxyz012345", .len = 32},
        {.name = "invalid-del", .buf = "bad\x7fname", .len = 8},
    };
    size_t iterations = 1000000;
    int simd_level = ihtp_scanner_simd_level();

    if (argc > 2) {
        fprintf(stderr, "usage: %s [iterations]\n", argv[0]);
        return 2;
    }

    if (argc == 2) {
        char *end = nullptr;
        unsigned long long parsed = strtoull(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || parsed == 0ULL) {
            fprintf(stderr, "invalid iterations: %s\n", argv[1]);
            return 2;
        }
        iterations = (size_t)parsed;
    }

    printf("iohttpparser scanner benchmark\n");
    printf("iterations: %zu\n", iterations);
    printf("simd-level: 0x%x\n\n", simd_level);

    run_backend_group("dispatch", ihtp_scan_find_char, ihtp_scan_is_token, find_cases,
                      sizeof(find_cases) / sizeof(find_cases[0]), token_cases,
                      sizeof(token_cases) / sizeof(token_cases[0]), iterations);
    run_backend_group("scalar", ihtp_scan_find_char_scalar, ihtp_scan_is_token_scalar, find_cases,
                      sizeof(find_cases) / sizeof(find_cases[0]), token_cases,
                      sizeof(token_cases) / sizeof(token_cases[0]), iterations);

#ifdef IOHTTPPARSER_HAVE_SSE42
    if ((simd_level & 0x01) != 0) {
        run_backend_group("sse42", ihtp_scan_find_char_sse42, ihtp_scan_is_token_sse42, find_cases,
                          sizeof(find_cases) / sizeof(find_cases[0]), token_cases,
                          sizeof(token_cases) / sizeof(token_cases[0]), iterations);
    }
#endif

#ifdef IOHTTPPARSER_HAVE_AVX2
    if ((simd_level & 0x02) != 0) {
        run_backend_group("avx2", ihtp_scan_find_char_avx2, ihtp_scan_is_token_avx2, find_cases,
                          sizeof(find_cases) / sizeof(find_cases[0]), token_cases,
                          sizeof(token_cases) / sizeof(token_cases[0]), iterations);
    }
#endif

    printf("\nsinks: %zu %u\n", find_sink, token_sink);
    return 0;
}
