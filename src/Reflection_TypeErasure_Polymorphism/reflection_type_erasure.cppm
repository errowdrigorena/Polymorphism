export module reflection_type_erasure;

import std;

export import :circle;
export import :square;
export import :triangle;

// A function is part of the model when it is a public, named, non-special member.
consteval bool is_regular_method(std::meta::info member)
{
    return std::meta::is_function(member) && std::meta::is_class_member(member) &&
           std::meta::is_public(member) && !std::meta::is_special_member_function(member) &&
           std::meta::has_identifier(member);
}

template <typename R, typename... Args> using ErasedFn = R (*)(void*, Args...);

// define_aggregate can add a data member, not a function. This is the callable
// stored under the method's name.
template <typename R, typename... Args> struct Fn {
    R (*fn_)(void*, Args...) = nullptr;
    void* self_ = nullptr;

    R operator()(Args... args) const
    {
        if (fn_ == nullptr) {
            if constexpr (std::is_void_v<R>) {
                return;
            }
            else {
                return R{};
            }
        }
        return fn_(self_, args...);
    }
};

consteval std::meta::info erased_fn_of(std::meta::info member)
{
    std::vector<std::meta::info> arguments{std::meta::return_type_of(member)};
    const auto parameter_count = std::meta::parameters_of(member).size();
    for (std::size_t index = 0; index < parameter_count; ++index) {
        arguments.push_back(std::meta::type_of(std::meta::parameters_of(member)[index]));
    }
    return std::meta::substitute(^^ErasedFn, arguments);
}

consteval std::meta::info callable_fn_of(std::meta::info member)
{
    std::vector<std::meta::info> arguments{std::meta::return_type_of(member)};
    const auto parameter_count = std::meta::parameters_of(member).size();
    for (std::size_t index = 0; index < parameter_count; ++index) {
        arguments.push_back(std::meta::type_of(std::meta::parameters_of(member)[index]));
    }
    return std::meta::substitute(^^Fn, arguments);
}

// The methods of this class are the vtable. Any other class of methods works the same way.
export struct ShapeModel {
    void draw()
    {
    }
    float area()
    {
        return {};
    }
    float perimeter()
    {
        return {};
    }
};

using CopyFn = void* (*)(const void*);
using DestroyFn = void (*)(void*) noexcept;

export template <typename Model> struct VTable;

// define_aggregate can add data members, not functions. A callable with the
// method's name is what makes shape.draw() a direct call.
export template <typename Model> struct Interface;

template <typename Model> consteval void define_vtable()
{
    constexpr auto access = std::meta::access_context::current();
    const auto total = std::meta::members_of(^^Model, access).size();
    std::vector<std::meta::info> methods;
    for (std::size_t index = 0; index < total; ++index) {
        const auto member = std::meta::members_of(^^Model, access)[index];
        if (is_regular_method(member)) {
            methods.push_back(member);
        }
    }

    std::vector<std::meta::info> specs;
    for (auto method : methods) {
        std::meta::data_member_options options{};
        options.name = std::meta::identifier_of(method);
        specs.push_back(std::meta::data_member_spec(erased_fn_of(method), options));
    }

    std::meta::data_member_options copy_options{};
    copy_options.name = "copy";
    specs.push_back(std::meta::data_member_spec(std::meta::dealias(^^CopyFn), copy_options));
    std::meta::data_member_options destroy_options{};
    destroy_options.name = "destroy";
    specs.push_back(std::meta::data_member_spec(std::meta::dealias(^^DestroyFn), destroy_options));

    std::meta::define_aggregate(^^VTable<Model>, specs);
}

template <typename Model> consteval void define_interface()
{
    constexpr auto access = std::meta::access_context::current();
    const auto total = std::meta::members_of(^^Model, access).size();
    std::vector<std::meta::info> specs;
    for (std::size_t index = 0; index < total; ++index) {
        const auto member = std::meta::members_of(^^Model, access)[index];
        if (!is_regular_method(member)) {
            continue;
        }
        std::meta::data_member_options options{};
        options.name = std::meta::identifier_of(member);
        specs.push_back(std::meta::data_member_spec(callable_fn_of(member), options));
    }

    std::meta::define_aggregate(^^Interface<Model>, specs);
}

// define_aggregate has to run from a consteval block. Add a line here for another model.
consteval
{
    define_vtable<ShapeModel>();
    define_interface<ShapeModel>();
}

