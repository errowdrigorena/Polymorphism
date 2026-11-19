#!/usr/bin/env python3
"""Shared runner and vertical bar chart for the polymorphism benchmarks."""

import html
import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build-benchmarks"
BINARY = BUILD / "polymorphism_benchmarks"


def run(command: list[str]) -> None:
    completed = subprocess.run(command, cwd=ROOT)
    if completed.returncode != 0:
        sys.exit(completed.returncode)


def ensure_binary(
    preset: str = "gcc16-benchmarks",
    build: Path = BUILD,
) -> Path:
    binary = build / "polymorphism_benchmarks"
    if not binary.is_file():
        run(["cmake", "--preset", preset])
    run(
        [
            "cmake",
            "--build",
            "--preset",
            preset,
            "--target",
            "polymorphism_benchmarks",
        ]
    )
    if not binary.is_file():
        sys.exit(f"missing {binary}")
    return binary


def run_benchmarks(
    results: Path,
    bench_filter: str | None = None,
    binary: Path = BINARY,
) -> dict:
    command = [
        str(binary),
        f"--benchmark_out={results}",
        "--benchmark_out_format=json",
    ]
    if bench_filter is not None:
        command.append(f"--benchmark_filter={bench_filter}")
    run(command)
    return json.loads(results.read_text(encoding="utf-8"))


def nanoseconds(bench: dict) -> float | None:
    if "ns_per_cycle" in bench:
        return float(bench["ns_per_cycle"])
    per_second = bench.get("items_per_second")
    if per_second in (None, 0):
        return None
    return 1e9 / float(per_second)


def rows_for(payload: dict, family: str) -> list[tuple[str, float]]:
    samples: dict[str, list[float]] = {}
    order: list[str] = []
    prefix = f"{family}/"
    for bench in payload.get("benchmarks", []):
        if bench.get("run_type") not in (None, "iteration"):
            continue
        name = str(bench.get("name", ""))
        if not name.startswith(prefix):
            continue
        value = nanoseconds(bench)
        if value is None:
            continue
        label = name.removeprefix(prefix)
        if label not in samples:
            order.append(label)
            samples[label] = []
        samples[label].append(value)
    if not order:
        sys.exit(f"no {family} results")
    return [(label, sum(samples[label]) / len(samples[label])) for label in order]


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


def number_label(number: float) -> str:
    return f"{number:.2f}"


def tick_label(number: float) -> str:
    if number == int(number):
        return str(int(number))
    return number_label(number)


def percent_label(number: float) -> str:
    rounded = round(number, 1)
    if rounded == int(rounded):
        return f"{int(rounded)}%"
    return f"{rounded:.1f}%"


def relative_percent(rows: list[tuple[str, float]]) -> list[tuple[str, float]]:
    """Scale each technique to the fastest one, leaving direct out.

    The fastest of the remaining techniques is 100%. A technique that takes
    twice as long is 200%.
    """
    kept = [(name, ns) for name, ns in rows if name != "direct"]
    if not kept:
        sys.exit("no results after leaving out direct")
    fastest = min(ns for _, ns in kept)
    if fastest <= 0:
        sys.exit("the fastest time is zero")
    return [(name, ns / fastest * 100.0) for name, ns in kept]


def chart_svg(
    rows: list[tuple[str, float]],
    *,
    title: str,
    subtitle: str,
    ylabel: str,
    aria: str,
    format_value=number_label,
    format_tick=tick_label,
    bar_fill: str = "#d97706",
) -> str:
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
            f'viewBox="0 0 {width} {height}" role="img" aria-label="{html.escape(aria)}">'
        ),
        '<rect width="100%" height="100%" fill="#ffffff"/>',
        (
            '<text x="64" y="28" font-family="sans-serif" font-size="16" fill="#1a1a1a">'
            f"{html.escape(title)}</text>"
        ),
        (
            '<text x="64" y="46" font-family="sans-serif" font-size="12" fill="#5c5c5c">'
            f"{html.escape(subtitle)}</text>"
        ),
    ]
    for tick in ticks:
        yy = y_of(tick)
        label = format_tick(tick)
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
        f'font-family="sans-serif" font-size="12" fill="#1a1a1a">{html.escape(ylabel)}</text>'
    )
    base = y_of(0)
    for index, (name, ns) in enumerate(rows):
        cx = margin["left"] + band * index + band / 2
        top = y_of(ns)
        safe_name = html.escape(name)
        parts.append(
            f'<rect x="{cx - bar_w / 2:.1f}" y="{top:.1f}" width="{bar_w:.1f}" '
            f'height="{base - top:.1f}" fill="{bar_fill}"/>'
        )
        parts.append(
            f'<text x="{cx:.1f}" y="{top - 6:.1f}" text-anchor="middle" '
            f'font-family="sans-serif" font-size="11" fill="#1a1a1a">{format_value(ns)}</text>'
        )
        parts.append(
            f'<text x="{cx:.1f}" y="{base + 16:.1f}" text-anchor="end" '
            f'transform="rotate(-40 {cx:.1f} {base + 16:.1f})" '
            f'font-family="sans-serif" font-size="11" fill="#1a1a1a">{safe_name}</text>'
        )
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def relative_chart_svg(rows: list[tuple[str, float]], *, title: str, aria: str) -> str:
    return chart_svg(
        relative_percent(rows),
        title=title,
        subtitle="100% is the fastest technique. direct is left out.",
        ylabel="percent of the fastest",
        aria=aria,
        format_value=percent_label,
        format_tick=percent_label,
        bar_fill="#1d4ed8",
    )
