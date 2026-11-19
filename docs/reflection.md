# Reflection

`reflection_type_erasure` reads a model class at compile time and finishes two class templates from it. `Poly<Model>` then owns a heap object and calls that object's methods through the members the generator added. `AnyShape` is `Poly<ShapeModel>`. The stored type does not inherit the model. `Circle`, `Square`, and `Triangle` are ordinary structs with ordinary member functions.

The generator is `define_model`, in `src/Reflection_TypeErasure_Polymorphism/reflection_type_erasure.cppm`. `ShapeModel` is registered in that file. Any other model is registered by a `consteval` block in the translation unit that owns the model.

## What a model is

`ShapeModel` is an abstract class used as a description:

```cpp
export struct ShapeModel {
    virtual ~ShapeModel() = default;

    virtual void draw() = 0;
    virtual float area() = 0;
    virtual float perimeter() = 0;
};
```

The destructor is a special member, so the generator never turns it into a slot. The stored object is destroyed by a separate function pointer that executes `delete` on the exact type that was stored. That type does not need a virtual destructor for the `delete` to call the right destructor, and it does not need to derive from `ShapeModel`.

A model method that the generator will expose is a public, named, non-special member function that is pure virtual. `define_model` rejects a model that has any other non-special member function.

## The purity check

`require_pure_virtual_methods<Model>()` runs before either aggregate is defined. It walks `std::meta::members_of(^^Model, std::meta::access_context::unchecked())`. `unchecked()` sees private and protected members.

For each member the walk skips three cases: it is not a function, it is not a class member, or it is a special member function. Constructors, the destructor, and the copy and move assignment operators fall in that last case. A defaulted virtual destructor is allowed, and it is not required to be pure.

Every remaining member function has to be pure virtual. That includes a static member function, a method with a body, and a non-virtual method. On failure the function throws `std::meta::exception`. The message is `model method must be pure virtual: ` followed by `identifier_of` when the member has an identifier, otherwise `display_string_of`. The exception also carries the member's reflection.

`define_model` is `consteval`, and the call sits in a `consteval` block. An exception that leaves that block makes the program ill-formed. A `consteval` function may catch it. `tests/reflection_impure_model.cpp` does that: `ImplementedModel::implemented()` has a body, `accepts_model<ImplementedModel>()` catches `std::meta::exception`, and the file still compiles. `static_assert(!accepts_model<ImplementedModel>())` records the rejection.

A pure virtual that is private or protected passes this check. It still does not become a slot. The slot filter requires a public member, and that filter is applied later.

Data members and nested types are not functions, so the check ignores them. They do not become slots either.

## Why the surface is made of data members

`std::meta::define_aggregate` adds data members to an incomplete class. It does not add member functions. The library therefore declares two incomplete class templates and completes them inside `define_model`:

```cpp
export template <typename Model> struct VTable;
export template <typename Model> struct Interface;
```

`define_vtable<Model>()` completes `VTable<Model>`. `define_interface<Model>()` completes `Interface<Model>`. Both walks use `access_context::current()` and keep a member only when `is_interface_method` is true.

`is_interface_method` requires all of the following:

- `is_function`
- `is_class_member`
- `is_public`
- not `is_special_member_function`
- `has_identifier`
- `is_pure_virtual`

The name of each generated data member is `identifier_of` of that method. A member function without an identifier cannot be a slot, because the data member needs that name. A non-pure function without an identifier still fails the purity check.

Method slots are added in the order `members_of` yields the matching methods. `VTable<Model>` then gets two more data members, always, with fixed names and fixed types:

| Member | Type |
| --- | --- |
| `copy` | `void* (*)(const void*)` |
| `destroy` | `void (*)(void*) noexcept` |

`CopyFn` and `DestroyFn` are aliases, so the spec passed to `define_aggregate` is `dealias` of those aliases. A model method named `copy` or `destroy` asks `define_aggregate` for two data members of the same name.

`Interface<Model>` gets one data member per interface method and nothing else. `copy` and `destroy` stay on the vtable. They are ownership operations `Poly` calls by name. They are not part of the call syntax.

## The two function types

Each interface method becomes one function-pointer type and one callable object type.

