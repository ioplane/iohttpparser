[![GitHub](https://img.shields.io/badge/GitHub-iohttpparser-181717)](https://github.com/ioplane/iohttpparser)

# iohttpparser Agent Development Plan

Date: 2026-10-26. Audited default branch: `main` at
`bfb3486103a95cfdf87ca927c8dfa9e461dadab6`. Planning audit: `ngtun-i8s.42.2`.
Durable execution work order: `ngtun-4zy.2` under independent portfolio `ngtun-4zy`.

Historical plans and local uncommitted August plans remain preserved. This dated
plan supplies the next validation/implementation contracts; it does not rewrite past
measurements, sprint completion records or current publish decisions.

## Verified baseline

The default repository implements separate scanner, parser, framing semantics and body
decoder layers. README declares strict policy, zero-copy spans, incremental state and
fixed/chunked decoding. The dated final development plan marks its release/evidence
phases complete and identifies tagged-release policy as the next step; those are historical
records, not fresh verification of another revision or toolchain.

## Next functional slice

Refresh the existing strict consumer and release contract on an exact source/image pair.
Use one fragmented CONNECT or chunked HTTP transaction through the existing parser,
semantics and body decoder. Prove consumed-byte accounting and span lifetime across
buffer retention/compaction. Keep transport ownership in iohttp/ioguard.

| Order | Owned paths | Exit evidence |
| --- | --- | --- |
| 1 | Existing consumer fixtures, `include/iohttpparser/` contract and tests | Whole-buffer versus fragmented byte-equivalent results; exact consumed count |
| 2 | Existing semantics/body tests and fuzz corpus | Request-smuggling/TE-CL conflict, overflow, truncation, limits, trailers and post-upgrade bytes rejected or retained as specified |
| 3 | Existing release scripts, candidate checklist and artifacts | Same-SHA container release gate, examples and Doxygen; current common and extended-contract bundle hashes |

## Dependencies and overlap

PR #42 owns the base-image and Python quality/release migration, including candidate docs.
Review and update that PR in place before regenerating evidence with its controller.
Do not duplicate its automation. Consumer API/policy changes require separate iohttp and
ioguard admission; named consumer presets do not prove adoption. Compatibility remains an
explicit profile with independent negative fixtures. Preserve scalar/SSE/AVX equivalence;
no scanner rewrite or speculative SIMD expansion is part of this slice. The selected consumer edges are iohttp and ioguard; qualify each actual HTTP
boundary against the pinned parser and its borrowed-span lifetime contract. Parser
work does not wait for ngtun or shared-core extraction.

## Publication contract

`docs/plans/ROADMAP.md` remains the stable entry point; this dated plan supplements the
historical completion records. Update English docs and agreed Russian mirrors when the
public contract or published evidence changes, plus CHANGELOG. Run repository docs lint,
container quality/release gate, examples and source-first packaging checks before tagging.
Do not turn historical PSI numbers into a current performance claim or create a release
because only a planning document changed.

## Execution and evidence rules

Read the target repository contract and applicable local skills before each leaf.
This independent portfolio does not gate ngtun release or require ngtun adoption.
Use an actually qualified project OCI; shared-base publication gates only its adoption.
Preserve dirty worktrees, unpublished plans, existing PRs and their path ownership.
Refresh exact default/PR identities before execution; keep compatibility explicit.
Record actual nonempty test/analyzer coverage and exact source/tool/image evidence.
Follow the repository task, changelog, review and delivery contract for each leaf.
This planning update claims no runtime success and authorizes no live mutation.
Repository quality gates remain pending until run; no host build or engine was used.

## Source baseline

- [Default README](https://github.com/ioplane/iohttpparser/blob/bfb3486103a95cfdf87ca927c8dfa9e461dadab6/README.md).
- [Default repository contract](https://github.com/ioplane/iohttpparser/blob/bfb3486103a95cfdf87ca927c8dfa9e461dadab6/AGENTS.md); applicable dirty local contracts were read without importing their changes.
- [Existing PR #42](https://github.com/ioplane/iohttpparser/pull/42), audited head `d34517cb594f6fbd184559f50c056e0d8f889dc3`; recheck the patchset before execution.
- [docs/plans/2026-03-11-iohttpparser-final-development-plan.md:161](https://github.com/ioplane/iohttpparser/blob/bfb3486103a95cfdf87ca927c8dfa9e461dadab6/docs/plans/2026-03-11-iohttpparser-final-development-plan.md#L161).
- [docs/en/05-consumer-contracts.md:1](https://github.com/ioplane/iohttpparser/blob/bfb3486103a95cfdf87ca927c8dfa9e461dadab6/docs/en/05-consumer-contracts.md#L1).
- [docs/en/12-release-candidate-checklist.md:43](https://github.com/ioplane/iohttpparser/blob/bfb3486103a95cfdf87ca927c8dfa9e461dadab6/docs/en/12-release-candidate-checklist.md#L43).
