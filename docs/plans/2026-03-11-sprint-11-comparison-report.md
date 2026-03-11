# Sprint 11 Comparison Report

## Scope

This report covers the first comparison campaign after Sprint 10 consumer integration was merged.

The campaign focuses on:

- parser-level comparison against `picohttpparser` and `llhttp`
- semantics-level comparison against `llhttp`
- host SIMD capability and backend-equivalence validation
- consumer-relevant scenarios added during Sprint 10

## Method

1. Reuse the maintained differential harnesses:
   - `tests/unit/test_differential_corpus.c`
   - `tests/unit/test_semantics_differential.c`
2. Extend the corpus with Sprint 10 consumer-sensitive cases:
   - `CONNECT` request authority-form parsing
   - `101 Switching Protocols` response parsing
   - request/response protocol-upgrade semantics
3. Validate scanner equivalence on the current host:
   - `test_scanner_backends`
   - `test_scanner_corpus`
4. Measure direct backend performance with:
   - `bench/bench_parser.c`
   - TSV output saved under `docs/tmp/comparison/results/`

## Reference Snapshots

- upstream `picohttpparser`: `f832609`
- upstream `llhttp`: `c893b86`

These snapshots were downloaded into `docs/tmp/comparison/` for the comparison campaign.

## Host SIMD Baseline

The current host is `x86_64` under KVM and exposes:

- `sse4_2`
- `avx2`

This means the Sprint 11 scanner campaign can exercise:

- scalar
- SSE4.2
- AVX2
- runtime dispatch on top of the same host

## Results

### Differential Campaign

The extended comparison cluster passed:

- `test_differential_corpus`
- `test_semantics_differential`
- `test_scanner_backends`
- `test_scanner_corpus`
- `test_iohttp_integration`

Covered consumer-sensitive additions now include:

- parser-level `CONNECT` request comparison against `picohttpparser` and `llhttp`
- parser-level `101 Switching Protocols` response comparison against `picohttpparser` and `llhttp`
- semantics-level request upgrade comparison against `llhttp`
- semantics-level response `101 Upgrade` comparison against `llhttp`

### Classified Divergences

- `llhttp` response-upgrade semantics treats `keep_alive` differently from `iohttpparser`.
  `iohttpparser` marks HTTP persistence as ended once protocol ownership transfers.
  The differential corpus now records this explicitly with separate reference and `ihtp` expectations.
- `llhttp` upgrade paths surface as `HPE_PAUSED_UPGRADE`, so the Sprint 11 harness maps that to successful parsing/semantics capture instead of treating it as a generic error.
- No new unexplained request or response parser divergences were found in the added `CONNECT` and `101 Switching Protocols` cases.

### RFC-Grounded Interpretation

- The `101 Upgrade` keep-alive split is not treated as a defect in `iohttpparser`.
  For consumer-facing handoff, the library follows the stricter ownership model:
  once the protocol switches, HTTP persistence semantics are no longer surfaced as reusable keep-alive state.
- This matches the already documented embedder contract for `iohttp` and `ioguard`:
  upgraded bytes become consumer-owned immediately after the header block.

### SIMD Verification

Scanner equivalence checks passed on this host for:

- scalar
- SSE4.2
- AVX2
- dispatch-vs-direct backend behavior

Benchmark raw data is saved under:

- `docs/tmp/comparison/results/scanner-bench.tsv`

Supporting host/reference metadata is saved under:

- `docs/tmp/comparison/results/host.txt`
- `docs/tmp/comparison/results/references.txt`

### Benchmark Summary

Average `ns/op` across the 64 recorded backend/case rows:

- `avx2`: `8.41`
- `dispatch`: `8.52`
- `sse42`: `9.12`
- `scalar`: `11.17`

Practical interpretation:

- `dispatch` tracks `avx2` closely on this host because the active backend is `avx2`.
- `avx2` wins several realistic scan paths, including:
  - `query-delim`
  - `status-space`
  - `header` token validation
  - `wide-valid` token validation
