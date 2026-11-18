#!/usr/bin/env python3
"""Run every benchmark family and write benchmark_results/README.md.

The page embeds one vertical chart per family. The directory is gitignored.
CI force-adds it and commits it after a push.
"""

import benchmark_chart

RESULTS = benchmark_chart.ROOT / "benchmark_results"

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


def report_markdown(payload: dict) -> str:
    context = payload.get("context", {})
    host = context.get("host_name", "unknown host")
    when = context.get("date", "unknown date")
    scaling = context.get("cpu_scaling_enabled")
    scaling_note = (
        "CPU frequency scaling was enabled, so small differences can move."
        if scaling
        else "CPU frequency scaling was disabled."
    )
    lines = [
        "# Benchmark results",
        "",
        f"Measured on `{host}` at {when} with the `gcc16-benchmarks` preset. {scaling_note}",
        "",
        "Each bar is the CPU time of the named operation. A cycle bar includes construction, the calls, and destruction. One pass walks 12288 objects.",
        "",
    ]
    for family, heading, _title, _subtitle, caption in FAMILIES:
        lines.extend(
            [
                f"## {heading}",
                "",
                caption,
                "",
                f"![{heading}]({family}.svg)",
                "",
            ]
        )
    return "\n".join(lines)


def main() -> None:
    benchmark_chart.ensure_binary()
    payload = benchmark_chart.run_benchmarks(benchmark_chart.BUILD / "all.json")
    RESULTS.mkdir(parents=True, exist_ok=True)
    for family, _heading, title, subtitle, _caption in FAMILIES:
        ylabel = "nanoseconds per cycle" if family.startswith("cycle") else "nanoseconds"
        chart = benchmark_chart.chart_svg(
            benchmark_chart.rows_for(payload, family),
            title=title,
            subtitle=subtitle,
            ylabel=ylabel,
            aria=title,
        )
        (RESULTS / f"{family}.svg").write_text(chart, encoding="utf-8")
    page = RESULTS / "README.md"
    page.write_text(report_markdown(payload), encoding="utf-8")
    print(page)


if __name__ == "__main__":
    main()