`ErasedFn<R, Args...>` is `R (*)(void*, Args...)`. The `void*` is the stored object. It is written into the alias. It is not a parameter of the model method. `erased_fn_of` builds the template argument list as the method's return type, then `type_of` of each parameter in order, and passes that list to `std::meta::substitute` on `^^ErasedFn`. For `float area()` the vtable member `area` has type `float (*)(void*)`. For `float accumulate(float& total, float step)` it has type `float (*)(void*, float&, float)`.

`Fn<R, Args...>` is the type stored in the interface:

```cpp
template <typename R, typename... Args> struct Fn {
    R (*fn_)(void*, Args...) = nullptr;
    void* self_ = nullptr;

    R operator()(Args... args) const;
};
```

`callable_fn_of` substitutes `^^Fn` with the same argument list. `Interface<ShapeModel>` therefore contains three public data members, `draw`, `area`, and `perimeter`, of types `Fn<void>`, `Fn<float>`, and `Fn<float>`.

`shape.area()` compiles because `area` is a data member whose type has `operator()`. `Poly<Model>` publicly inherits `Interface<Model>`, so those members are the call syntax. `Poly` has no virtual functions and no member function named `area`.

`operator()` calls `fn_(self_, ...)`. When `fn_` is null, a `void` method returns, and any other method returns `R{}`. `0.f` for `float`, an empty `std::string` for `std::string`. `self_` is the address of the `Poly`, stored as `void*`. It is not the stored object.

Each `Fn` is two pointers living inside the handle. `Poly` adds the object pointer and the vtable pointer. `ShapeModel` has three methods, so an `AnyShape` holds eight pointers. `DrawModel` has one method, so a `Poly<DrawModel>` holds four. A `Shape_TE` holds two pointers however many operations its vtable has, because that vtable is a static table outside the handle.

## Registering a model

`define_aggregate` runs as part of the evaluation of a `consteval` block. `define_model` is the function the block calls:

```cpp
export template <typename Model> consteval void define_model()
{
    define_vtable<Model>();
    define_interface<Model>();
}

consteval
{
    define_model<ShapeModel>();
}
```

That block completes `VTable<ShapeModel>` and `Interface<ShapeModel>` while the module is compiled. An importer that wants another interface writes the same block itself. `tests/reflection_parameter_model.cpp` and `tests/reflection_pure_model.cpp` do. On this GCC the body of `define_model` is instantiated in the importer, and that instantiation looks up `std::vector`, so the importer also has `import std;`. The benchmark file keeps the block out of the anonymous namespace for the same registration.

The purity check runs first. If it throws, neither aggregate has been defined.

## Accepting a stored type

The converting constructor is constrained:

```cpp
template <typename T>
    requires(implements_model<std::decay_t<T>, Model>() && !std::same_as<std::decay_t<T>, Poly>)
Poly(T&& shape);
```

`std::decay_t<T>` is the type that will be allocated. The constraint rejects `Poly` itself so copy and move use the special members. Failure here is a constraint failure. It is not the `std::meta::exception` used for an impure model. There is no custom diagnostic for a missing method.

`implements_model<T, Model>()` walks the model's members with `access_context::current()`. For every interface method it searches `T` for a public, named, non-special member function with the same identifier and the same signature. Extra members on `T` are irrelevant. One matching candidate is enough. The candidate does not have to be virtual, and `T` does not have to inherit `Model`. `tests/reflection_shapes_cases.cpp` stores a local `Dot` that is unrelated to `ShapeModel`.

`same_signature` compares:

- `return_type_of` of the two functions, by reflection equality
- the number of parameters
- `type_of` of each parameter, in order, by reflection equality

`float` and `double` do not match. `float&` matches `float&`, and that includes the reference. `const float&` does not match `float&`. This is exact type equality. `Shape_TE` instead accepts a `draw() const` whose return type is convertible to `std::string`.

The comparison does not read const, volatile, ref-qualifiers, or `noexcept` on the function. A const method and a non-const method with the same parameter types compare equal. `parameters_of` does not include the implicit object parameter, so the const that would appear there is outside the check too.

## Building the vtable

`vtable_for<T>()` holds a function-local static `VTable<Model>` built by `make_vtable<Model, T>()`. There is one static table per stored type per model. Every `Poly` that owns that type points at it.

