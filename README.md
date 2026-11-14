# Polymorphism

[![coverage](badges/coverage.svg)](#coverage)
[![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![GCC 16](https://img.shields.io/badge/GCC-16-00599C?logo=gnu&logoColor=white)](#requirements)
[![CMake 3.28+](https://img.shields.io/badge/CMake-3.28%2B-064F8C?logo=cmake&logoColor=white)](https://cmake.org/cmake/help/latest/)

Two small C++26 libraries that print a shape, built as named modules. `shapes` is a classic abstract base with three concrete types. `type_erasure` is a value that stores any object with a `draw()` method and calls it through a hand-written vtable.

`src/main.cpp` walks the classic hierarchy. The type-erased value is exercised by the tests.

## Contents

- [Layout](#layout)
- [Requirements](#requirements)
- [Build and run](#build-and-run)
- [Sanitizers](#sanitizers)
- [Coverage](#coverage)
- [Local checks](#local-checks)
- [Continuous integration](#continuous-integration)

## Layout

| Path | Role |
| --- | --- |
| `src/Traditional_Polymorphism/` | Module `shapes` |
| `src/TypeErasure_Polymorphism/` | Module `type_erasure` |
| `src/main.cpp` | Demo executable `draw_shapes` |
| `tests/` | GoogleTest drivers |

### `shapes`

Partitions `:shape`, `:circle`, `:triangle`, and `:rectangle`, re-exported by the primary interface `shapes`. `Shape` declares `virtual std::string draw() const`. `Circle`, `Triangle`, and `Rectangle` are `final` and define `draw()` in a module implementation unit (`module shapes;`), so the function body stays out of the module interface.

### `type_erasure`

Partitions `:drawable` and `:shape`, re-exported by `type_erasure`. `DrawableShape_TE` accepts a type whose `draw() const` returns something convertible to `std::string`. `Shape_TE` owns that object and dispatches `draw`, copy, and destroy through a private vtable. The constructor template and `vtable_for` stay in the interface unit, because callers instantiate them. An empty `Shape_TE` reports `has_value() == false`, and `draw()` returns `"<empty>"`.

## Requirements

- GCC 16. The presets call `g++-16`, and the build uses C++26 named modules.
- CMake 3.28 or newer, and Ninja.
- GoogleTest, for the test targets. `find_package(GTest)` skips them when the package is absent. CI installs the Ubuntu packages `libgtest-dev` and `libgmock-dev`. The tests use Google Mock matchers.

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

The `gcc16-coverage` preset instruments the same tests with gcov. The `coverage` target reruns them, prints a gcovr summary for `src/`, and rewrites `badges/coverage.svg` with the rounded line percentage. `main.cpp` stays out of that summary, because the tests never run the demo. On a push, CI commits the SVG when that integer changes. gcov mis-counts inline functions and templates inside module interface units, so the pass/fail result stays with the tests. The badge color follows the percentage.

`gcov-16` ships with GCC 16. The summary also needs `gcovr`.

```bash
cmake --preset gcc16-coverage
cmake --build --preset gcc16-coverage
cmake --build build-coverage --target coverage
```

Coverage and sanitizers are separate configures. Turning both options on in one build directory stops at configure time.

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

`.github/workflows/ci.yml` runs on pushes to `develop` and `master`, and on pull requests. The job uses an Ubuntu resolute container, then the `gcc16`, `gcc16-sanitizers`, and `gcc16-coverage` presets, and finishes with `format-check`.
