# iohttpparser Release Management

## Purpose

Use this skill when preparing a release candidate, final release, tag, release
notes, or changelog update for `iohttpparser`.

## Required standards

- Follow [Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/).
- Follow [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
- Use Git tags with a leading `v`.

## Versioning policy

- `v0.y.z` is a normal pre-1.0 release.
- `v0.y.z-rc.N` is a release candidate.
- `v1.0.0` is reserved for the first release with a stable public contract,
  stable release gate, and published verification evidence.
- The current release line starts at `v0.1.0-rc.N`.

## Changelog rules

- Update `CHANGELOG.md` in the same branch as the change.
- Keep `## [Unreleased]` at the top.
- Move completed work into a dated version section when preparing a tag.
- Use only the standard Keep a Changelog sections:
  - `Added`
  - `Changed`
  - `Deprecated`
  - `Removed`
  - `Fixed`
  - `Security`
- Write one factual bullet per externally visible change.

## Release workflow

1. Verify `main` is clean and synchronized.
2. Update `CHANGELOG.md`.
3. Update release-candidate or release documentation if the published evidence
   set changed.
4. Run the release gate.
5. Create an annotated tag.
6. Push the tag.
7. Verify the GitHub release workflow and published release assets.
