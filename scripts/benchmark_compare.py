#!/usr/bin/env python3
"""Draw each benchmark family once, colored by optimization level.

Reads the JSON written for -O0, -O2, and -O3. The runs have to report the
same machine. The bars are the relative chart: direct is left out, and at
each level the fastest of the other four is 100%.

    python3 scripts/benchmark_compare.py
"""

import html
import json
import sys
from pathlib import Path

import benchmark_chart
import benchmark_report

RESULTS = benchmark_report.RESULTS
COMPARE = RESULTS / "compare"
COLORS = {
    "O0": "#b91c1c",
    "O2": "#d97706",
    "O3": "#1d4ed8",
}


def load_payload(level: str) -> dict:
    published = RESULTS / level / "results.json"
    if published.is_file():
        return json.loads(published.read_text(encoding="utf-8"))
    _preset, build = benchmark_report.LEVELS[level]
    source = build / "all.json"
    if not source.is_file():
        sys.exit(f"missing results for -{level}: {published} and {source}")
    return json.loads(source.read_text(encoding="utf-8"))


def recorded_cpu(level: str) -> str | None:
    page = RESULTS / level / "README.md"
    if not page.is_file():
        return None
    for line in page.read_text(encoding="utf-8").splitlines():
        if line.startswith("| CPU |"):
            return line.split("|")[2].strip()
    return None


def require_one_machine(payloads: dict[str, dict]) -> dict:
    identity = {}
    for level, payload in payloads.items():
        context = payload.get("context", {})
        identity[level] = (
            context.get("host_name"),
            context.get("num_cpus"),
            context.get("caches"),
        )
    signatures = {level: json.dumps(value, sort_keys=True) for level, value in identity.items()}
    if len(set(signatures.values())) != 1:
        lines = [
            f"-{level}: {identity[level][0]}, {identity[level][1]} threads" for level in payloads
        ]
        sys.exit("these runs are not one machine:\n" + "\n".join(lines))
    cpus = {level: recorded_cpu(level) for level in payloads}
    named = {name for name in cpus.values() if name}
    if len(named) > 1:
        sys.exit("the result pages name different CPUs: " + ", ".join(sorted(named)))
    return payloads[next(iter(payloads))].get("context", {})


def grouped_svg(
    groups: list[tuple[str, dict[str, float]]],
    levels: list[str],
    *,
    title: str,
    subtitle: str,
    ylabel: str,
    aria: str,
) -> str:
    width, height = 920, 520
    margin = {"top": 86, "right": 24, "bottom": 148, "left": 72}
    plot_w = width - margin["left"] - margin["right"]
    plot_h = height - margin["top"] - margin["bottom"]
    highest = max(values[level] for _name, values in groups for level in levels)
    ticks = benchmark_chart.axis_ticks(highest)
    step = ticks[1] - ticks[0] if len(ticks) > 1 else 1
    peak = max(highest * 1.22, ticks[-1] + step)
    band = plot_w / len(groups)
    gap = 4
    bar_w = min(26, (band * 0.72 - gap * (len(levels) - 1)) / len(levels))
    span = len(levels) * bar_w + (len(levels) - 1) * gap

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
            '<text x="72" y="28" font-family="sans-serif" font-size="16" fill="#1a1a1a">'
            f"{html.escape(title)}</text>"
        ),
        (
            '<text x="72" y="46" font-family="sans-serif" font-size="12" fill="#5c5c5c">'
            f"{html.escape(subtitle)}</text>"
        ),
    ]
    legend_x = 72
    for level in levels:
        parts.append(
            f'<rect x="{legend_x}" y="56" width="12" height="12" fill="{COLORS[level]}"/>'
        )
        parts.append(
            f'<text x="{legend_x + 16}" y="66" font-family="sans-serif" font-size="12" '
            f'fill="#1a1a1a">-{html.escape(level)}</text>'
        )
        legend_x += 72
    for tick in ticks:
        yy = y_of(tick)
        parts.append(
            f'<line x1="{margin["left"]}" y1="{yy:.1f}" x2="{width - margin["right"]}" '
            f'y2="{yy:.1f}" stroke="#e4e4e4"/>'
        )
        parts.append(
            f'<text x="{margin["left"] - 8}" y="{yy + 4:.1f}" text-anchor="end" '
            f'font-family="sans-serif" font-size="11" fill="#5c5c5c">'
            f"{benchmark_chart.percent_label(tick)}</text>"
        )
    label_y = margin["top"] + plot_h / 2
    parts.append(
        f'<text x="18" y="{label_y:.0f}" transform="rotate(-90 18 {label_y:.0f})" '
        f'font-family="sans-serif" font-size="12" fill="#1a1a1a">{html.escape(ylabel)}</text>'
    )
    base = y_of(0)
    for index, (name, values) in enumerate(groups):
        origin = margin["left"] + band * index + (band - span) / 2
        for series, level in enumerate(levels):
            ns = values[level]
            x = origin + series * (bar_w + gap)
            top = y_of(ns)
            parts.append(
                f'<rect x="{x:.1f}" y="{top:.1f}" width="{bar_w:.1f}" height="{base - top:.1f}" '
                f'fill="{COLORS[level]}"/>'
            )
            parts.append(
                f'<text x="{x + bar_w / 2:.1f}" y="{top - 4:.1f}" text-anchor="middle" '
                f'font-family="sans-serif" font-size="10" fill="#1a1a1a">'
                f"{benchmark_chart.percent_label(ns)}</text>"
            )
        cx = margin["left"] + band * index + band / 2
        parts.append(
            f'<text x="{cx:.1f}" y="{base + 18:.1f}" text-anchor="end" '
            f'transform="rotate(-40 {cx:.1f} {base + 18:.1f})" '
            f'font-family="sans-serif" font-size="11" fill="#1a1a1a">{html.escape(name)}</text>'
        )
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def groups_for(payloads: dict[str, dict], family: str, levels: list[str]) -> list[tuple[str, dict[str, float]]]:
    rows = {
        level: benchmark_chart.relative_percent(benchmark_chart.rows_for(payloads[level], family))
        for level in levels
    }
    names = [name for name, _percent in rows[levels[0]]]
    for level in levels[1:]:
        if [name for name, _percent in rows[level]] != names:
            sys.exit(f"{family} techniques differ at -{level}")
    grouped = []
    for name in names:
        grouped.append((name, {level: dict(rows[level])[name] for level in levels}))
    return grouped


