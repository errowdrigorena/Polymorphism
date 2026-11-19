#!/usr/bin/env python3
"""Run every benchmark family once per optimization level.

Writes benchmark_results/<level>/README.md. The first section is the machine
and the compiler flags. The charts follow. The directory is gitignored.
CI force-adds it and commits it after a push.

    python3 scripts/benchmark_report.py        # -O0, -O2, and -O3
    python3 scripts/benchmark_report.py O0     # one level
"""

import json
import shlex
import subprocess
import sys
from pathlib import Path

import benchmark_chart

RESULTS = benchmark_chart.ROOT / "benchmark_results"

# The directory name is the -O flag the compile line must contain.
LEVELS = {
    "O0": ("gcc16-benchmarks-o0", benchmark_chart.ROOT / "build-benchmarks-o0"),
    "O2": ("gcc16-benchmarks-o2", benchmark_chart.ROOT / "build-benchmarks-o2"),
    "O3": ("gcc16-benchmarks", benchmark_chart.ROOT / "build-benchmarks"),
}

FAMILIES = (
    (
        "cycle",
        "Cycle",
        "Time for one cycle",
        "CPU ns · construct, draw, area, and destroy",
        "Construct, draw, compute the area, and destroy.",
    ),
    (
        "cycle_5",
        "Cycle, 5 calls",
        "Time for one cycle, 5 calls",
        "CPU ns · construct, 3 draw() and 2 area(), destroy",
        "Construct, alternate draw() and area() five times (three draws and two areas), and destroy.",
    ),
    (
        "cycle_50",
        "Cycle, 50 calls",
        "Time for one cycle, 50 calls",
        "CPU ns · construct, 25 draw() and 25 area(), destroy",
        "Construct, alternate draw() and area() fifty times, and destroy.",
    ),
    (
        "cycle_500",
        "Cycle, 500 calls",
        "Time for one cycle, 500 calls",
        "CPU ns · construct, 250 draw() and 250 area(), destroy",
        "Construct, alternate draw() and area() five hundred times, and destroy.",
    ),
    (
        "cycle_5000",
        "Cycle, 5000 calls",
        "Time for one cycle, 5000 calls",
        "CPU ns · construct, 2500 draw() and 2500 area(), destroy",
        "Construct, alternate draw() and area() five thousand times, and destroy.",
    ),
    (
        "draw",
        "Draw",
        "Time for one draw()",
        "CPU ns · one call that returns std::string",
        "One draw() call. The string fits in the small buffer.",
    ),
    (
        "area",
        "Area",
        "Time for one area()",
        "CPU ns · one call that returns float",
        "One area() call. traditional_polymorphism, type_erasure, and std::polymorphic use a local base because the shapes hierarchy has no numeric method.",
    ),
    (
        "copy",
        "Copy",
        "Time for one copy",
        "CPU ns · copy one owning object",
        "Copy one owning object. traditional_polymorphism clones through a virtual function because unique_ptr does not copy the derived object.",
    ),
    (
        "build",
        "Build",
        "Time for one construction",
        "CPU ns · construct one object",
        "Construct one object. direct stores it in the array; the others allocate on the heap.",
    ),
)


def cpu_model() -> str:
    cpuinfo = Path("/proc/cpuinfo")
    if not cpuinfo.is_file():
        return "unknown"
    for line in cpuinfo.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.lower().startswith("model name"):
            return line.split(":", 1)[1].strip()
    return "unknown"


def compile_entry(build: Path) -> dict:
    commands = json.loads((build / "compile_commands.json").read_text(encoding="utf-8"))
    for entry in commands:
        if str(entry.get("file", "")).endswith("/benchmarks/benchmark_work.cpp"):
            return entry
    sys.exit(f"benchmark_work.cpp is missing from {build / 'compile_commands.json'}")


def codegen_flags(command: str) -> list[str]:
    """Flags that change the code under test, without the module-mapper plumbing."""
    skip_with_argument = {"-o", "-c", "-isystem", "-I", "-x"}
    tokens = shlex.split(command)
    kept: list[str] = []
    index = 1
    while index < len(tokens):
        token = tokens[index]
        if token in skip_with_argument:
            index += 2
            continue
        if (
            token in {"-MD"}
            or token.startswith("-fmodule-mapper=")
            or token.startswith("-fdeps-format=")
        ):
            index += 1
            continue
        if token.startswith("-"):
            kept.append(token)
        index += 1
    return kept


def opt_flag(flags: list[str]) -> str:
    found = [
        flag
        for flag in flags
        if flag in {"-O0", "-O1", "-O2", "-O3", "-Og", "-Os", "-Ofast", "-Oz"}
    ]
    if len(found) != 1:
        sys.exit(f"expected one optimization flag, found {found or 'none'}")
    return found[0]