- `sse42` still wins some specific long-search cases:
  - `long-header-crlf`
  - `wide-not-found`
- `scalar` remains best on a few very short microcases:
  - `request-space`
  - `conn-token`
  - `invalid-colon`

Engineering conclusion:

- keep runtime dispatch enabled
- keep scalar as the baseline truth implementation
- do not force additional SIMD specialization simply because a SIMD backend exists
- continue treating token-validation SIMD work as proof-driven, not benchmark-driven alone

### Tuning Batch 1 (Executed)

Low-risk parser hot-path changes were applied in `src/ihtp_parser.c`:

- method lookup changed from table loop to length-based switch dispatch
- scanner vtable pointer cached per parse function (`parse_request_line`, `parse_header_block`)
- reverse-space search changed to `memrchr` in request-line parsing

Verification:

- full `./scripts/quality.sh` passed
- differential and integration tests stayed green

Standalone throughput rerun (`iterations=200000`, same isolated parser-only harness):

| Parser | Avg req/s | Avg MiB/s | Avg ns/req |
|---|---:|---:|---:|
| `iohttpparser-lenient` | `8,107,738.12` | `677.89` | `132.43` |
| `iohttpparser-strict` | `8,519,675.17` | `708.94` | `125.69` |
| `llhttp` | `9,487,660.67` | `783.46` | `118.89` |
| `picohttpparser` | `25,727,346.57` | `2,039.21` | `44.62` |

CONNECT-focused rerun:

| Parser | req/s | MiB/s | ns/req |
|---|---:|---:|---:|
| `iohttpparser-lenient` | `7,816,358.72` | `723.06` | `127.94` |
| `iohttpparser-strict` | `7,334,469.37` | `678.49` | `136.34` |
| `llhttp` | `9,929,785.49` | `918.57` | `100.71` |
| `picohttpparser` | `21,839,604.48` | `2,020.30` | `45.79` |

Result:

- this batch did not show a clear throughput win for `iohttpparser`
- changes are functionally safe, but they should not be treated as proven performance improvements on current evidence

Tuning Batch 2 (common-header fast path + same safety gates):

- added a header-name fast path for common names:
  - `Host`
  - `Connection`
  - `Content-Length`
  - `Transfer-Encoding`
  - `Expect`
  - `Trailer`
  - `Upgrade`
- fallback for all other names remains the original scanner token validation path
- full quality gate stayed green

Batch 2 standalone throughput rerun:

| Parser | Avg req/s | Avg MiB/s | Avg ns/req |
|---|---:|---:|---:|
| `iohttpparser-lenient` | `8,858,610.31` | `725.51` | `123.83` |
| `iohttpparser-strict` | `9,105,549.66` | `741.24` | `121.34` |
| `llhttp` | `9,793,357.86` | `804.28` | `113.11` |
| `picohttpparser` | `24,502,925.38` | `1,965.90` | `46.29` |

Batch 2 CONNECT-focused rerun:

| Parser | req/s | MiB/s | ns/req |
|---|---:|---:|---:|
| `iohttpparser-lenient` | `7,791,801.51` | `720.79` | `128.34` |
| `iohttpparser-strict` | `8,215,341.18` | `759.97` | `121.72` |
| `llhttp` | `9,719,908.68` | `899.15` | `102.88` |
| `picohttpparser` | `25,135,082.22` | `2,325.16` | `39.79` |

Observed impact vs Batch 1:

- `iohttpparser-strict` avg req/s: `+6.9%` (`8.52M -> 9.11M`)
- `iohttpparser-lenient` avg req/s: `+9.3%` (`8.11M -> 8.86M`)
- strict `CONNECT` req/s: `+12.0%` (`7.33M -> 8.22M`)

Conclusion:

- Batch 2 is the first tuning step in this sequence that shows a consistent positive signal for `iohttpparser` while preserving behavior and test parity.

Next optimization focus with higher expected payoff and no contract changes:

1. fast path for frequent header names before generic token validation
2. reduce repeated header-value trim overhead in the generic path
3. evaluate each tuning patch using 5-run median results (to suppress run-to-run noise)

### Standalone Throughput Harness

The standalone parser-only throughput comparison is now captured in a repository benchmark target:

- `bench/bench_throughput_compare.c`
- `scripts/run-throughput-compare.sh`

This harness is intentionally independent from `iohttp` / `ioguard` so parser cost can be measured
without consumer-side buffering, scheduling, or TLS overhead.

Recommended runs:

- common matrix: `FORMAT=tsv ITERATIONS=200000 bash scripts/run-throughput-compare.sh`
- `CONNECT`-focused: `FORMAT=tsv CONNECT_ONLY=1 ITERATIONS=200000 bash scripts/run-throughput-compare.sh`

Current reproducible results from the repository harness (`200000` iterations):

Common 5-scenario matrix:

| Parser | Avg req/s | Avg MiB/s | Avg ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `27,516,765.75` | `2,005.68` | `44.27` |
| `llhttp` | `13,928,436.53` | `1,053.44` | `82.50` |
| `iohttpparser-strict` | `12,029,852.83` | `893.12` | `99.50` |
| `iohttpparser-lenient` | `9,918,687.37` | `737.93` | `119.70` |

Per-scenario `req/s`:

| Scenario | `iohttpparser-strict` | `iohttpparser-lenient` | `llhttp` | `picohttpparser` |
|---|---:|---:|---:|---:|
| `req-small` | `14,133,141.97` | `13,810,868.99` | `21,230,327.18` | `41,119,587.69` |
| `req-headers` | `5,517,044.44` | `5,466,941.00` | `7,407,958.07` | `13,398,946.19` |
| `resp-small` | `19,183,402.83` | `15,936,007.37` | `16,935,235.51` | `41,272,301.23` |
| `resp-headers` | `8,825,395.46` | `6,547,334.73` | `9,946,674.88` | `17,578,092.21` |
| `resp-upgrade` | `12,490,279.44` | `7,832,284.78` | `14,121,986.99` | `24,214,901.44` |

`CONNECT`-focused matrix:

| Parser | req/s | MiB/s | ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `24,034,125.57` | `2,269.15` | `41.61` |
| `llhttp` | `10,930,685.57` | `1,032.01` | `91.49` |

## Upstream Design Signals

Official upstream documentation points in a consistent direction:

- `llhttp` explicitly positions itself as a port of `http_parser` to `llparse`, with the output C parser
  generated from a state-machine description instead of handwritten parser code.
- The same `llhttp` README says that optimizations and multi-character matching are generated
  automatically, which lowers maintenance cost while keeping the parser core flat and fast.
- `picohttpparser` explicitly describes itself as a tiny, primitive, fast HTTP parser that is stateless,
  allocates no memory, and only sets pointers into the caller-owned buffer.

Practical interpretation for this comparison:

- `llhttp` is not "cheating"; it wins part of its throughput from generated DFA-style structure and a
  narrower embedder contract.
- `picohttpparser` is faster still because it intentionally does less work and exposes a thinner API.
- `iohttpparser` sits in a different design point: a pull-style parser with typed outputs, explicit
  semantics/framing application, and consumer-facing ownership flags.

## What `iohttpparser` Covers That The References Do Not

`iohttpparser` intentionally covers more wire-level responsibility than `picohttpparser`, and packages
that responsibility differently than `llhttp`.

Meaningful extra scope already present in this repository:

- typed request/response/header/body structures instead of callback-only or raw pointer-only output
- explicit parser state API for incremental consumers
- separate semantics layer for framing and ambiguity resolution
- body decoder with chunked/fixed-length handoff contracts
- named consumer presets for `iohttp` and `ioguard`
- consumer-facing ownership flags:
  - `protocol_upgrade`
  - `expects_continue`
  - `has_trailer_fields`
- maintained differential and consumer-integration test layers

