# Benchmark results

Each directory is one optimization level. Read Conditions in that directory before the charts. A bar in one directory is a different run from a bar in the other.

- [-O0](O0/README.md) from the `gcc16-benchmarks-o0` preset.
- [-O2](O2/README.md) from the `gcc16-benchmarks-o2` preset.
- [-O3](O3/README.md) from the `gcc16-benchmarks` preset.

`-O0` is no optimization. It still defines `NDEBUG`, so the three directories differ by the `-O` flag. The `gcc16` Debug preset is a different build and is not one of these directories.

When every level is present and the runs report the same machine, [compare/README.md](compare/README.md) draws each family once, with one color per optimization level.
