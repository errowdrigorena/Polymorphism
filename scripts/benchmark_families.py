#!/usr/bin/env python3
"""Run draw, area, copy, and build, and write one vertical chart each.

Charts land in the Release build directory:

    build-benchmarks/draw.svg
    build-benchmarks/area.svg
    build-benchmarks/copy.svg
    build-benchmarks/build.svg
"""

from pathlib import Path

import benchmark_chart

FAMILIES = (
    ("draw", "Time for one draw()", "CPU ns · one call that returns std::string"),
    ("area", "Time for one area()", "CPU ns · one call that returns float"),
    ("copy", "Time for one copy", "CPU ns · copy one owning object"),
    ("build", "Time for one construction", "CPU ns · construct one object"),
)


def write_family(payload: dict, family: str, title: str, subtitle: str, directory: Path) -> Path:
    chart = directory / f"{family}.svg"
    chart.write_text(
        benchmark_chart.chart_svg(
            benchmark_chart.rows_for(payload, family),
            title=title,
            subtitle=subtitle,
            ylabel="nanoseconds",
            aria=title,
        ),
        encoding="utf-8",
    )
    return chart


def main() -> None:
    benchmark_chart.ensure_binary()
    payload = benchmark_chart.run_benchmarks(
        benchmark_chart.BUILD / "families.json",
        "^(draw|area|copy|build)/",
    )
    for family, title, subtitle in FAMILIES:
        print(write_family(payload, family, title, subtitle, benchmark_chart.BUILD))


if __name__ == "__main__":
    main()
