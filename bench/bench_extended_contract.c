/*
 * iohttpparser — Extended contract throughput benchmark
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include <iohttpparser/ihtp_body.h>
#include <iohttpparser/ihtp_parser.h>
#include <iohttpparser/ihtp_semantics.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    OUTPUT_HUMAN = 0,
    OUTPUT_TSV = 1,
} output_mode_t;

typedef struct {
    const char *name;
    const char *group;
    const char *baseline;
    const char *wire;
    size_t len;
    bool (*fn)(const char *wire, size_t len, size_t *work_bytes_out);
} extended_scenario_t;

static volatile uint64_t g_sink;

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static bool scenario_stateful_reuse_request(const char *wire, size_t len, size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_STRICT;
    ihtp_parser_state_t state;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (consumed != len) {
        return false;
    }

    ihtp_parser_state_reset(&state);
    memset(&req, 0, sizeof(req));
    consumed = 0;
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (consumed != len) {
        return false;
    }

    *work_bytes_out = len * 2U;
    return true;
}

static bool scenario_request_chunked_parse_only(const char *wire, size_t len, size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_STRICT;
    ihtp_parser_state_t state;
    ihtp_request_t req;
    size_t consumed = 0;
    const char *headers_end = strstr(wire, "\r\n\r\n");

    if (headers_end == nullptr) {
        return false;
    }

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (consumed != (size_t)(headers_end + 4 - wire)) {
        return false;
    }

    *work_bytes_out = consumed;
    return true;
}

static bool scenario_request_chunked_parse_semantics(const char *wire, size_t len,
                                                     size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_parser_state_t state;
    ihtp_request_t req;
    size_t consumed = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }
    if (req.body_mode != IHTP_BODY_CHUNKED) {
        return false;
    }

    *work_bytes_out = consumed;
    return true;
}

static bool scenario_request_chunked_parse_semantics_body(const char *wire, size_t len,
                                                          size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_parser_state_t state;
    ihtp_request_t req;
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    size_t consumed = 0;
    size_t body_len = 0;
    char tail[256];
    int trailing = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }
    if (req.body_mode != IHTP_BODY_CHUNKED) {
        return false;
    }

    body_len = len - consumed;
    if (body_len >= sizeof(tail)) {
        return false;
    }
    memcpy(tail, wire + consumed, body_len);
    trailing = ihtp_decode_chunked(&dec, tail, &body_len);
    if (trailing < 0) {
        return false;
    }

    *work_bytes_out = consumed + body_len + (size_t)trailing;
    return true;
}

static bool scenario_consumer_iohttp_pipeline(const char *wire, size_t len, size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    ihtp_chunked_decoder_t dec = {.consume_trailer = true};
    char tail[256];
    size_t consumed = 0;
    size_t body_len = 0;
    int trailing = 0;
    const char *next = nullptr;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }

    body_len = len - consumed;
    if (body_len >= sizeof(tail)) {
        return false;
    }
    memcpy(tail, wire + consumed, body_len);
    trailing = ihtp_decode_chunked(&dec, tail, &body_len);
    if (trailing < 0) {
        return false;
    }

    next = tail + body_len;
    ihtp_parser_state_reset(&state);
    memset(&req, 0, sizeof(req));
    consumed = 0;
    if (ihtp_parse_request_stateful(&state, next, (size_t)trailing, &req, &policy, &consumed) !=
        IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }

    *work_bytes_out = len + (size_t)trailing;
    return true;
}

static bool scenario_consumer_iohttp_expect_trailers(const char *wire, size_t len,
                                                     size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    ihtp_chunked_decoder_t dec = {.consume_trailer = false};
    char tail[320];
    size_t consumed = 0;
    size_t body_len = 0;
    int trailing = 0;
    const char *trailer_and_next = nullptr;
    const char *trailer_end = nullptr;
    const char *next = nullptr;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }

    body_len = len - consumed;
    if (body_len >= sizeof(tail)) {
        return false;
    }
    memcpy(tail, wire + consumed, body_len);
    trailing = ihtp_decode_chunked(&dec, tail, &body_len);
    if (trailing < 0) {
        return false;
    }

    trailer_and_next = tail + body_len;
    trailer_end = strstr(trailer_and_next, "\r\n\r\n");
    if (trailer_end == nullptr) {
        return false;
    }
    next = trailer_end + 4;

    ihtp_parser_state_reset(&state);
    memset(&req, 0, sizeof(req));
    consumed = 0;
    if (ihtp_parse_request_stateful(&state, next, strlen(next), &req, &policy, &consumed) !=
        IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }

    *work_bytes_out = len + strlen(next);
    return true;
}

static bool scenario_response_upgrade_parse_only(const char *wire, size_t len,
                                                 size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_parser_state_t state;
    ihtp_response_t resp;
    size_t consumed = 0;
    const char *headers_end = strstr(wire, "\r\n\r\n");

    if (headers_end == nullptr) {
        return false;
    }

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);
    memset(&resp, 0, sizeof(resp));
    if (ihtp_parse_response_stateful(&state, wire, len, &resp, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (consumed != (size_t)(headers_end + 4 - wire)) {
        return false;
    }

    *work_bytes_out = consumed;
    return true;
}

static bool scenario_response_upgrade_parse_semantics(const char *wire, size_t len,
                                                      size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_parser_state_t state;
    ihtp_response_t resp;
    size_t consumed = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);
    memset(&resp, 0, sizeof(resp));
    if (ihtp_parse_response_stateful(&state, wire, len, &resp, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_response_apply_semantics(&resp, &policy) != IHTP_OK) {
        return false;
    }
    if (!resp.protocol_upgrade) {
        return false;
    }

    *work_bytes_out = consumed;
    return true;
}

static bool scenario_consumer_ioguard_connect(const char *wire, size_t len, size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOGUARD;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    size_t consumed = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_OK) {
        return false;
    }
    if (req.method != IHTP_METHOD_CONNECT) {
        return false;
    }

    *work_bytes_out = consumed;
    return true;
}

static bool scenario_response_fixed_parse_semantics_body(const char *wire, size_t len,
                                                         size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_parser_state_t state;
    ihtp_response_t resp;
    ihtp_fixed_decoder_t dec;
    size_t consumed = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);
    memset(&resp, 0, sizeof(resp));
    if (ihtp_parse_response_stateful(&state, wire, len, &resp, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_response_apply_semantics(&resp, &policy) != IHTP_OK) {
        return false;
    }
    if (resp.body_mode != IHTP_BODY_FIXED || resp.content_length == 0) {
        return false;
    }
    ihtp_fixed_decoder_init(&dec, resp.content_length);
    if (ihtp_decode_fixed(&dec, (size_t)resp.content_length) != IHTP_OK) {
        return false;
    }

    *work_bytes_out = consumed + (size_t)resp.content_length;
    return true;
}

static bool scenario_consumer_iohttp_fixed_response(const char *wire, size_t len,
                                                    size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOHTTP;
    ihtp_response_t resp;
    ihtp_parser_state_t state;
    ihtp_fixed_decoder_t dec;
    size_t consumed = 0;
    const char *body = nullptr;
    const char *next = nullptr;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_RESPONSE);
    memset(&resp, 0, sizeof(resp));
    if (ihtp_parse_response_stateful(&state, wire, len, &resp, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_response_apply_semantics(&resp, &policy) != IHTP_OK) {
        return false;
    }
    if (resp.body_mode != IHTP_BODY_FIXED || resp.content_length == 0) {
        return false;
    }

    body = wire + consumed;
    ihtp_fixed_decoder_init(&dec, resp.content_length);
    if (ihtp_decode_fixed(&dec, (size_t)resp.content_length) != IHTP_OK) {
        return false;
    }

    next = body + resp.content_length;
    ihtp_parser_state_reset(&state);
    memset(&resp, 0, sizeof(resp));
    consumed = 0;
    if (ihtp_parse_response_stateful(&state, next, strlen(next), &resp, &policy, &consumed) !=
        IHTP_OK) {
        return false;
    }
    if (ihtp_response_apply_semantics(&resp, &policy) != IHTP_OK) {
        return false;
    }

    *work_bytes_out = len + strlen(next);
    return true;
}

static bool scenario_consumer_ioguard_reject_te_cl(const char *wire, size_t len,
                                                   size_t *work_bytes_out)
{
    ihtp_policy_t policy = IHTP_POLICY_IOGUARD;
    ihtp_request_t req;
    ihtp_parser_state_t state;
    size_t consumed = 0;

    ihtp_parser_state_init(&state, IHTP_PARSER_MODE_REQUEST);
    memset(&req, 0, sizeof(req));
    if (ihtp_parse_request_stateful(&state, wire, len, &req, &policy, &consumed) != IHTP_OK) {
        return false;
    }
    if (ihtp_request_apply_semantics(&req, &policy) != IHTP_ERROR) {
        return false;
    }

    *work_bytes_out = consumed;
    return true;
}

static void bench_scenario(output_mode_t mode, const extended_scenario_t *sc, size_t iterations)
{
    size_t consumed = 0;
    uint64_t start_ns = 0;
    uint64_t elapsed_ns = 0;
    double elapsed_s = 0.0;
    double req_per_s = 0.0;
    double mib_per_s = 0.0;
    double ns_per_op = 0.0;

    for (size_t warmup = 0; warmup < 1000; warmup++) {
        if (!sc->fn(sc->wire, sc->len, &consumed)) {
            fprintf(stderr, "warmup failed: scenario=%s\n", sc->name);
            exit(2);
        }
        g_sink += (uint64_t)consumed;
    }

    start_ns = monotonic_ns();
    for (size_t iter = 0; iter < iterations; iter++) {
        if (!sc->fn(sc->wire, sc->len, &consumed)) {
            fprintf(stderr, "run failed: scenario=%s iter=%zu\n", sc->name, iter);
            exit(2);
        }
        g_sink += (uint64_t)consumed;
    }
    elapsed_ns = monotonic_ns() - start_ns;
    elapsed_s = (double)elapsed_ns / 1e9;
    req_per_s = elapsed_s > 0.0 ? (double)iterations / elapsed_s : 0.0;
    mib_per_s =
        elapsed_s > 0.0 ? (((double)iterations * (double)consumed) / (1024.0 * 1024.0)) / elapsed_s
                        : 0.0;
    ns_per_op = iterations > 0 ? (double)elapsed_ns / (double)iterations : 0.0;

    if (mode == OUTPUT_TSV) {
        printf("%s\t%s\t%s\t%zu\t%zu\t%" PRIu64 "\t%.2f\t%.2f\t%.2f\n", sc->group, sc->name,
               sc->baseline, sc->len, consumed, elapsed_ns, req_per_s, mib_per_s, ns_per_op);
        return;
    }

    printf("%-32s req/s=%12.2f  MiB/s=%10.2f  ns/op=%10.2f  baseline=%s\n", sc->name, req_per_s,
           mib_per_s, ns_per_op, sc->baseline);
}

int main(int argc, char **argv)
{
    output_mode_t mode = OUTPUT_HUMAN;
    size_t iterations = 200000;
    const char *scenario_filter = nullptr;

    static const char stateful_reuse_wire[] =
        "GET /health HTTP/1.1\r\nHost: example.com\r\nConnection: keep-alive\r\n\r\n";
    static const char request_chunked_wire[] =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Trailer: Digest\r\n"
        "\r\n"
        "4\r\n"
        "Wiki\r\n"
        "5\r\n"
        "pedia\r\n"
        "0\r\n"
        "Digest: sha-256=abc\r\n"
        "\r\n";
    static const char consumer_iohttp_pipeline_wire[] =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Trailer: Digest\r\n"
        "\r\n"
        "4\r\n"
        "Wiki\r\n"
        "5\r\n"
        "pedia\r\n"
        "0\r\n"
        "Digest: sha-256=abc\r\n"
        "\r\n"
        "GET /next HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    static const char consumer_iohttp_expect_wire[] =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Expect: 100-continue\r\n"
        "Trailer: Digest\r\n"
        "\r\n"
        "4\r\n"
        "Wiki\r\n"
        "5\r\n"
        "pedia\r\n"
        "0\r\n"
        "Digest: sha-256=demo\r\n"
        "\r\n"
        "GET /final HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    static const char response_upgrade_wire[] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
        "\r\n"
        "\x81\x05hello";
    static const char response_fixed_wire[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";
    static const char consumer_iohttp_fixed_wire[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello"
        "HTTP/1.1 204 No Content\r\n"
        "Connection: close\r\n"
        "\r\n";
    static const char connect_wire[] =
        "CONNECT tunnel.example:443 HTTP/1.1\r\n"
        "Host: tunnel.example:443\r\n"
        "\r\n";
    static const char reject_te_cl_wire[] =
        "POST /strict HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Length: 3\r\n"
        "\r\n";

    const extended_scenario_t scenarios[] = {
        {
            .name = "stateful-reuse-request",
            .group = "parser-state",
            .baseline = "req-small/iohttpparser-stateful-strict",
            .wire = stateful_reuse_wire,
            .len = sizeof(stateful_reuse_wire) - 1U,
            .fn = scenario_stateful_reuse_request,
        },
        {
            .name = "request-chunked-parse",
            .group = "semantics-body",
            .baseline = "req-headers/iohttpparser-stateful-strict",
            .wire = request_chunked_wire,
            .len = sizeof(request_chunked_wire) - 1U,
            .fn = scenario_request_chunked_parse_only,
        },
        {
            .name = "request-chunked-parse-semantics",
            .group = "semantics-body",
            .baseline = "request-chunked-parse",
            .wire = request_chunked_wire,
            .len = sizeof(request_chunked_wire) - 1U,
            .fn = scenario_request_chunked_parse_semantics,
        },
        {
            .name = "request-chunked-parse-semantics-body",
            .group = "semantics-body",
            .baseline = "request-chunked-parse-semantics",
            .wire = request_chunked_wire,
            .len = sizeof(request_chunked_wire) - 1U,
            .fn = scenario_request_chunked_parse_semantics_body,
        },
        {
            .name = "consumer-iohttp-pipeline",
            .group = "consumer-iohttp",
            .baseline = "request-chunked-parse-semantics-body",
            .wire = consumer_iohttp_pipeline_wire,
            .len = sizeof(consumer_iohttp_pipeline_wire) - 1U,
            .fn = scenario_consumer_iohttp_pipeline,
        },
        {
            .name = "consumer-iohttp-expect-trailers",
            .group = "consumer-iohttp",
            .baseline = "request-chunked-parse-semantics-body",
            .wire = consumer_iohttp_expect_wire,
            .len = sizeof(consumer_iohttp_expect_wire) - 1U,
            .fn = scenario_consumer_iohttp_expect_trailers,
        },
        {
            .name = "response-fixed-parse-semantics-body",
            .group = "semantics-body",
            .baseline = "resp-headers/iohttpparser-stateful-strict",
            .wire = response_fixed_wire,
            .len = sizeof(response_fixed_wire) - 1U,
            .fn = scenario_response_fixed_parse_semantics_body,
        },
        {
            .name = "consumer-iohttp-fixed-response",
            .group = "consumer-iohttp",
            .baseline = "response-fixed-parse-semantics-body",
            .wire = consumer_iohttp_fixed_wire,
            .len = sizeof(consumer_iohttp_fixed_wire) - 1U,
            .fn = scenario_consumer_iohttp_fixed_response,
        },
        {
            .name = "response-upgrade-parse",
            .group = "upgrade",
            .baseline = "resp-upgrade/iohttpparser-stateful-strict",
            .wire = response_upgrade_wire,
            .len = sizeof(response_upgrade_wire) - 1U,
            .fn = scenario_response_upgrade_parse_only,
        },
        {
            .name = "response-upgrade-parse-semantics",
            .group = "upgrade",
            .baseline = "response-upgrade-parse",
            .wire = response_upgrade_wire,
            .len = sizeof(response_upgrade_wire) - 1U,
            .fn = scenario_response_upgrade_parse_semantics,
        },
        {
            .name = "consumer-ioguard-connect",
            .group = "consumer-ioguard",
            .baseline = "req-connect/iohttpparser-stateful-strict",
            .wire = connect_wire,
            .len = sizeof(connect_wire) - 1U,
            .fn = scenario_consumer_ioguard_connect,
        },
        {
            .name = "consumer-ioguard-reject-te-cl",
            .group = "consumer-ioguard",
            .baseline = "request-chunked-parse-semantics",
            .wire = reject_te_cl_wire,
            .len = sizeof(reject_te_cl_wire) - 1U,
            .fn = scenario_consumer_ioguard_reject_te_cl,
        },
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--format=tsv") == 0) {
            mode = OUTPUT_TSV;
            continue;
        }
        if (strncmp(argv[i], "--iterations=", 13) == 0) {
            iterations = (size_t)strtoull(argv[i] + 13, nullptr, 10);
            continue;
        }
        if (strncmp(argv[i], "--scenario=", 11) == 0) {
            scenario_filter = argv[i] + 11;
            continue;
        }
        fprintf(stderr, "unknown option: %s\n", argv[i]);
        return 2;
    }

    if (mode == OUTPUT_TSV) {
        puts("scenario_group\tscenario\tbaseline\twire_len\twork_len\telapsed_ns\treq_per_s\tmib_per_s\tns_per_op");
    }

    for (size_t i = 0; i < sizeof(scenarios) / sizeof(scenarios[0]); i++) {
        if (scenario_filter != nullptr && strcmp(scenarios[i].name, scenario_filter) != 0) {
            continue;
        }
        bench_scenario(mode, &scenarios[i], iterations);
    }

    return (int)(g_sink == 0);
}