`members_of` and `nonstatic_data_members_of` return `std::vector<std::meta::info>`. An element of that vector is not a constant, so it cannot be a template argument or the operand of a splice. `std::meta::define_static_array` materializes the vector as a static array of reflections. `template for` instantiates its body once per element, and the loop variable of that instantiation is a constant reflection.

`make_vtable` expands every non-static data member of `VTable<Model>` and, inside that, every member of `T`. A `T` member is assigned into the slot when it is a regular method and the two identifiers are equal. The match at this point is the name. `same_signature` has already run in the constructor constraint, and it is not run again here.

`assign_slot` reads the parameter count from the concrete method and builds `std::make_index_sequence<count>`. Each index is its own instantiation of:

```cpp
template <std::meta::info Member, std::size_t Index>
using parameter_type_at = [:std::meta::type_of(std::meta::parameters_of(Member)[Index]):];
```

`parameters_of` returns a temporary vector, so the pack that survives is the pack of indices. The alias runs the query again for one index and splices that parameter's type into the lambda's parameter list.

The thunk is a non-capturing lambda, converted to a function pointer and stored in `table.[:Slot:]`. `[:Slot:]` is the data member whose name matched. `[:Concrete:]` is the member function of `T`. For a non-void return type the lambda is:

```cpp
[](void* object, parameter_type_at<Concrete, ParameterIndex>... args) -> Result {
    return static_cast<T*>(object)->[:Concrete:](
        std::forward<parameter_type_at<Concrete, ParameterIndex>>(args)...);
};
```

`Result` is the splice of `return_type_of` of the concrete method. A `void` method is a separate lambda because it cannot return a value. Both versions call the method on a `T*`, never on a `const T*`. A const method of `T` can be called through that pointer. A non-const method can be called as well. If the selected member of `T` is virtual, this call is a virtual call, under the ordinary rules of a call on `T*`. `Circle::area` is not virtual. The indirect call added by the library is the function pointer, and the body then runs as a normal call.

After the loops, `make_vtable` always sets:

```cpp
table.copy = [](const void* object) -> void* {
    return new T(*static_cast<const T*>(object));
};
table.destroy = [](void* object) noexcept {
    delete static_cast<T*>(object);
};
```

Those two assignments replace whatever the name match wrote into `copy` and `destroy`. The replacement is reached only if the match compiled. The name match visits every vtable data member, including `copy` and `destroy`. A public method of `T` named `copy` or `destroy` is passed to `assign_slot`, and the lambda is assigned to a slot of type `CopyFn` or `DestroyFn`. If that assignment is ill-formed, building the vtable is ill-formed.

The same thing happens for overloads. `implements_model` succeeds when one overload matches. `make_vtable` then instantiates `assign_slot` for every public method of that name. Each lambda is assigned to the one slot, whose type was fixed from the model. An overload whose signature does not match that function-pointer type makes the assignment ill-formed. A constructor can satisfy the constraint and still fail when its vtable is instantiated. When more than one overload is compatible with the slot, each assignment overwrites the previous one, in the order `members_of` yields them.

`T` has to be copy-constructible. The copy lambda is instantiated with the vtable, which is instantiated from the converting constructor. A move-only type fails there, including when the program never copies a `Poly`.

## Binding the handle

Constructing a `Poly` allocates the object and points at the static vtable, then calls `bind()`. `bind()` is what writes the `Fn` members. The vtable pointer alone is not enough to call `shape.area()`, because `area` lives in the base and its `fn_` starts out null.

If `vtable_` is null, `bind()` value-initializes the `Interface` base and returns. Every `Fn` is then null.

Otherwise `bind()` expands three ranges: the data members of `Interface<Model>`, the data members of `VTable<Model>`, and the members of `Model`. A triple is kept when the model member is an interface method and the three identifiers are equal. `copy` and `destroy` never match, because the interface has no such members and they are not interface methods.

`bind_method` installs one non-capturing lambda into that `Fn`. The lambda's parameter types come from the model method. Its body casts `self` back to `Poly*` and, when `object_` is non-null, calls the vtable slot:

```cpp
auto* poly = static_cast<Poly*>(self);
return poly->vtable_->[:Slot:](
    poly->object_,
    std::forward<parameter_type_at<Method, ParameterIndex>>(args)...);
```

When `object_` is null the lambda returns `Result{}`, or returns without a value when the method is `void`. `target.self_ = this` stores the address of this `Poly`.

