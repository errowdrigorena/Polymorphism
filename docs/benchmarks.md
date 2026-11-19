# Benchmarks

## Conditions

The numbers are split by optimization level:

- [`benchmark_results/O0/`](../benchmark_results/O0/README.md) is the `gcc16-benchmarks-o0` preset, `-O0 -DNDEBUG`. No optimization.
- [`benchmark_results/O2/`](../benchmark_results/O2/README.md) is the `gcc16-benchmarks-o2` preset, `-O2 -DNDEBUG`.
- [`benchmark_results/O3/`](../benchmark_results/O3/README.md) is the `gcc16-benchmarks` preset, `-O3 -DNDEBUG`.

`-O0` is required. Without it, a difference between the bars can be the optimizer rather than the program. `NDEBUG` stays defined at every level, so the three directories differ by the `-O` flag. The `gcc16` Debug preset is a different build and is not the `-O0` directory.

[compare/README.md](../benchmark_results/compare/README.md) draws each family once, one color per level, after checking that the three runs are the same machine. The Conditions section of a single directory names the CPU, the caches, the frequency the benchmark library reported, whether frequency scaling was on, the compiler, and the flags on `benchmark_work.cpp`. A bar in one directory is not a bar in another, and none of them is a bar from another machine.

The root [README](../README.md) explains how to build the binary and how each family is timed. The rest of this note is about coverage. The programs below are the same at every level. The times are not. The names on the bars are broader than the programs, so the numbers are not sufficient to rank type erasure, virtual dispatch, `std::polymorphic`, or reflection.

## Five labels, more than five machines

Every family reports `direct`, `traditional_polymorphism`, `type_erasure`, `std::polymorphic`, and `reflection_type_erasure`. The C++ type behind a label changes with the family. The types are fixed in `benchmarks/benchmark_work.cpp`.

| Family | `direct` | `traditional_polymorphism` | `type_erasure` | `std::polymorphic` | `reflection_type_erasure` |
| --- | --- | --- | --- | --- | --- |
| `draw` | `ValueDisk`, `ValueBox`, `ValueWedge`, each in its own vector | `std::unique_ptr<Shape>` to `DrawnDisk`, `DrawnBox`, `DrawnWedge` | `Shape_TE` holding `ValueDisk`, `ValueBox`, `ValueWedge` | `std::polymorphic<Shape>` holding the `Drawn*` types | `Poly<DrawModel>` holding the `Value*` types |
| `area` | `ScalarDisk`, `ScalarBox`, `ScalarWedge`, each in its own vector | `std::unique_ptr<Area>` to `AreaDisk`, `AreaBox`, `AreaWedge` | `AreaErased` holding the `Value*` types | `std::polymorphic<Area>` holding the `Area*` types | `AnyShape` (`Poly<ShapeModel>`) holding the `Scalar*` types |
| `copy` | copy of the three `Value*` vectors | `CloneShape::clone()` through `std::unique_ptr<CloneShape>` | copy of `std::vector<Shape_TE>` | copy of `std::vector<std::polymorphic<Shape>>` | copy of `std::vector<AnyShape>` |
| `build` | three vectors of inline `Value*` | `std::make_unique<Drawn*>` | `Shape_TE` constructed from `Value*` | `std::polymorphic<Shape>` constructed in place from `Drawn*` | `AnyShape` constructed from `Scalar*` |
| `cycle` and `cycle_N` | three vectors of inline `Value*` | `std::unique_ptr<CycleShape>` to `CycleDisk`, `CycleBox`, `CycleWedge` | `CycleErased` holding the `Value*` types | `std::polymorphic<CycleShape>` holding the `Cycle*` types | `Poly<CycleModel>` holding the `Value*` types |

`draw`, `copy`, and `build` are the only rows whose `type_erasure` bar is the library type `Shape_TE`. `area` times `AreaErased`. `cycle` times `CycleErased`. Both are local classes written in the benchmark file. `AreaErased` is move-only and its vtable has `area` and `destroy`. `CycleErased` is move-only and its vtable has `draw`, `area`, and `destroy`. Neither has `copy`. The copy chart never constructs them.