template <typename T, typename Model> consteval bool implements_model()
{
    constexpr auto access = std::meta::access_context::current();
    const auto total = std::meta::members_of(^^Model, access).size();
    for (std::size_t index = 0; index < total; ++index) {
        const auto required = std::meta::members_of(^^Model, access)[index];
        if (!is_regular_method(required)) {
            continue;
        }

        bool found = false;
        const auto candidates = std::meta::members_of(^^T, access).size();
        for (std::size_t candidate_index = 0; candidate_index < candidates; ++candidate_index) {
            const auto candidate = std::meta::members_of(^^T, access)[candidate_index];
            if (is_regular_method(candidate) &&
                std::meta::identifier_of(candidate) == std::meta::identifier_of(required) &&
                std::meta::return_type_of(candidate) == std::meta::return_type_of(required) &&
                std::meta::parameters_of(candidate).size() ==
                    std::meta::parameters_of(required).size()) {
                found = true;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

template <typename T, std::meta::info Slot, std::meta::info Concrete> void assign_slot(auto& table)
{
    static_assert(std::meta::parameters_of(Concrete).size() == 0);
    using Result = [:std::meta::return_type_of(Concrete):];

    if constexpr (std::is_void_v<Result>) {
        table.[:Slot:] = [](void* object) { static_cast<T*>(object)->[:Concrete:](); };
    }
    else {
        table.[:Slot:] = [](void* object) -> Result {
            return static_cast<T*>(object)->[:Concrete:]();
        };
    }
}

template <typename Model, typename T> VTable<Model> make_vtable()
{
    VTable<Model> table{};
    template for (constexpr auto slot : std::define_static_array(
                      std::meta::nonstatic_data_members_of(
                          ^^VTable<Model>, std::meta::access_context::current())))
    {
        template for (constexpr auto member : std::define_static_array(
                          std::meta::members_of(^^T, std::meta::access_context::current())))
        {
            if constexpr (is_regular_method(member) &&
                          std::meta::identifier_of(member) == std::meta::identifier_of(slot)) {
                assign_slot<T, slot, member>(table);
            }
        }
    }

    table.copy = [](const void* object) -> void* { return new T(*static_cast<const T*>(object)); };
    table.destroy = [](void* object) noexcept { delete static_cast<T*>(object); };
    return table;
}

// Same ownership as the hand-written type erasure. Model supplies the vtable,
// and Interface exposes those methods as shape.draw().
export template <typename Model> class Poly : public Interface<Model> {
    template <std::meta::info Surface, std::meta::info Slot, std::meta::info Method>
    void bind_method()
    {
        using Result = [:std::meta::return_type_of(Method):];
        static_assert(std::meta::parameters_of(Method).size() == 0);

        auto& target = static_cast<Interface<Model>&>(*this).[:Surface:];
        if constexpr (std::is_void_v<Result>) {
            target.fn_ = [](void* self) {
                auto* poly = static_cast<Poly*>(self);
                if (poly->object_ != nullptr) {
                    poly->vtable_->[:Slot:](poly->object_);
                }
            };
        }
        else {
            target.fn_ = [](void* self) -> Result {
                auto* poly = static_cast<Poly*>(self);
                if (poly->object_ == nullptr) {
                    return Result{};
                }
                return poly->vtable_->[:Slot:](poly->object_);
            };
        }
        target.self_ = this;
    }

    void bind()
    {
        if (vtable_ == nullptr) {
            static_cast<Interface<Model>&>(*this) = {};
            return;
        }

        template for (constexpr auto surface : std::define_static_array(
                          std::meta::nonstatic_data_members_of(
                              ^^Interface<Model>, std::meta::access_context::current())))
        {
            template for (constexpr auto slot : std::define_static_array(
                              std::meta::nonstatic_data_members_of(
                                  ^^VTable<Model>, std::meta::access_context::current())))
            {
                template for (constexpr auto method : std::define_static_array(
                                  std::meta::members_of(^^Model,
                                                        std::meta::access_context::current())))
                {
                    if constexpr (is_regular_method(method) &&
                                  std::meta::identifier_of(surface) ==
                                      std::meta::identifier_of(slot) &&
                                  std::meta::identifier_of(slot) ==
                                      std::meta::identifier_of(method)) {
                        bind_method<surface, slot, method>();
                    }
                }
            }
        }
    }

public:
    Poly() = default;

    template <typename T>
        requires(implements_model<std::decay_t<T>, Model>() &&
                 !std::same_as<std::decay_t<T>, Poly>)
    Poly(T&& shape)
        : object_(new std::decay_t<T>(std::forward<T>(shape))),
          vtable_(&vtable_for<std::decay_t<T>>())
    {
        bind();
    }

    Poly(const Poly& other)
        : object_(other.object_ == nullptr ? nullptr : other.vtable_->copy(other.object_)),
          vtable_(other.vtable_)
    {
        bind();
    }

    Poly(Poly&& other) noexcept
        : object_(std::exchange(other.object_, nullptr)),
          vtable_(std::exchange(other.vtable_, nullptr))
    {
        static_cast<Interface<Model>&>(other) = {};
        bind();
    }

    Poly& operator=(const Poly& other)
    {
        if (this != &other) {
            Poly copy(other);
            swap(copy);
            bind();
        }
        return *this;
    }

    Poly& operator=(Poly&& other) noexcept
    {
        if (this != &other) {
            reset();
            object_ = std::exchange(other.object_, nullptr);
            vtable_ = std::exchange(other.vtable_, nullptr);
            static_cast<Interface<Model>&>(other) = {};
            bind();
        }
        return *this;
    }

    ~Poly()
    {
        reset();
    }

    bool has_value() const noexcept
    {
        return object_ != nullptr;
    }

private:
    void reset() noexcept
    {
        if (object_ != nullptr) {
            vtable_->destroy(object_);
            object_ = nullptr;
            vtable_ = nullptr;
        }
        static_cast<Interface<Model>&>(*this) = {};
    }

    void swap(Poly& other) noexcept
    {
        std::swap(object_, other.object_);
        std::swap(vtable_, other.vtable_);
    }

    template <typename T> static const VTable<Model>& vtable_for() noexcept
    {
        static const VTable<Model> table = make_vtable<Model, T>();
        return table;
    }

    void* object_ = nullptr;
    const VTable<Model>* vtable_ = nullptr;
};

export using AnyShape = Poly<ShapeModel>;
