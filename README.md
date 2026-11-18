# Polymorphism

[![coverage](badges/coverage.svg)](#coverage)
[![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![GCC 16](https://img.shields.io/badge/GCC-16-00599C?logo=gnu&logoColor=white)](#requirements)
[![CMake 3.28+](https://img.shields.io/badge/CMake-3.28%2B-064F8C?logo=cmake&logoColor=white)](https://cmake.org/cmake/help/latest/)

Four small C++26 libraries that put shapes behind one interface, built as named modules:

- `shapes` is a classic abstract base with three concrete types.
- `type_erasure` is a value that stores any object with a `draw()` method and calls it through a hand-written vtable.
- `polymorphic` gives the `shapes` hierarchy value semantics with `std::polymorphic`.
- `reflection_type_erasure` uses C++26 reflection to generate the vtable and the call surface from a model class.

`src/main.cpp` walks the classic hierarchy. `polymorphic` and `reflection_type_erasure` have their own demos. The type-erased value is exercised by the tests.

## Contents

- [Layout](#layout)
- [Requirements](#requirements)
- [Build and run](#build-and-run)
- [Sanitizers](#sanitizers)
- [Coverage](#coverage)
- [Benchmarks](#benchmarks)
- [Local checks](#local-checks)
- [Continuous integration](#continuous-integration)

## Layout

| Path | Role |
| --- | --- |
| `src/Traditional_Polymorphism/` | Module `shapes` |
| `src/TypeErasure_Polymorphism/` | Module `type_erasure` |
| `src/Polymorphic_Polymorphism/` | Module `polymorphic` and demo `draw_shapes_polymorphic` |
| `src/Reflection_TypeErasure_Polymorphism/` | Module `reflection_type_erasure` and demo `draw_shapes_reflection` |
| `src/main.cpp` | Demo executable `draw_shapes` |
| `tests/` | GoogleTest drivers |
| `benchmarks/` | Release timings for dispatch, copy, and construction |

### `shapes`

Partitions `:shape`, `:circle`, `:triangle`, and `:rectangle`, re-exported by the primary interface `shapes`. `Shape` declares `virtual std::string draw() const`. `Circle`, `Triangle`, and `Rectangle` are `final` and define `draw()` in a module implementation unit (`module shapes;`), so the function body stays out of the module interface.

### `type_erasure`

Partitions `:drawable` and `:shape`, re-exported by `type_erasure`. `DrawableShape_TE` accepts a type whose `draw() const` returns something convertible to `std::string`. `Shape_TE` owns that object and dispatches `draw`, copy, and destroy through a private vtable. The constructor template and `vtable_for` stay in the interface unit, because callers instantiate them. An empty `Shape_TE` reports `has_value() == false`, and `draw()` returns `"<empty>"`.

### `polymorphic`

Re-exports `shapes` and adds `PolyShape`, an alias for `std::polymorphic<Shape>`. A `PolyShape` owns a `Circle`, `Triangle`, `Rectangle`, or any other class derived from `Shape`. Copying it copies the derived object, so a `std::vector<PolyShape>` behaves like a vector of values. After a move, the source reports `valueless_after_move()`. Any type stored this way must derive from `Shape`. `type_erasure` and `reflection_type_erasure` accept unrelated types.

### `reflection_type_erasure`

`ShapeModel` is the interface every stored type provides: pure virtual `draw()`, `area()`, and `perimeter()`. A method that is not pure virtual stops the build. A namespace-scope `consteval` block reads that class with `std::meta` and uses `define_aggregate` to generate two structs. `VTable<Model>` holds one function pointer per method plus copy and destroy. `Interface<Model>` holds one callable data member per method. `Poly<Model>` owns the object the same way `Shape_TE` does and inherits `Interface<Model>`, so `AnyShape` (`Poly<ShapeModel>`) is called as `shape.area()`. Any type whose public methods match the model, including each parameter type, is accepted. A method may take as many parameters as it declares. Another interface calls `define_model<ThatInterface>()` from its own consteval block.

The library needs `-freflection`. The build compiles GCC's `import std;` module with that flag, so `std::meta` is available to every importer.

## Requirements

- GCC 16. The presets call `g++-16`, and the build uses C++26 named modules.
- CMake 3.28 or newer, and Ninja.
- GoogleTest, for the test targets. `find_package(GTest)` skips them when the package is absent. CI installs the Ubuntu packages `libgtest-dev` and `libgmock-dev`. The tests use Google Mock matchers.
- Google Benchmark, for `polymorphism_benchmarks`. `find_package(benchmark)` skips that target when the package is absent. CI installs the Ubuntu package `libbenchmark-dev`.

## Build and run

```bash
cmake --preset gcc16
cmake --build --preset gcc16
./build/src/draw_shapes
```

```text
Drawing Circle
Drawing Triangle
Drawing Rectangle
```

`./build/src/draw_shapes_polymorphic` prints the same three lines and then a copy of the first shape. `./build/src/draw_shapes_reflection` prints each shape with its area and perimeter.

GoogleTest's headers and `import std;` cannot share a translation unit with GCC 16. Each test target therefore has two files: a `*_cases.cpp` that imports the modules and runs each case, and a `test_*.cpp` that includes GoogleTest and checks the results.

```bash
ctest --test-dir build --output-on-failure
```

CI runs CTest with `--no-tests=error`, so a configure that skipped GoogleTest fails the job.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer use their own build directory.

```bash
cmake --preset gcc16-sanitizers
cmake --build --preset gcc16-sanitizers
ctest --test-dir build-sanitizers --output-on-failure
```

## Coverage

The `gcc16-coverage` preset instruments the same tests with gcov. The `coverage` target reruns them, prints a gcovr summary for `src/`, and rewrites `badges/coverage.svg` with the rounded line percentage. Every demo `main.cpp` stays out of that summary, because the tests never run the demos. On a push, CI commits the SVG when that integer changes. gcov mis-counts inline functions and templates inside module interface units, so the pass/fail result stays with the tests. The badge color follows the percentage.

`gcov-16` ships with GCC 16. The summary also needs `gcovr`.

```bash
cmake --preset gcc16-coverage
cmake --build --preset gcc16-coverage
cmake --build build-coverage --target coverage
```

Coverage and sanitizers are separate configures. Turning both options on in one build directory stops at configure time.

## Benchmarks

`benchmarks/` builds `polymorphism_benchmarks` with Google Benchmark. It is not a test. The `gcc16` preset compiles it when the package is present. CI builds that target in the Debug preset and does not read those timings. Timings come from the Release preset. A Debug run says so and is not worth reading.

Google Benchmark's headers and `import std;` cannot share a translation unit with this GCC, the same limit as GoogleTest. `benchmark_work` is a module whose interface does not import `std`. `timer.cpp` includes the library and imports that interface. `benchmark_work.cpp` is the implementation unit and imports `std` and the shape modules.

```bash
cmake --preset gcc16-benchmarks
cmake --build --preset gcc16-benchmarks
./build-benchmarks/polymorphism_benchmarks --benchmark_counters_tabular=true
```

`--benchmark_filter=area` selects one family. Each iteration walks 12288 objects. `items_per_second` counts one `draw()`, `area()`, copy, or build. The `bytes` counter is `sizeof` the handle, or the object itself for `direct`. Ubuntu's `libbenchmark-dev` is built without `NDEBUG`, so the run prints that the benchmark library is Debug. The code under test still follows the preset.

Every family times the same five techniques: `direct`, `traditional_polymorphism`, `type_erasure`, `std::polymorphic`, and `reflection_type_erasure`. `direct` stores one concrete type in a contiguous array. The other benchmarks alternate three types.

`draw()` returns `std::string` for `traditional_polymorphism` (`std::unique_ptr<Shape>`), `type_erasure`, and `std::polymorphic`. The iteration builds that string, and the string fits in the small-string buffer. That construction is what these benchmarks measure. `type_erasure` stores the same concrete types `direct` calls. Those types are unrelated to `Shape`. The reflection library's `draw()` writes to `std::cout`, so `reflection_type_erasure` uses `Poly` of a model whose `draw()` returns the same string.

`area()` returns `float`. The `shapes` hierarchy and `Shape_TE` have no numeric method, so `traditional_polymorphism`, `type_erasure`, and `std::polymorphic` use a local base with the same formula. `reflection_type_erasure` calls `area()` on the library model.

`copy/` and `build/` time owning a vector. `direct` keeps the object inline. The other benchmarks heap-allocate each object. `traditional_polymorphism` copies through a virtual `clone()` because `std::unique_ptr` does not clone the derived object.

`cycle` is one construct, one `draw()`, one `area()`, and one destroy. `cycle_5`, `cycle_50`, `cycle_500`, and `cycle_5000` alternate `draw()` and `area()` that many times before destruction. An odd count starts and ends on `draw()`, so five calls are three draws and two areas. The even counts are split in half.

Before timing, the executable checks that the five techniques in each family produce the same result. A mismatch exits with status 1.

`scripts/benchmark_families.py` runs `draw`, `area`, `copy`, and `build`, and writes one SVG each under `build-benchmarks/`. `scripts/cycle_chart.py` does the same for `cycle`. `scripts/benchmark_report.py` runs every family once and writes `benchmark_results/README.md` with all five charts. That directory is gitignored. On a push, CI runs the script and commits the directory.

## Local checks

| Target | Tool |
| --- | --- |
| `format`, `format-check` | clang-format, style in `.clang-format` |
| `tidy` | clang-tidy, checks in `.clang-tidy` |
| `cppcheck` | cppcheck against `compile_commands.json` |

`.clang-format` follows the C++ Core Guidelines layout: Stroustrup braces, `T*` and `T&`, `const T`. `.clang-tidy` enables the Core Guidelines checks plus `bugprone-*`, `performance-*`, and `clang-analyzer-*`.

CI runs `format-check` on the GCC build. `tidy` and `cppcheck` stay available locally; that job skips them because those tools cannot read the module artifacts GCC writes.

```bash
cmake --build build --target format-check
```

## Continuous integration

`.github/workflows/ci.yml` runs on pushes to `develop` and `master`, and on pull requests. The job uses an Ubuntu resolute container, then the `gcc16`, `gcc16-sanitizers`, and `gcc16-coverage` presets, and finishes with `format-check`. A push also runs the Release benchmark preset and commits `benchmark_results/`.