def compiler_version(compiler: str) -> str:
    completed = subprocess.run(
        [compiler, "--version"],
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        sys.exit(completed.stderr or f"{compiler} --version failed")
    return completed.stdout.splitlines()[0]


def format_caches(caches: list) -> str:
    parts: list[str] = []
    for cache in caches:
        size = int(cache.get("size", 0))
        kib = size / 1024
        if kib >= 1024 and kib % 1024 == 0:
            size_text = f"{int(kib // 1024)} MiB"
        else:
            size_text = f"{kib:.0f} KiB"
        kind = str(cache.get("type", "")).lower()
        level = cache.get("level", "?")
        sharing = cache.get("num_sharing", "?")
        parts.append(f"L{level} {kind} {size_text}, shared by {sharing}")
    return "; ".join(parts) if parts else "not reported"


def conditions_markdown(level: str, payload: dict, build: Path) -> list[str]:
    context = payload.get("context", {})
    entry = compile_entry(build)
    command = str(entry["command"])
    flags = codegen_flags(command)
    found = opt_flag(flags)
    if found != f"-{level}":
        sys.exit(f"{build} compiled benchmark_work.cpp with {found}, not -{level}")
    compiler = shlex.split(command)[0]
    when = context.get("date", "unknown date")
    scaling = context.get("cpu_scaling_enabled")
    scaling_text = (
        "enabled, so small differences can move"
        if scaling
        else "disabled"
    )
    threads = context.get("num_cpus", "unknown")
    mhz = context.get("mhz_per_cpu", "unknown")
    library = context.get("library_version", "unknown")
    library_build = context.get("library_build_type", "unknown")
    lto = "present on the compile line" if "-flto" in flags else "absent"
    return [
        f"# Benchmark results, -{level}",
        "",
        "## Conditions",
        "",
        "These charts are this run. Another optimization level is another directory, and a bar there is not this bar.",
        "",
        "| | |",
        "| --- | --- |",
        f"| CPU | {cpu_model()} |",
        f"| Hardware threads | {threads} |",
        f"| Frequency the benchmark library reported | {mhz} MHz |",
        f"| Caches | {format_caches(context.get('caches', []))} |",
        f"| Frequency scaling | {scaling_text} |",
        f"| Compiler | {compiler_version(compiler)} (`{compiler}`) |",
        f"| Flags on `benchmark_work.cpp` | `{' '.join(flags)}` |",
        f"| Link-time optimization | {lto}. The benchmark target sets `INTERPROCEDURAL_OPTIMIZATION` off, so the timed bodies stay out of the translation unit that runs the loop. |",
        f"| Google Benchmark | {library}, library itself built as {library_build} |",
        f"| When | {when} |",
        "",
        "Each bar is the CPU time of the named operation. A cycle bar includes construction, the calls, and destruction. One pass walks 12288 objects.",
        "",
    ]


def report_markdown(level: str, payload: dict, build: Path) -> str:
    lines = conditions_markdown(level, payload, build)
    for family, heading, _title, _subtitle, caption in FAMILIES:
        lines.extend(
            [
                f"## {heading}",
                "",
                caption,
                "",
                f"![{heading}]({family}.svg)",
                "",
                "The chart below leaves out direct. 100% is the fastest of the four, and the others are that time scaled up.",
                "",
                f"![{heading}, relative]({family}_relative.svg)",
                "",
            ]
        )
    return "\n".join(lines)


def write_level(level: str) -> None:
    try:
        preset, build = LEVELS[level]
    except KeyError:
        sys.exit(f"unknown level {level}. Choose from {', '.join(LEVELS)}")
    binary = benchmark_chart.ensure_binary(preset, build)
    found = opt_flag(codegen_flags(str(compile_entry(build)["command"])))
    if found != f"-{level}":
        sys.exit(f"{build} compiled benchmark_work.cpp with {found}, not -{level}")
    payload = benchmark_chart.run_benchmarks(build / "all.json", binary=binary)
    destination = RESULTS / level
    destination.mkdir(parents=True, exist_ok=True)
    for family, _heading, title, subtitle, _caption in FAMILIES:
        ylabel = "nanoseconds per cycle" if family.startswith("cycle") else "nanoseconds"
        rows = benchmark_chart.rows_for(payload, family)
        chart = benchmark_chart.chart_svg(
            rows,
            title=title,
            subtitle=subtitle,
            ylabel=ylabel,
            aria=title,
        )
        (destination / f"{family}.svg").write_text(chart, encoding="utf-8")
        relative = benchmark_chart.relative_chart_svg(
            rows,
            title=f"{title}, relative",
            aria=f"{title}, relative to the fastest technique",
        )
        (destination / f"{family}_relative.svg").write_text(relative, encoding="utf-8")
    page = destination / "README.md"
    page.write_text(report_markdown(level, payload, build), encoding="utf-8")
    (destination / "results.json").write_text(json.dumps(payload), encoding="utf-8")
    print(page)


def write_index() -> None:
    lines = [
        "# Benchmark results",
        "",
        "Each directory is one optimization level. Read Conditions in that directory before the charts. A bar in one directory is a different run from a bar in the other.",
        "",
    ]
    for level, (preset, _build) in LEVELS.items():
        if (RESULTS / level / "README.md").is_file():
            lines.append(f"- [-{level}]({level}/README.md) from the `{preset}` preset.")
    lines.extend(
        [
            "",
            "`-O0` is no optimization. It still defines `NDEBUG`, so the three directories differ by the `-O` flag. The `gcc16` Debug preset is a different build and is not one of these directories.",
            "",
            "When every level is present and the runs report the same machine, [compare/README.md](compare/README.md) draws each family once, with one color per optimization level.",
            "",
        ]
    )
    RESULTS.mkdir(parents=True, exist_ok=True)
    # Earlier runs wrote the charts next to this index. A level directory replaces them.
    for family, *_rest in FAMILIES:
        for name in (f"{family}.svg", f"{family}_relative.svg"):
            stale = RESULTS / name
            if stale.is_file():
                stale.unlink()
    page = RESULTS / "README.md"
    page.write_text("\n".join(lines), encoding="utf-8")
    print(page)


def main() -> None:
    selected = sys.argv[1:] or list(LEVELS)
    for level in selected:
        write_level(level)
    write_index()
    import benchmark_compare

    benchmark_compare.write_comparison(strict=False)


if __name__ == "__main__":
    main()
