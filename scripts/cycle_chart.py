#!/usr/bin/env python3
"""Run the cycle benchmarks and write a vertical bar chart.

The chart is one bar per approach: nanoseconds for construct, draw, area, and
destroy. Outputs land in the Release build directory:

    build-benchmarks/cycle.json
    build-benchmarks/cycle.svg
"""

import benchmark_chart

RESULTS = benchmark_chart.BUILD / "cycle.json"
CHART = benchmark_chart.BUILD / "cycle.svg"


def chart_svg(rows: list[tuple[str, float]]) -> str:
    return benchmark_chart.chart_svg(
        rows,
        title="Time for one cycle",
        subtitle="CPU ns · construct, draw, area, and destroy",
        ylabel="nanoseconds per cycle",
        aria="Time for one cycle by approach",
    )


def main() -> None:
    benchmark_chart.ensure_binary()
    payload = benchmark_chart.run_benchmarks(RESULTS, "^cycle/")
    rows = benchmark_chart.rows_for(payload, "cycle")
    CHART.write_text(chart_svg(rows), encoding="utf-8")
    print(CHART)


if __name__ == "__main__":
    main()