def markdown_table(groups: list[tuple[str, dict[str, float]]], levels: list[str]) -> list[str]:
    header = "| Technique | " + " | ".join(f"-{level}" for level in levels) + " |"
    rule = "| --- | " + " | ".join("---" for _level in levels) + " |"
    lines = [header, rule]
    for name, values in groups:
        cells = " | ".join(benchmark_chart.percent_label(values[level]) for level in levels)
        lines.append(f"| {name} | {cells} |")
    return lines


def write_comparison(*, strict: bool = True) -> None:
    missing = [level for level in benchmark_report.LEVELS if not (RESULTS / level / "README.md").is_file()]
    payloads: dict[str, dict] = {}
    for level in benchmark_report.LEVELS:
        try:
            payloads[level] = load_payload(level)
        except SystemExit:
            if strict:
                raise
            return
    if missing and strict:
        sys.exit("missing result pages for " + ", ".join(f"-{level}" for level in missing))
    if missing:
        return
    context = require_one_machine(payloads)
    levels = list(benchmark_report.LEVELS)
    cpus = [recorded_cpu(level) for level in levels]
    cpu = next((name for name in cpus if name), "not recorded on the result pages")
    scaling = context.get("cpu_scaling_enabled")
    scaling_text = (
        "enabled, so small differences can move" if scaling else "disabled"
    )
    dates = [str(payloads[level].get("context", {}).get("date", "unknown")) for level in levels]
    lines = [
        "# Compared by optimization level",
        "",
        "The three runs are one machine. The host, the thread count, and the caches in the benchmark JSON match, and each result page names the same CPU.",
        "",
        "| | |",
        "| --- | --- |",
        f"| CPU | {cpu} |",
        f"| Hardware threads | {context.get('num_cpus', 'unknown')} |",
        f"| Frequency the benchmark library reported | {context.get('mhz_per_cpu', 'unknown')} MHz |",
        f"| Caches | {benchmark_report.format_caches(context.get('caches', []))} |",
        f"| Frequency scaling | {scaling_text} |",
        f"| When | {dates[0]} through {dates[-1]} |",
        "",
        "Each group is one technique. The color is the optimization level: -O0 red, -O2 amber, -O3 blue. direct is left out. At each level, 100% is the fastest of the other four, and the rest are that time scaled up.",
        "",
    ]
    COMPARE.mkdir(parents=True, exist_ok=True)
    for family, *_rest in benchmark_report.FAMILIES:
        stale = COMPARE / f"{family}_o2_o3.svg"
        if stale.is_file():
            stale.unlink()
    for family, heading, title, _subtitle, caption in benchmark_report.FAMILIES:
        grouped = groups_for(payloads, family, levels)
        chart = grouped_svg(
            grouped,
            levels,
            title=title,
            subtitle="100% is the fastest of the four at that level. direct is left out.",
            ylabel="percent of the fastest",
            aria=f"{title}, percent of the fastest at -O0, -O2, and -O3",
        )
        (COMPARE / f"{family}.svg").write_text(chart, encoding="utf-8")
        lines.extend(
            [
                f"## {heading}",
                "",
                caption,
                "",
                f"![{heading}]({family}.svg)",
                "",
                *markdown_table(grouped, levels),
                "",
            ]
        )
    page = COMPARE / "README.md"
    page.write_text("\n".join(lines), encoding="utf-8")
    print(page)


def main() -> None:
    write_comparison(strict=True)


if __name__ == "__main__":
    main()
