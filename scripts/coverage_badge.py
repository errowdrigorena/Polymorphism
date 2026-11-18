#!/usr/bin/env python3
"""Write badges/coverage.svg from a gcovr JSON summary."""

import json
import sys
from pathlib import Path


def text_width(text: str) -> int:
    return 10 + 7 * len(text)


def color_for(percent: int) -> str:
    if percent >= 90:
        return "#4c1"
    if percent >= 75:
        return "#97ca00"
    if percent >= 60:
        return "#a4a61d"
    if percent >= 40:
        return "#dfb317"
    return "#e05d44"


def badge(label: str, value: str, color: str) -> str:
    label_width = text_width(label)
    value_width = text_width(value)
    total = label_width + value_width
    label_x = label_width / 2
    value_x = label_width + value_width / 2
    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="{total}" height="20" role="img" aria-label="{label}: {value}">
  <title>{label}: {value}</title>
  <linearGradient id="smooth" x2="0" y2="100%">
    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
    <stop offset="1" stop-opacity=".1"/>
  </linearGradient>
  <clipPath id="round"><rect width="{total}" height="20" rx="3" fill="#fff"/></clipPath>
  <g clip-path="url(#round)">
    <rect width="{label_width}" height="20" fill="#555"/>
    <rect x="{label_width}" width="{value_width}" height="20" fill="{color}"/>
    <rect width="{total}" height="20" fill="url(#smooth)"/>
  </g>
  <g fill="#fff" text-anchor="middle" font-family="DejaVu Sans,Verdana,Geneva,sans-serif" font-size="11">
    <text x="{label_x}" y="14">{label}</text>
    <text x="{value_x}" y="14">{value}</text>
  </g>
</svg>
"""


def main() -> None:
    summary = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
    output = Path(sys.argv[2])
    percent = summary.get("line_percent")
    if percent is None:
        value = "n/a"
        color = "#9f9f9f"
    else:
        rounded = round(float(percent))
        value = f"{rounded}%"
        color = color_for(rounded)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(badge("coverage", value, color), encoding="utf-8")


if __name__ == "__main__":
    main()
