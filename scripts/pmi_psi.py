#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Generate the published iohttpparser PMI/PSI evidence bundle."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
from datetime import UTC, datetime
from pathlib import Path


PARSER_ORDER = [
    "picohttpparser",
    "llhttp",
    "iohttpparser-stateful-strict",
    "iohttpparser-strict",
    "iohttpparser-stateful-lenient",
    "iohttpparser-lenient",
]
SCENARIO_ORDER = [
    "req-small",
    "req-headers",
    "resp-small",
    "resp-headers",
    "resp-upgrade",
    "req-line-only",
    "req-line-hot",
    "req-line-long-target",
    "req-line-connect",
    "req-line-options",
    "req-pico-bench",
    "hdr-common-heavy",
    "hdr-name-heavy",
    "hdr-uncommon-valid",
    "hdr-value-ascii-clean",
    "hdr-value-heavy",
    "hdr-value-obs-text",
    "hdr-value-trim-heavy",
    "hdr-count-04-minimal",
    "hdr-count-16-minimal",
    "hdr-count-32-minimal",
]
EXTENDED_GROUP_ORDER = [
    "parser-state",
    "semantics-body",
    "consumer-iohttp",
    "upgrade",
    "consumer-ioguard",
]
EXTENDED_GROUP_TITLES = {
    "parser-state": "Parser State",
    "semantics-body": "Semantics And Body Handoff",
    "consumer-iohttp": "iohttp-style Consumer Flows",
    "upgrade": "Upgrade Handoff",
    "consumer-ioguard": "ioguard-style Consumer Flows",
}


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def positive_env(name: str, default: str) -> str:
    value = os.environ.get(name, default)
    if not value.isdigit() or int(value) < 1:
        raise SystemExit(f"{name} must be a positive integer")
    return value