The reflection label is three generated types. `DrawModel` has one method, `std::string draw()`. `ShapeModel` has `draw`, `area`, and `perimeter`. `CycleModel` has `std::string draw()` and `float area()`. `define_model` runs once per model, so each `Poly<Model>` has a different interface base. The `area` bar's reflection handle is an `AnyShape`: the timed call is `area()`, and the handle still carries the `Fn` objects for `draw` and `perimeter`. The `area` bar's type-erasure handle is an `AreaErased`, whose vtable has two function pointers and whose handle is an object pointer plus a vtable pointer.

`direct` is three monomorphic vectors. The other four labels are one vector of mixed handles. Before timing, the executable checks that the five programs in a family produce the same values. That check says the formulas agree. It does not say the layouts are the same kind of machine.

## The layouts that are actually timed

`Shape_TE`, `AreaErased`, and `CycleErased` share one shape. The handle is `void*` plus a pointer to a static vtable. A call is a member function of the handle that loads one function pointer and jumps. The thunk then calls the concrete method. `Shape_TE::draw` calls through `const T*`. The local area and cycle thunks do too. `Shape_TE` can copy, through `new T(*static_cast<const T*>)`. The local types delete their copy operations. All three always allocate with `new`.

`Poly<Model>` is the layout traced in [reflection.md](reflection.md). The handle holds two pointers per model method, then an object pointer and a vtable pointer. `shape.area()` calls the `Fn` in the handle. That function pointer lands back on the same `Poly` and calls a second function pointer in the static vtable. Copy and destroy live on that vtable. Every handle stores its own address inside each `Fn`, so copying a `Poly` writes those `self_` pointers on the copy as well as allocating a new object. Copying a `Shape_TE` copies two pointers and allocates.

`std::unique_ptr<Base>` is one pointer. The vptr is inside the heap object, and the call is a virtual call. The `copy` row does not copy a `unique_ptr` of `Shape`. `unique_ptr` does not clone the derived object, so that row uses another hierarchy, `CloneShape`, and a virtual `clone()` that returns `std::unique_ptr<CloneShape>`. The timed work is a virtual call plus a heap allocation of the clone.

`std::polymorphic<Base>` is libstdc++'s type, used as a value that owns a `Drawn*`, `Area*`, or `Cycle*` object. The bar is that implementation, on that hierarchy, at that object size. The suite does not open the library type and does not substitute another value-semantic design.

`direct` stores the concrete object in the vector. `ValueDisk` and `ScalarDisk` each store one `float`. There is no per-object allocation and no function pointer. The compiler sees the callee. Three typed vectors are not a closed mixed container either: a `std::variant` of the three types, or one vector of a discriminated union, is another layout, and it is not in the suite.

The `bytes` counter is `sizeof` of the handle, or `sizeof` of the object for `direct`. It reports the handle the loop walks. It does not report the heap object, the static vtable, or the vptr inside a derived object.

## Axes the suite does not cross

Type erasure is the set of ways to call a concrete operation without naming the concrete type in the caller. The ways differ in the generated code. This suite sits on one point of most of the axes below, and on a different point per family for the two axes it does move.

**Where the function pointers live.** One point is a static vtable and a two-pointer handle. That is `Shape_TE`, `AreaErased`, and `CycleErased`. Another point is one function pointer per operation stored in the handle, with a single jump and no table. Another is a compiler vptr inside the allocation, which is the `unique_ptr` rows. Another is both: pointers in the handle whose target loads a static table. That is `Poly`. A reflection program that emitted the `Shape_TE` shape would be reflection and would time like `Shape_TE`. The suite has no second generated shape, so the reflection bars are the cost of this `Poly`, not the cost of generating a vtable.

**How many indirect jumps a call performs.** `Shape_TE::draw` performs one, then an ordinary call on `T`. A virtual call performs one through the vptr. `Poly` performs two before the body. If the stored method is itself virtual, the thunk adds a further virtual call. The benchmark types used with `Poly` have non-virtual methods, so that extra jump is not in the data. `docs/reflection.md` covers why the second jump is there: `define_aggregate` adds data members, the callable `Fn` has to recover the `Poly`, and the `Poly` then uses the table.