These responsibilities are appropriate inside this library when they remain strictly wire-level:

- request/status line parsing
- header parsing and field-value validity
- framing decisions
- `Transfer-Encoding` / `Content-Length` ambiguity handling
- `CONNECT` / `Upgrade` / `100 Continue` / trailer handoff metadata
- chunked and fixed-length body decode

These responsibilities do **not** belong here and should remain above the library:

- URI normalization / decoding
- routing
- cookie parsing
- auth policy
- content decoding
- WebSocket frame parsing
- application-level upgrade protocols

So the current scope is correct, but it must stay disciplined: secure wire semantics yes, HTTP application
behavior no.

## Why We Are Still Slower Than `llhttp`

After the Sprint 13 localization passes, there is no evidence that the remaining gap comes from request-line
handling or from SIMD dispatch.

The strongest evidence points to the generic header path:

- `hdr-common-heavy` improved materially once common header names got a fast path
- `hdr-uncommon-valid` remains the most expensive header-focused scenario
- `req-pico-bench` degrades in the same direction, which means long header-heavy requests amplify the same cost

This is consistent with the implementation differences:

- `llhttp` pays for a generated state machine and callback streaming model
- `picohttpparser` pays for very little beyond structure detection and pointer slicing
- `iohttpparser` pays extra constant factors for:
  - richer typed output
  - generic uncommon-header validation
  - value trimming/validation
  - explicit pull-style consumer contract

There is no miracle left to discover here. The remaining gap is the sum of:

- a flatter parser core in `llhttp`
- a much thinner contract in `picohttpparser`
- our own extra parser-layer work on every uncommon/long header path

## Negative Result: Table-Driven Field-Value Validation

One mathematically plausible optimization was tested in Sprint 13 and rejected:

- replace per-byte field-value classification in `trim_and_validate_field_value()` with a table-driven lookup

The rationale was sound on paper:

- finite alphabet of `256` byte values
- branch entropy on header-heavy workloads
- membership test reducible to table lookup

But the measured result was negative.

Focused 5-run median comparison on the same host (`RUNS=5`, `ITERATIONS=100000`):

| Scenario | Baseline strict req/s | Table strict req/s | Delta |
|---|---:|---:|---:|
| `hdr-uncommon-valid` | `6,034,987.72` | `4,859,242.33` | `-19.5%` |
| `req-headers` | `5,436,421.81` | `4,643,772.50` | `-14.6%` |
| `req-pico-bench` | `1,632,850.95` | `1,221,283.53` | `-25.2%` |

Why it likely regressed:

- the original predicate is already simple and compiler-friendly
- the table-driven version added an extra dependent memory load per byte
- on these workloads, the added load cost dominated any branch-prediction win

Engineering decision:

- do not pursue table-driven field-value classification further
- prefer optimizations that remove work, not optimizations that replace arithmetic with extra memory traffic

## Tuning Directions That Still Look Safe

The next safe directions should stay narrow and evidence-driven:

1. keep using focused scenario groups:
   - `hdr-uncommon-valid`
   - `req-headers`
   - `req-pico-bench`
2. optimize only when the 5-run median improves on those scenarios
3. reject changes that merely reshuffle work without reducing it

Promising directions:

- reduce generic uncommon-header overhead without adding table lookups
- reduce repeated pointer arithmetic in long header lines
- consider generated or semi-generated parser techniques only if they preserve the current public contract

Unpromising directions based on current evidence:

- branchless/table-driven field-value validation
- wider SIMD in parser logic without proof that scanner cost is the limiting factor
- moving wire-level ambiguity handling out of the library just to gain parser throughput
| `iohttpparser-strict` | `9,525,021.02` | `899.29` | `104.99` |
| `iohttpparser-lenient` | `8,560,968.78` | `808.27` | `116.81` |

### Sprint 13 Micro-Localization Findings

To localize the remaining gap against `llhttp`, the harness was extended with focused request-side
scenarios:

- `req-line-hot`
- `hdr-common-heavy`
- `hdr-uncommon-valid`