The nested expansions exist so each surviving triple is a separate instantiation. At run time `bind()` performs the assignments that those instantiations produced.

`operator()` on `Fn` is a const member. The lambda it calls casts `self_` back to `Poly*`, a non-const pointer. `bind()` stored that address while `this` was non-const. A call through a `const Poly` still reaches a non-const method of `T`.

## One call

`AnyShape shape{Circle{1.0f}}; float a = shape.area();` does the following.

The constructor decays the argument to `Circle`. `implements_model<Circle, ShapeModel>()` finds `draw`, `area`, and `perimeter`. It heap-allocates a `Circle` and takes the address of `vtable_for<Circle>()`. That table's `area` member is `float (*)(void*)`, and the thunk calls `Circle::area` on a `Circle*`. `copy` heap-allocates a `Circle` copy. `destroy` deletes a `Circle*`.

`bind()` then sets `Interface::area`. Its `fn_` is `float (*)(void*)` and its `self_` is the address of `shape`.

`shape.area()` is `Fn<float>::operator()` on that member. `fn_` is non-null, so it calls `fn_(self_)`. The first lambda loads `shape.vtable_` and `shape.object_` and calls `vtable_->area(object_)`. The second lambda casts the `void*` to `Circle*` and calls `Circle::area()`, which returns `pi * radius * radius`.

Two indirect calls, then the body. The first recovers the `Poly` from `self_`. The second is the jump from the static table to `Circle::area`.

A default-constructed `AnyShape` never calls `bind()`. Its `Fn` members are value-initialized, `fn_` is null, and `area()` returns `0.f`. `has_value()` is `object_ != nullptr`. After a move, the source is in that empty state: its pointers are null and its `Interface` base has been value-initialized, so a later call takes the null-`fn_` path.

`tests/reflection_parameter_model.cpp` is the same path with parameters. `Poly<ParameterModel>::accumulate` is `Fn<float, float&, float>`. The reference is the parameter type, `std::forward<float&>` keeps it a reference, and the caller's `float` is updated by `Linear::accumulate`. `scale` takes two `float`s and `blend` takes three. Each arity is its own function-pointer type, generated from that method's parameter list.

## Ownership

The heap operations match `Shape_TE`. The difference is that every `Fn` stores the address of its own `Poly`, so any operation that creates or reuses a handle has to call `bind()` again. Copying the bytes of the `Interface` base would leave `self_` aimed at the previous handle.

| Operation | What happens |
| --- | --- |
| Convert from `T` | `new decay_t<T>(...)`, point at `vtable_for<decay_t<T>>()`, `bind()` |
| Copy | `vtable_->copy` when `object_` is non-null, copy the vtable pointer, `bind()` on the new handle |
| Move | Steal both pointers, value-initialize the source `Interface`, `bind()` on the destination |
| Copy assign | Copy-and-swap, then `bind()` on the left-hand side |
| Move assign | `reset()`, steal both pointers, value-initialize the source `Interface`, `bind()` |
| Destroy | `reset()` |

`swap` exchanges `object_` and `vtable_` only. It does not exchange the `Interface` base. After copy-and-swap the left-hand side holds the new pointers and still has its previous `Fn` members, so the assignment calls `bind()` before it returns. The temporary keeps the `Fn` members that were bound to the temporary. Its destructor does not use them. `reset()` calls `vtable_->destroy` on `object_`.

`reset()` destroys the object when the pointer is non-null, clears both pointers, and value-initializes the `Interface` base. The destroy thunk is `noexcept` and deletes `T*`.

Self-assignment is skipped. Copy assignment is copy-and-swap, so the strong guarantee of that pattern applies to the heap pointer. `bind()` runs after the swap has published the new pointers in the left-hand side.

## What this generator stores

The generated `Poly` is one layout:

- The stored type is unrelated to the model and is allocated with `new`.
- The handle contains, per method, a function pointer and a `self_` pointer, plus an object pointer and a vtable pointer.
- A call indirects through the `Fn`, lands back on that `Poly`, and indirects again through the static vtable.
- The vtable also has `copy` and `destroy`.
- The method match is name, return type, and parameter types.
- The thunk calls through `T*`.

A reflection generator can emit a different layout from the same language facility. This library emits this one. `docs/benchmarks.md` is about what follows from that when the timings are read as a comparison of techniques.
