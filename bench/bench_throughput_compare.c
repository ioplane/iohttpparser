/*
 * iohttpparser — Standalone throughput comparison benchmark
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/ihtp_parser.h>
#include <llhttp.h>
#include <picohttpparser.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    SCENARIO_REQUEST = 0,
    SCENARIO_RESPONSE = 1,
} scenario_kind_t;

typedef struct {
    const char *name;
    scenario_kind_t kind;
    const char *wire;
    size_t len;
    bool connect_only;
} scenario_t;

typedef struct {
    bool complete;
} llhttp_capture_t;

typedef enum {
    OUTPUT_HUMAN = 0,
    OUTPUT_TSV = 1,
} output_mode_t;

typedef bool (*parser_fn_t)(const char *buf, size_t len, scenario_kind_t kind, size_t *consumed_out);

static volatile uint64_t g_sink;

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static int llhttp_on_message_complete(llhttp_t *parser)
{
    llhttp_capture_t *capture = (llhttp_capture_t *)parser->data;
    capture->complete = true;
    return 0;
}

static bool parse_ihtp_with_policy(const char *buf, size_t len, scenario_kind_t kind,
                                   const ihtp_policy_t *policy, size_t *consumed_out)
{
    size_t consumed = 0;

    if (kind == SCENARIO_REQUEST) {
        ihtp_request_t req = {0};
        if (ihtp_parse_request(buf, len, &req, policy, &consumed) != IHTP_OK) {
            return false;
        }
    } else {
        ihtp_response_t resp = {0};
        if (ihtp_parse_response(buf, len, &resp, policy, &consumed) != IHTP_OK) {
            return false;
        }
    }

    if (consumed != len) {
        return false;
    }
    *consumed_out = consumed;
    return true;
}

static bool parse_ihtp_strict(const char *buf, size_t len, scenario_kind_t kind, size_t *consumed_out)
{
    static const ihtp_policy_t strict = IHTP_POLICY_STRICT;
    return parse_ihtp_with_policy(buf, len, kind, &strict, consumed_out);
}

static bool parse_ihtp_lenient(const char *buf, size_t len, scenario_kind_t kind, size_t *consumed_out)
{
    static const ihtp_policy_t lenient = IHTP_POLICY_LENIENT;
    return parse_ihtp_with_policy(buf, len, kind, &lenient, consumed_out);
}

static bool parse_pico(const char *buf, size_t len, scenario_kind_t kind, size_t *consumed_out)
{
    struct phr_header headers[IHTP_MAX_HEADERS];
    size_t num_headers = IHTP_MAX_HEADERS;
    int minor = 0;

    if (kind == SCENARIO_REQUEST) {
        const char *method = nullptr;
        size_t method_len = 0;
        const char *path = nullptr;
        size_t path_len = 0;
        int ret = phr_parse_request(buf, len, &method, &method_len, &path, &path_len, &minor,
                                    headers, &num_headers, 0);
        if (ret <= 0) {
            return false;
        }
        *consumed_out = (size_t)ret;
        return (size_t)ret == len;
    }

    int status = 0;
    const char *reason = nullptr;
    size_t reason_len = 0;
    int ret = phr_parse_response(buf, len, &minor, &status, &reason, &reason_len, headers,
                                 &num_headers, 0);
    if (ret <= 0) {
        return false;
    }
    *consumed_out = (size_t)ret;
    return (size_t)ret == len;
}

static bool parse_llhttp(const char *buf, size_t len, scenario_kind_t kind, size_t *consumed_out)
{
    llhttp_settings_t settings;
    llhttp_t parser;
    llhttp_capture_t capture = {0};

    llhttp_settings_init(&settings);
    settings.on_message_complete = llhttp_on_message_complete;

    llhttp_type_t type = kind == SCENARIO_REQUEST ? HTTP_REQUEST : HTTP_RESPONSE;
    llhttp_init(&parser, type, &settings);
    parser.data = &capture;

    llhttp_errno_t execute_err = llhttp_execute(&parser, buf, len);
    if (execute_err != HPE_OK && execute_err != HPE_PAUSED_UPGRADE) {
        return false;
    }
    llhttp_errno_t finish_err = llhttp_finish(&parser);
    if (finish_err != HPE_OK && finish_err != HPE_PAUSED_UPGRADE) {
        return false;
    }
    if (!capture.complete && execute_err != HPE_PAUSED_UPGRADE && finish_err != HPE_PAUSED_UPGRADE) {
        return false;
    }

    *consumed_out = len;
    return true;
}

static void bench_parser(output_mode_t mode, const char *parser_name, parser_fn_t parse_fn,
                         const scenario_t *scenarios, size_t scenario_count, size_t iterations,
                         bool connect_only)
{
    for (size_t i = 0; i < scenario_count; i++) {
        const scenario_t *sc = &scenarios[i];
        size_t consumed = 0;

        if (connect_only != sc->connect_only) {
            continue;
        }

        for (size_t warmup = 0; warmup < 1000; warmup++) {
            if (!parse_fn(sc->wire, sc->len, sc->kind, &consumed)) {
                fprintf(stderr, "warmup failed: parser=%s scenario=%s\n", parser_name, sc->name);
                exit(2);
            }
            g_sink += (uint64_t)consumed;
        }

        uint64_t start_ns = monotonic_ns();
        for (size_t iter = 0; iter < iterations; iter++) {
            if (!parse_fn(sc->wire, sc->len, sc->kind, &consumed)) {
                fprintf(stderr, "parse failed: parser=%s scenario=%s iter=%zu\n", parser_name,
                        sc->name, iter);
                exit(2);
            }
            g_sink += (uint64_t)consumed;
        }
        uint64_t elapsed_ns = monotonic_ns() - start_ns;
        double elapsed_s = (double)elapsed_ns / 1e9;
        double req_per_s = elapsed_s > 0.0 ? (double)iterations / elapsed_s : 0.0;
        double mib_per_s = elapsed_s > 0.0 ? (((double)iterations * (double)sc->len) /
                                              (1024.0 * 1024.0)) /
                                                 elapsed_s
                                           : 0.0;
        double ns_per_req = iterations > 0 ? (double)elapsed_ns / (double)iterations : 0.0;

        if (mode == OUTPUT_TSV) {
            printf("%s\t%s\t%s\t%zu\t%" PRIu64 "\t%.2f\t%.2f\t%.2f\n", parser_name, sc->name,
                   sc->kind == SCENARIO_REQUEST ? "request" : "response", sc->len, elapsed_ns,
                   req_per_s, mib_per_s, ns_per_req);
        } else {
            printf("%-20s %-12s %-8s %-4zu %-12" PRIu64 " %-12.2f %-12.2f %-10.2f\n", parser_name,
                   sc->name, sc->kind == SCENARIO_REQUEST ? "request" : "response", sc->len,
                   elapsed_ns, req_per_s, mib_per_s, ns_per_req);
        }
    }
}

int main(int argc, char **argv)
{
    static const char req_line_hot[] =
        "GET /v1/ping?x=1&y=2 HTTP/1.1\r\nHost: a\r\n\r\n";
    static const char req_small[] = "GET /api/v1/ping HTTP/1.1\r\nHost: example.test\r\n\r\n";
    static const char req_headers[] =
        "GET /resource/alpha?x=1&y=2 HTTP/1.1\r\nHost: example.test\r\nConnection: keep-alive\r\n"
        "Accept: application/json\r\nUser-Agent: bench-client/1.0\r\nX-Forwarded-For: "
        "203.0.113.10, 198.51.100.42\r\n\r\n";
    static const char hdr_common_heavy[] =
        "GET /r HTTP/1.1\r\nHost: example.test\r\nConnection: keep-alive\r\nContent-Length: 0\r\n"
        "Expect: 100-continue\r\nUpgrade: websocket\r\n\r\n";
    static const char hdr_uncommon_valid[] =
        "GET /r HTTP/1.1\r\nX-Custom-Alpha-Token: abcdef1234567890\r\n"
        "X-Trace-Vector-Path: a,b,c,d,e,f,g\r\n"
        "X-Long-Meta-Field: token1 token2 token3 token4 token5\r\n"
        "X-Forwarded-Proto-Chain: https,http,https\r\n\r\n";
    static const char req_connect[] =
        "CONNECT vpn.example.test:443 HTTP/1.1\r\nHost: vpn.example.test:443\r\n"
        "Proxy-Connection: keep-alive\r\n\r\n";

    static const char resp_small[] =
        "HTTP/1.1 204 No Content\r\nConnection: keep-alive\r\n\r\n";
    static const char resp_headers[] =
        "HTTP/1.1 304 Not Modified\r\nConnection: keep-alive\r\nCache-Control: max-age=60\r\n"
        "ETag: \"abc123\"\r\nServer: iohttp/1.0\r\n\r\n";
    static const char resp_upgrade[] =
        "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n\r\n";

    static const scenario_t scenarios[] = {
        {"req-line-hot", SCENARIO_REQUEST, req_line_hot, sizeof(req_line_hot) - 1, false},
        {"req-small", SCENARIO_REQUEST, req_small, sizeof(req_small) - 1, false},
        {"req-headers", SCENARIO_REQUEST, req_headers, sizeof(req_headers) - 1, false},
        {"hdr-common-heavy", SCENARIO_REQUEST, hdr_common_heavy, sizeof(hdr_common_heavy) - 1, false},
        {"hdr-uncommon-valid", SCENARIO_REQUEST, hdr_uncommon_valid, sizeof(hdr_uncommon_valid) - 1,
         false},
        {"req-connect", SCENARIO_REQUEST, req_connect, sizeof(req_connect) - 1, true},
        {"resp-small", SCENARIO_RESPONSE, resp_small, sizeof(resp_small) - 1, false},
        {"resp-headers", SCENARIO_RESPONSE, resp_headers, sizeof(resp_headers) - 1, false},
        {"resp-upgrade", SCENARIO_RESPONSE, resp_upgrade, sizeof(resp_upgrade) - 1, false},
    };

    size_t iterations = 200000;
    output_mode_t mode = OUTPUT_HUMAN;
    bool connect_only = false;

    if (argc > 4) {
        fprintf(stderr, "usage: %s [iterations] [--tsv] [--connect-only]\n", argv[0]);
        return 2;
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tsv") == 0) {
            mode = OUTPUT_TSV;
            continue;
        }
        if (strcmp(argv[i], "--connect-only") == 0) {
            connect_only = true;
            continue;
        }
        char *end = nullptr;
        unsigned long long parsed = strtoull(argv[i], &end, 10);
        if (end == argv[i] || *end != '\0' || parsed == 0ULL) {
            fprintf(stderr, "invalid iterations: %s\n", argv[i]);
            return 2;
        }
        iterations = (size_t)parsed;
    }

    if (mode == OUTPUT_TSV) {
        puts("format\ttsv\tv1");
        printf("meta\titerations\t%zu\n", iterations);
        printf("meta\tconnect_only\t%s\n", connect_only ? "true" : "false");
        puts("columns\tparser\tscenario\tkind\tlen\telapsed_ns\treq_per_s\tmib_per_s\tns_per_req");
    } else {
        printf("Standalone throughput comparison benchmark\n");
        printf("iterations: %zu\n", iterations);
        printf("connect_only: %s\n\n", connect_only ? "true" : "false");
        printf("%-20s %-12s %-8s %-4s %-12s %-12s %-12s %-10s\n", "parser", "scenario", "kind",
               "len", "elapsed_ns", "req/s", "MiB/s", "ns/req");
    }

    bench_parser(mode, "iohttpparser-strict", parse_ihtp_strict, scenarios,
                 sizeof(scenarios) / sizeof(scenarios[0]), iterations, connect_only);
    bench_parser(mode, "iohttpparser-lenient", parse_ihtp_lenient, scenarios,
                 sizeof(scenarios) / sizeof(scenarios[0]), iterations, connect_only);
    bench_parser(mode, "picohttpparser", parse_pico, scenarios,
                 sizeof(scenarios) / sizeof(scenarios[0]), iterations, connect_only);
    bench_parser(mode, "llhttp", parse_llhttp, scenarios, sizeof(scenarios) / sizeof(scenarios[0]),
                 iterations, connect_only);

    if (mode == OUTPUT_TSV) {
        printf("meta\tsink\t%" PRIu64 "\n", g_sink);
    } else {
        printf("\nsink: %" PRIu64 "\n", g_sink);
    }
    return 0;
}
