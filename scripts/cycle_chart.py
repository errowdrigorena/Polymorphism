#!/usr/bin/env python3
"""Run the cycle benchmarks and write a vertical bar chart.

The chart is one bar per approach: nanoseconds for construct, draw, area, and
destroy. Outputs land in the Release build directory:

    build-benchmarks/cycle.json
    build-benchmarks/cycle.svg
"""

import html
import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build-benchmarks"
BINARY = BUILD / "polymorphism_benchmarks"
RESULTS = BUILD / "cycle.json"
CHART = BUILD / "cycle.svg"


def run(command: list[str]) -> None:
    completed = subprocess.run(command, cwd=ROOT)
    if completed.returncode != 0:
        sys.exit(completed.returncode)


def ensure_binary() -> None:
    if not BINARY.is_file():
        run(["cmake", "--preset", "gcc16-benchmarks"])
    run(
        [
            "cmake",
            "--build",
            "--preset",
            "gcc16-benchmarks",
            "--target",
            "polymorphism_benchmarks",
        ]
    )
    if not BINARY.is_file():
        sys.exit(f"missing {BINARY}")


def run_benchmarks() -> None:
    run(
        [
            str(BINARY),
            "--benchmark_filter=^cycle/",
            f"--benchmark_out={RESULTS}",
            "--benchmark_out_format=json",
        ]
    )


def cycle_rows(payload: dict) -> list[tuple[str, float]]:
    samples: dict[str, list[float]] = {}
    order: list[str] = []
    for bench in payload.get("benchmarks", []):
        if bench.get("run_type") not in (None, "iteration"):
            continue
        if "ns_per_cycle" not in bench:
            continue
        name = str(bench["name"]).removeprefix("cycle/")
        if name not in samples:
            order.append(name)
            samples[name] = []
        samples[name].append(float(bench["ns_per_cycle"]))
    if not order:
        sys.exit(f"{RESULTS} has no ns_per_cycle results")
    return [(name, sum(samples[name]) / len(samples[name])) for name in order]


def axis_ticks(highest: float) -> list[float]:
    if highest <= 0:
        return [0.0]
    rough = highest / 3
    magnitude = 10 ** math.floor(math.log10(rough))
    step = 10 * magnitude
    for factor in (1, 2, 5, 10):
        if factor * magnitude >= rough:
            step = factor * magnitude
            break
    ticks: list[float] = []
    value = 0.0
    while True:
        ticks.append(value)
        if value >= highest:
            break
        value += step
    return ticks


def comma(number: float) -> str:
    return f"{number:.2f}".replace(".", ",")


def chart_svg(rows: list[tuple[str, float]]) -> str:
    width, height = 760, 480
    margin = {"top": 56, "right": 24, "bottom": 148, "left": 64}
    plot_w = width - margin["left"] - margin["right"]
    plot_h = height - margin["top"] - margin["bottom"]
    highest = max(ns for _, ns in rows)
    ticks = axis_ticks(highest)
    peak = max(highest * 1.18, ticks[-1] + (ticks[1] - ticks[0] if len(ticks) > 1 else 1))
    band = plot_w / len(rows)
    bar_w = min(32, band * 0.45)

    def y_of(ns: float) -> float:
        return margin["top"] + plot_h - (ns / peak) * plot_h

    parts = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        (
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
            f'viewBox="0 0 {width} {height}" role="img" '
            'aria-label="Tiempo de un ciclo por enfoque">'
        ),
        '<rect width="100%" height="100%" fill="#ffffff"/>',
        (
            '<text x="64" y="28" font-family="sans-serif" font-size="16" fill="#1a1a1a">'
            "Tiempo de un ciclo</text>"
        ),
        (
            '<text x="64" y="46" font-family="sans-serif" font-size="12" fill="#5c5c5c">'
            "ns de CPU · construir, dibujar, área y destruir</text>"
        ),
    ]
    for tick in ticks:
        yy = y_of(tick)
        label = str(int(tick)) if tick == int(tick) else comma(tick)
        parts.append(
            f'<line x1="{margin["left"]}" y1="{yy:.1f}" x2="{width - margin["right"]}" '
            f'y2="{yy:.1f}" stroke="#e4e4e4"/>'
        )
        parts.append(
            f'<text x="{margin["left"] - 8}" y="{yy + 4:.1f}" text-anchor="end" '
            f'font-family="sans-serif" font-size="11" fill="#5c5c5c">{label}</text>'
        )
    label_y = margin["top"] + plot_h / 2
    parts.append(
        f'<text x="16" y="{label_y:.0f}" transform="rotate(-90 16 {label_y:.0f})" '
        'font-family="sans-serif" font-size="12" fill="#1a1a1a">nanosegundos por ciclo</text>'
    )
    base = y_of(0)
    for index, (name, ns) in enumerate(rows):
        cx = margin["left"] + band * index + band / 2
        top = y_of(ns)
        safe_name = html.escape(name)
        parts.append(
            f'<rect x="{cx - bar_w / 2:.1f}" y="{top:.1f}" width="{bar_w:.1f}" '
            f'height="{base - top:.1f}" fill="#d97706"/>'
        )
        parts.append(
            f'<text x="{cx:.1f}" y="{top - 6:.1f}" text-anchor="middle" '
            f'font-family="sans-serif" font-size="11" fill="#1a1a1a">{comma(ns)}</text>'
        )
        parts.append(
            f'<text x="{cx:.1f}" y="{base + 16:.1f}" text-anchor="end" '
            f'transform="rotate(-40 {cx:.1f} {base + 16:.1f})" '
            f'font-family="sans-serif" font-size="11" fill="#1a1a1a">{safe_name}</text>'
        )
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def main() -> None:
    ensure_binary()
    run_benchmarks()
    rows = cycle_rows(json.loads(RESULTS.read_text(encoding="utf-8")))
    CHART.write_text(chart_svg(rows), encoding="utf-8")
    print(CHART)


if __name__ == "__main__":
    main()
