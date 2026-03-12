# PMI/PSI Artifact Contract

Repository-published functional and performance test artifacts live here.

## Layout

```text
tests/artifacts/pmi-psi/
  README.md
  index.tsv
  latest.txt
  runs/<run_id>/
```

## Required run files

- `manifest.json`
- `run.txt`
- `host.txt`
- `toolchain.txt`
- `ctest.txt`
- `throughput.tsv`
- `throughput-connect.tsv`
- `throughput-median.tsv`
- `throughput-connect-median.tsv`
- `throughput-extended.tsv`
- `throughput-extended-median.tsv`
- `charts/common-three-way.svg`
- `charts/connect-three-way.svg`
- `charts/extended-parser-state.svg`
- `charts/extended-semantics-body.svg`
- `charts/extended-consumer-iohttp.svg`
- `charts/extended-upgrade-ioguard.svg`
- `summary.md`
- `summary-extended.md`