These scenarios separate:

- mostly request-line cost
- common-header fast-path cost
- uncommon-header and value-validation cost

Observed pattern:

- the gap versus `llhttp` is present on request-line work, but it becomes materially larger on the
  generic header path
- `hdr-common-heavy` narrows the gap relative to `hdr-uncommon-valid`, which supports the earlier
  conclusion that the common-header fast path is useful
- `hdr-uncommon-valid` remains the clearest evidence that uncommon header-name handling plus value
  validation is the hottest remaining parser cost center

Sprint 13 safe tuning step:

- non-continuation header values now use a single-pass trim+validate helper instead of:
  - trim-left
  - trim-right
  - validate

Initial signal after this change:

- request/header-heavy cases improved materially:
  - `req-headers`
  - `hdr-common-heavy`
  - `hdr-uncommon-valid`
- response-side results were noisier on a single run, so the next acceptance step should be
  5-run median reporting before treating this as a settled win

Engineering read:

- the next high-value safe optimization area is still the generic header path
- request-line work matters, but it does not currently explain the whole remaining gap to `llhttp`
- no evidence yet suggests hidden “cheating” by `llhttp`; the simpler explanation is still lower
  parser-core cost plus a narrower embedding contract

### `picohttpparser bench.c` Workload Check

The repository harness now also includes the long request shape used by upstream
`picohttpparser/bench.c` as `req-pico-bench`.

5-run median (`ITERATIONS=100000`):

| Parser | Median req/s | Median MiB/s | Median ns/req |
|---|---:|---:|---:|
| `picohttpparser` | `7,600,239.56` | `5,095.45` | `131.57` |
| `llhttp` | `3,173,277.25` | `2,127.47` | `315.13` |
| `iohttpparser-lenient` | `1,556,839.11` | `1,043.76` | `642.33` |
| `iohttpparser-strict` | `1,545,166.32` | `1,035.93` | `647.18` |

Interpretation:

- this workload is heavily request-header dominated, so it amplifies the cost of generic header
  parsing and validation
- the result is consistent with the earlier micro-localization:
  - request-line cost is not the main problem
  - generic header-name and header-value handling are the larger remaining cost center
- the tiny gap between strict and lenient here also suggests that the dominant cost is not mainly
  strict-policy branching; it is the always-on structural work in the parser path

### Sprint 13 Follow-up Tuning Signal

The next safe tuning step combined two changes in the generic header path:

- single-pass trim+validate for non-continuation header values
- single-pass header-name validation while searching for `:`

Observed single-run impact on the focused scenarios:

| Scenario | `iohttpparser-strict` before | `iohttpparser-strict` after | Approx. delta |
|---|---:|---:|---:|
| `req-headers` | `5,411,711.07` | `5,784,550.15` | `+6.9%` |
| `hdr-common-heavy` | `8,019,308.89` | `8,155,851.80` | `+1.7%` |
| `hdr-uncommon-valid` | `5,510,077.06` | `6,085,385.75` | `+10.4%` |
| `req-pico-bench` | `1,545,166.32` | `1,633,692.92` | `+5.7%` |

Interpretation:

- the uncommon/generic header path was indeed a real cost center
- the new one-pass header-name/value work is directionally correct
- `iohttpparser-strict` now slightly edges `llhttp` on `hdr-common-heavy`, which suggests the
  common-header path is no longer the main problem
- the remaining gap is now concentrated more clearly in uncommon header handling and richer
  parser-contract work on large header-heavy requests

Next measurement requirement:

- keep using 5-run median before accepting any additional micro-optimization as a real win
- prefer focused scenarios (`hdr-uncommon-valid`, `req-pico-bench`) over coarse all-scenario
  averages when evaluating the next parser hot-path changes

### Why `iohttpparser` Still Trails `llhttp`

The current evidence does not suggest hidden magic. The remaining gap is explainable by contract shape
and hot-path cost.

Most plausible cost centers in `iohttpparser`:

1. Multi-pass header parsing
- one header line currently goes through separate scans for line ending, colon, common-name/token
  validation, OWS trimming, and field-text validation.
- `llhttp` is closer to a single-pass state machine on the same bytes.

2. Always-on field-value validation
- `field_text_is_valid` walks every header value and continuation span.
- this is safety work that minimal parsers often defer or omit.

3. Non-common header names still use generic token validation
- the common-header fast path reduced cost for popular names, but uncommon headers still fall back to
  generic validation.

4. Richer parser-layer contract
- `iohttpparser` does more than “find token boundaries”.
- it fills typed request/response structures, preserves zero-copy spans, enforces stricter line and
  token rules, and keeps a cleaner consumer-facing pull API.

5. `llhttp` is optimized for a different embedding model
- `llhttp` is a generated callback-oriented state machine with very low parser-core abstraction
  overhead.
- that design is efficient, but it does not expose the same typed pull-style contract that
  `iohttp`/`ioguard` want from `iohttpparser`.

In short: the remaining gap is not evidence that `llhttp` is “cheating”; it is mostly the cost of a
broader parser-layer contract plus a hotter multi-pass header path.

### Where `iohttpparser` Is Better Than `picohttpparser` and `llhttp`

`iohttpparser` is not trying to be only a byte scanner. Its differentiator is the consumer contract.

What it provides beyond the raw parser baselines:

- explicit split contract:
  - `parse -> apply_semantics -> decode_body`
- typed outputs for consumer decisions:
  - `body_mode`
  - `content_length`
  - `keep_alive`
  - `protocol_upgrade`
  - `expects_continue`
  - `has_trailer_fields`
- public stateful pull API for accumulated-buffer incremental parsing
- named policy presets:
  - `IHTP_POLICY_IOHTTP`
  - `IHTP_POLICY_IOGUARD`
- maintained differential corpus against `picohttpparser` and `llhttp`
- explicit ownership and zero-copy handoff rules for:
  - upgraded protocol bytes
  - trailers
  - body-decoder leftovers

Where the competitors cover less or differently:

- `picohttpparser`
  - excellent minimal parsing baseline
  - much lighter API
  - fewer typed semantics outputs
  - less consumer-facing contract surface
- `llhttp`
  - very strong parser state machine
  - efficient callback-based embedding
  - different integration model than a stateful pull-style parser with typed semantic handoff

### Are These Extra Responsibilities in the Right Layer?

Mostly yes, with an important boundary.

Responsibilities that are appropriate for `iohttpparser`:

- HTTP/1.1 wire parsing
- framing resolution
- `Content-Length` / `Transfer-Encoding` interpretation
- strict-vs-lenient policy enforcement
- incremental chunked decoding
- parser-adjacent semantic flags needed immediately by transport consumers

Responsibilities that should stay outside this library:

- URI normalization and routing
- cookie/session policy
- multipart parsing
- content-coding decompression
- application auth/business rules
- server policy about how to act on `100 Continue`

That boundary is important: `iohttpparser` should expose enough structured semantics to make
`iohttp`/`ioguard` simple and safe, but it should not grow into a full application-layer HTTP stack.

## Assessment

The comparison posture is now materially stronger than before Sprint 11:

- Sprint 10 consumer scenarios are no longer integration-only; they are also represented in the maintained differential matrix where the reference libraries can express them.
- Known reference differences are now encoded explicitly instead of being left as implicit surprises.
- SIMD validation has been rechecked on a host that actually exposes both `sse4_2` and `avx2`.
- Consumer-facing ownership decisions now have comparison evidence behind them, not just unit-level assertions.

## Next Step

The next batch should move from comparison expansion to broader reporting and integration interpretation:

- decide whether any additional comparison cases are still missing for `iohttp` / `ioguard`
- keep the `101 Upgrade` keep-alive divergence classified as intentional unless a consumer disproves it
- treat the current TSV results as a baseline for future performance work, not as a release gate by themselves