def capture(command: list[str], root: Path, *, check: bool = True) -> str:
    result = subprocess.run(
        command,
        cwd=root,
        check=check,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return result.stdout.strip()


def run_to_file(
    command: list[str],
    root: Path,
    output: Path,
    environment: dict[str, str] | None = None,
) -> None:
    with output.open("w", encoding="utf-8") as stream:
        subprocess.run(command, cwd=root, check=True, env=environment, stdout=stream)


def parse_tsv(path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    header: list[str] | None = None
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = line.split("\t")
        if not parts or parts[0] in {"", "format", "meta"}:
            continue
        if parts[0] == "columns":
            header = parts[1:]
            continue
        if header is None:
            header = parts
            continue
        rows.append(dict(zip(header, parts, strict=False)))
    return rows


def render_reqs_matrix(rows: list[dict[str, str]]) -> str:
    grouped: dict[str, dict[str, str]] = {}
    for row in rows:
        grouped.setdefault(row["scenario"], {})[row["parser"]] = row["req_per_s_median"]
    lines = [
        "| Scenario | picohttpparser | llhttp | iohttpparser-stateful-strict | iohttpparser-strict | iohttpparser-stateful-lenient | iohttpparser-lenient |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]
    seen: set[str] = set()
    scenarios = [scenario for scenario in SCENARIO_ORDER if scenario in grouped]
    scenarios.extend(sorted(set(grouped) - set(scenarios)))
    for scenario in scenarios:
        seen.add(scenario)
        data = grouped[scenario]
        lines.append(
            "| {scenario} | {pico} | {llhttp} | {stateful_strict} | {strict} | {stateful_lenient} | {lenient} |".format(
                scenario=scenario,
                pico=data.get(PARSER_ORDER[0], "—"),
                llhttp=data.get(PARSER_ORDER[1], "—"),
                stateful_strict=data.get(PARSER_ORDER[2], "—"),
                strict=data.get(PARSER_ORDER[3], "—"),
                stateful_lenient=data.get(PARSER_ORDER[4], "—"),
                lenient=data.get(PARSER_ORDER[5], "—"),
            )
        )
    return "\n".join(lines)


def write_contract_files(base_dir: Path) -> None:
    base_dir.mkdir(parents=True, exist_ok=True)
    readme = base_dir / "README.md"
    if not readme.exists():
        readme.write_text(
            """# PMI/PSI Artifact Contract

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
- `scanner-bench.tsv`
- `charts/common-three-way.svg`
- `charts/connect-three-way.svg`
- `charts/extended-parser-state.svg`
- `charts/extended-semantics-body.svg`
- `charts/extended-consumer-iohttp.svg`
- `charts/extended-upgrade-ioguard.svg`
- `charts/scanner-backends.svg`
- `summary.md`
- `summary-extended.md`
""",
            encoding="utf-8",
        )
    index = base_dir / "index.tsv"
    if not index.exists():
        index.write_text("run_id\tdate_utc\tgit_head_short\tstatus\n", encoding="utf-8")


def write_summary(out_dir: Path, run_id: str, run_meta: dict[str, str]) -> None:
    median_rows = parse_tsv(out_dir / "throughput-median.tsv")
    connect_rows = parse_tsv(out_dir / "throughput-connect-median.tsv")
    extended_rows = parse_tsv(out_dir / "throughput-extended-median.tsv")
    summary = f"""# PSI Run Summary

Run id: `{run_id}`

## Source code used for the run

- `bench/bench_throughput_compare.c`
- `scripts/throughput.py compare`
- `scripts/throughput.py median`
- `scripts/pmi_psi.py`
- `tests/unit/test_differential_corpus.c`
- `tests/unit/test_semantics_differential.c`
- `tests/unit/test_iohttp_integration.c`

## Functional result

`ctest --preset {run_meta["debug_preset"]} --output-on-failure` completed successfully.

## Common median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-median.tsv`.

{render_reqs_matrix(median_rows)}

## CONNECT median matrix

The table below uses `req/s median` only. Full `MiB/s` and `ns/req` values are
published in `throughput-connect-median.tsv`.

{render_reqs_matrix(connect_rows)}
"""
    (out_dir / "summary.md").write_text(summary, encoding="utf-8")
    grouped: dict[str, list[dict[str, str]]] = {}
    for row in extended_rows:
        grouped.setdefault(row["scenario_group"], []).append(row)
    lines = [
        "# Extended PSI Run Summary",
        "",
        f"Run id: `{run_id}`",
        "",
        "## Scope",
        "",
        "This summary publishes the extended-contract performance layer.",
        "",
        "## Source code used for the run",
        "",
        "- `bench/bench_extended_contract.c`",
        "- `scripts/throughput.py extended`",
        "- `scripts/throughput.py extended-median`",
        "- `scripts/pmi_psi.py`",
        "- `tests/unit/test_iohttp_integration.c`",
        "- `tests/unit/test_semantics.c`",
        "- `tests/unit/test_body_decoder.c`",
        "",
        "## Median matrix",
        "",
        "Full values are published in `throughput-extended-median.tsv`.",
    ]
    for group in EXTENDED_GROUP_ORDER:
        rows = grouped.get(group)
        if not rows:
            continue
        lines.extend(
            [
                "",
                f"### {EXTENDED_GROUP_TITLES.get(group, group)}",
                "",
                "| Scenario | Baseline | req/s median | MiB/s median | ns/op median |",
                "|---|---|---:|---:|---:|",
            ]
        )
        lines.extend(
            f"| {row['scenario']} | {row['baseline']} | {row['req_per_s_median']} | {row['mib_per_s_median']} | {row['ns_per_op_median']} |"
            for row in rows
        )
    (out_dir / "summary-extended.md").write_text(
        "\n".join(lines) + "\n", encoding="utf-8"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    root = root_dir()
    base_dir = Path(
        os.environ.get("BASE_DIR", str(root / "tests" / "artifacts" / "pmi-psi"))
    )
    debug_preset = os.environ.get("DEBUG_PRESET", "clang-debug")
    release_preset = os.environ.get("RELEASE_PRESET", "clang-release")
    iterations = positive_env("ITERATIONS", "200000")
    runs = positive_env("RUNS", "5")
    head_short = capture(["git", "-C", str(root), "rev-parse", "--short", "HEAD"], root)
    run_id = os.environ.get(
        "RUN_ID", f"{datetime.now(UTC).strftime('%Y%m%dT%H%M%SZ')}-{head_short}"
    )
    out_dir = Path(os.environ.get("OUT_DIR", str(base_dir / "runs" / run_id)))
    out_dir.mkdir(parents=True, exist_ok=True)
    write_contract_files(base_dir)
    run_meta = {
        "date_utc": datetime.now(UTC).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "root": str(root),
        "run_id": run_id,
        "debug_preset": debug_preset,
        "release_preset": release_preset,
        "iterations": iterations,
        "median_runs": runs,
    }
    (out_dir / "run.txt").write_text(
        "\n".join(f"{key}={value}" for key, value in run_meta.items()) + "\n",
        encoding="utf-8",
    )

    host_lines = [
        f"kernel={capture(['uname', '-srvmo'], root)}",
        f"machine={capture(['uname', '-m'], root)}",
    ]
    if shutil.which("lscpu"):
        host_lines.extend(
            ["--- lscpu ---", *capture(["lscpu"], root).splitlines()[:80]]
        )
    cpuinfo = Path("/proc/cpuinfo")
    if cpuinfo.is_file():
        flags = next(
            (
                line
                for line in cpuinfo.read_text(encoding="utf-8").splitlines()
                if line.lower().startswith("flags")
            ),
            None,
        )
        if flags:
            host_lines.extend(["--- cpu flags ---", flags])
    (out_dir / "host.txt").write_text("\n".join(host_lines) + "\n", encoding="utf-8")

    tool_commands = {
        "git_head": ["git", "-C", str(root), "rev-parse", "HEAD"],
        "git_head_short": ["git", "-C", str(root), "rev-parse", "--short", "HEAD"],
        "clang": ["clang", "--version"],
        "cmake": ["cmake", "--version"],
        "ctest": ["ctest", "--version"],
        "valgrind": ["valgrind", "--version"],
        "uftrace": ["uftrace", "--version"],
        "gdb": ["gdb", "--version"],
    }
    tool_lines = []
    for name, command in tool_commands.items():
        if shutil.which(command[0]):
            output = capture(command, root, check=False).splitlines()
            tool_lines.append(f"{name}={output[0] if output else 'unavailable'}")
        else:
            tool_lines.append(f"{name}=unavailable")
    (out_dir / "toolchain.txt").write_text(
        "\n".join(tool_lines) + "\n", encoding="utf-8"
    )

    subprocess.run(["cmake", "--preset", debug_preset], cwd=root, check=True)
    subprocess.run(["cmake", "--build", "--preset", debug_preset], cwd=root, check=True)
    run_to_file(
        ["ctest", "--preset", debug_preset, "--output-on-failure"],
        root,
        out_dir / "ctest.txt",
    )
    subprocess.run(
        ["cmake", "--preset", release_preset, "-DIOHTTPPARSER_BUILD_BENCH=ON"],
        cwd=root,
        check=True,
    )
    subprocess.run(
        [
            "cmake",
            "--build",
            "--preset",
            release_preset,
            "--target",
            "bench_throughput_compare",
            "bench_extended_contract",
        ],
        cwd=root,
        check=True,
    )

    def child_environment(**updates: str) -> dict[str, str]:
        environment = os.environ.copy()
        environment.update(updates)
        return environment

    throughput = root / "scripts" / "throughput.py"
    run_to_file(
        ["uv", "run", "--script", str(throughput), "compare"],
        root,
        out_dir / "throughput.tsv",
        child_environment(FORMAT="tsv", ITERATIONS=iterations),
    )
    run_to_file(
        ["uv", "run", "--script", str(throughput), "compare"],
        root,
        out_dir / "throughput-connect.tsv",
        child_environment(FORMAT="tsv", CONNECT_ONLY="1", ITERATIONS=iterations),
    )
    run_to_file(
        ["uv", "run", "--script", str(throughput), "median"],
        root,
        out_dir / "throughput-median.tsv",
        child_environment(ITERATIONS=iterations, RUNS=runs),
    )
    run_to_file(
        ["uv", "run", "--script", str(throughput), "median"],
        root,
        out_dir / "throughput-connect-median.tsv",
        child_environment(CONNECT_ONLY="1", ITERATIONS=iterations, RUNS=runs),
    )
    run_to_file(
        ["uv", "run", "--script", str(throughput), "extended"],
        root,
        out_dir / "throughput-extended.tsv",
        child_environment(FORMAT="tsv", ITERATIONS=iterations),
    )
    run_to_file(
        ["uv", "run", "--script", str(throughput), "extended-median"],
        root,
        out_dir / "throughput-extended-median.tsv",
        child_environment(ITERATIONS=iterations, RUNS=runs),
    )
    run_to_file(
        ["uv", "run", "--script", str(root / "scripts" / "run_scanner_bench.py")],
        root,
        out_dir / "scanner-bench.tsv",
        child_environment(FORMAT="tsv", ITERATIONS=iterations),
    )
    subprocess.run(
        [
            "uv",
            "run",
            "--script",
            str(root / "scripts" / "render-performance-charts.py"),
            str(out_dir),
        ],
        cwd=root,
        check=True,
    )

    files = {
        "run": "run.txt",
        "host": "host.txt",
        "toolchain": "toolchain.txt",
        "ctest": "ctest.txt",
        "throughput": "throughput.tsv",
        "throughput_connect": "throughput-connect.tsv",
        "throughput_median": "throughput-median.tsv",
        "throughput_connect_median": "throughput-connect-median.tsv",
        "throughput_extended": "throughput-extended.tsv",
        "throughput_extended_median": "throughput-extended-median.tsv",
        "scanner_bench": "scanner-bench.tsv",
        "chart_common_three_way": "charts/common-three-way.svg",
        "chart_connect_three_way": "charts/connect-three-way.svg",
        "chart_extended_parser_state": "charts/extended-parser-state.svg",
        "chart_extended_semantics_body": "charts/extended-semantics-body.svg",
        "chart_extended_consumer_iohttp": "charts/extended-consumer-iohttp.svg",
        "chart_extended_upgrade_ioguard": "charts/extended-upgrade-ioguard.svg",
        "chart_scanner_backends": "charts/scanner-backends.svg",
        "summary": "summary.md",
        "summary_extended": "summary-extended.md",
    }
    (out_dir / "manifest.json").write_text(
        json.dumps(
            {
                "schema_version": 1,
                "run_id": run_id,
                "metadata": run_meta,
                "files": files,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    write_summary(out_dir, run_id, run_meta)
    (base_dir / "latest.txt").write_text(f"{run_id}\n", encoding="utf-8")
    with (base_dir / "index.tsv").open("a", encoding="utf-8") as stream:
        stream.write(f"{run_id}\t{run_meta['date_utc']}\t{head_short}\tPASS\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