**What the handle owns.** Every non-`direct` row heap-allocates each object, including when the value is a single `float`. A small-buffer handle would keep that `float` inside the handle and would remove the allocation from `build`, `copy`, and the short cycles. A non-owning handle would store a pointer and a table and would not allocate or delete. A shared handle would add a control block or an atomic count. None of those are built. Move-only and copyable are both present, but under one label and in different families: `area` and `cycle` erase with move-only local types, while `copy` erases with `Shape_TE`.

**How a copy is spelled.** `Shape_TE` and `Poly` copy through a function pointer that executes `new T(T const&)`. The traditional copy row calls a virtual `clone()`. `std::polymorphic` uses the library's copy. `AreaErased` and `CycleErased` have no copy. The copy chart therefore compares four copy implementations and does not include the two local erasures that the call charts use.

**What the stored type has to be.** Virtual dispatch and `std::polymorphic` require a type derived from the base named in the handle. `Shape_TE` requires `draw() const` convertible to `std::string`, and the type is unrelated to `Shape`. `Poly` requires an exact return type and exact parameter types, ignores const and `noexcept` on the function, and calls through `T*`. The acceptance rule changes which programs compile and which thunk the call uses. The suite stores types written to satisfy the rule of the row they sit on. It does not time a converting return, a method with parameters, a reference parameter, or a const/non-const mismatch. The library does accept parameters: `tests/reflection_parameter_model.cpp` calls methods of two and three parameters, including a `float&`. No benchmark calls them.

**Where the bytes of one iteration sit.** `direct` scans contiguous objects of one type. The other rows scan a contiguous array of handles and chase one heap node per element. An inline buffer, a segregated array, or a closed `std::variant` changes that chase without changing the name of the technique. `draw()` also builds a `std::string` that fits in the small-string buffer. The root README records that this construction is what the `draw` family measures. A difference of one load is inside that work. `area()` returns `float`, so the call is a larger fraction of that family, and the handles compared there are still `AreaErased` against `AnyShape` against `unique_ptr<Area>`.

**How much of the time is the call.** `build` is construction. `copy` is construction of a clone. `cycle` is construction, one `draw()`, one `area()`, and destruction. `cycle_5` through `cycle_5000` repeat the two calls before the object dies. The long cycles are where dispatch can dominate allocation, for these layouts. They still compare one `CycleErased` with one `Poly<CycleModel>` with one `unique_ptr<CycleShape>`. Extending the cycle does not sample a new erasure.

## What a bar can support

A bar supports a statement about the function that produced it: this loop, this handle, this allocation, this signature, compiled by this GCC, on this machine. The relative chart drops `direct` and scales the other four to the fastest of those four. The remaining four are still the four machines in the table, not four techniques with their layouts held fixed.

Comparing `reflection_type_erasure` with `type_erasure` on the `area` chart compares an `AnyShape` of three methods and two jumps with an `AreaErased` of one method and one jump. Comparing them on the `cycle_5000` chart compares `Poly<CycleModel>` with `CycleErased`. A generator that stamped `CycleErased`'s vtable out of `CycleModel`, with a two-pointer handle and one jump, would answer a different question. That program is not in `benchmarks/`.

A suite that could support a ranking would have to name the layout it varies and hold the rest still. The layouts that change the numbers, and that this binary does not build, include a one-jump table with the same methods as the `Poly` it is compared to, the same table generated by `define_model` rather than written by hand, function pointers stored in the handle with no second jump, a small buffer for the one-`float` values, a non-owning handle, and a closed mixed container beside the three monomorphic vectors. The call used for the ranking has to be the `float` call, or another call whose body is small relative to a jump, on one signature, including a signature with parameters if that is part of the claim.

Until those programs exist, the useful reading of a directory under `benchmark_results/` is the reading the charts already are: timings of the functions in the table, on the machine and with the flags named at the top of that directory.
