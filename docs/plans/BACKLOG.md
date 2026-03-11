# Backlog

This file tracks non-immediate items that are intentionally outside the current
execution queue.

## Technical backlog

- Decide whether SSE4.2 token validation should remain scalar-backed or gain a
  dedicated SIMD implementation with proof obligations.
- Revisit whether `IHTP_POLICY_IOHTTP` and `IHTP_POLICY_IOGUARD` should diverge
  after the consumer integration campaign.
- Consider a broader public example for response-side upgrade handoff if
  integration feedback shows that the current minimal sample is not sufficient.

## Process backlog

- Revisit CI/release gating only after the functional-completion and
  integration-comparison phases are done.
- Consider whether generated API docs should be published as an artifact or
  Pages workflow after the first release-candidate gate is defined.
- Recheck repository metadata and support files against the C23 project
  recommendations before release candidate work starts.
