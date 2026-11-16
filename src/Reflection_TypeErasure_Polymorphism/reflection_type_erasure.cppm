export module reflection_type_erasure;

import std;

export import :circle;
export import :square;
export import :triangle;

// A stored type supplies a public, named, non-special member.
consteval bool is_regular_method(std::meta::info member)
{
    return std::meta::is_function(member) && std::meta::is_class_member(member) &&
           std::meta::is_public(member) && !std::meta::is_special_member_function(member) &&
           std::meta::has_identifier(member);
}

// The model contributes only its pure virtual methods. Its destructor is a
// special member, so it stays out of the vtable.
consteval bool is_interface_method(std::meta::info member)
{
    return is_regular_method(member) && std::meta::is_pure_virtual(member);
}

// Every non-special member function has to be pure virtual. Constructors and
// the destructor are special members and are not part of this check.
template <typename Model> consteval void require_pure_virtual_methods()
{
    constexpr auto access = std::meta::access_context::unchecked();
    const auto total = std::meta::members_of(^^Model, access).size();
    for (std::size_t index = 0; index < total; ++index) {
        const auto member = std::meta::members_of(^^Model, access)[index];
        if (!std::meta::is_function(member) || !std::meta::is_class_member(member) ||
            std::meta::is_special_member_function(member)) {
            continue;
        }
        if (std::meta::is_pure_virtual(member)) {
            continue;
        }

        std::string message{"model method must be pure virtual: "};
        if (std::meta::has_identifier(member)) {
            message += std::meta::identifier_of(member);
        }
        else {
            message += std::meta::display_string_of(member);
        }
        throw std::meta::exception(message, member);
    }
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
        return fn_(self_, std::forward<Args>(args)...);
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

// These pure virtual methods are the vtable. Any other interface works the same way.
export struct ShapeModel {
    virtual ~ShapeModel() = default;

    virtual void draw() = 0;
    virtual float area() = 0;
    virtual float perimeter() = 0;
};

using CopyFn = void* (*)(const void*);
using DestroyFn = void (*)(void*) noexcept;

export template <typename Model> struct VTable;

// define_aggregate can add data members, not functions. A callable with the
// method's name is what makes shape.draw() a direct call.
export template <typename Model> struct Interface;

template <typename Model> consteval void define_vtable()
{
    require_pure_virtual_methods<Model>();

    constexpr auto access = std::meta::access_context::current();
    const auto total = std::meta::members_of(^^Model, access).size();
    std::vector<std::meta::info> methods;
    for (std::size_t index = 0; index < total; ++index) {
        const auto member = std::meta::members_of(^^Model, access)[index];
        if (is_interface_method(member)) {
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
        if (!is_interface_method(member)) {
            continue;
        }
        std::meta::data_member_options options{};
        options.name = std::meta::identifier_of(member);
        specs.push_back(std::meta::data_member_spec(callable_fn_of(member), options));
    }

    std::meta::define_aggregate(^^Interface<Model>, specs);
}

// define_aggregate has to run from a consteval block. An importer registers
// another interface with define_model<ThatInterface>() in its own block.
export template <typename Model> consteval void define_model()
{
    define_vtable<Model>();
    define_interface<Model>();
}

consteval
{
    define_model<ShapeModel>();
}

consteval bool same_signature(std::meta::info required, std::meta::info candidate)
{
    if (std::meta::return_type_of(candidate) != std::meta::return_type_of(required)) {
        return false;
    }
    const auto count = std::meta::parameters_of(required).size();
    if (std::meta::parameters_of(candidate).size() != count) {
        return false;
    }
    for (std::size_t index = 0; index < count; ++index) {
        const auto required_parameter =
            std::meta::type_of(std::meta::parameters_of(required)[index]);
        const auto candidate_parameter =
            std::meta::type_of(std::meta::parameters_of(candidate)[index]);
        if (required_parameter != candidate_parameter) {
            return false;
        }
    }
    return true;
}

template <typename T, typename Model> consteval bool implements_model()
{
    constexpr auto access = std::meta::access_context::current();
    const auto total = std::meta::members_of(^^Model, access).size();
    for (std::size_t index = 0; index < total; ++index) {
        const auto required = std::meta::members_of(^^Model, access)[index];
        if (!is_interface_method(required)) {
            continue;
        }

        bool found = false;
        const auto candidates = std::meta::members_of(^^T, access).size();
        for (std::size_t candidate_index = 0; candidate_index < candidates; ++candidate_index) {
            const auto candidate = std::meta::members_of(^^T, access)[candidate_index];
            if (is_regular_method(candidate) &&
                std::meta::identifier_of(candidate) == std::meta::identifier_of(required) &&
                same_signature(required, candidate)) {
                found = true;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

// One instantiation per parameter. parameters_of returns a temporary vector,
// so the index stays in the alias instead of a pack that holds that vector.
template <std::meta::info Member, std::size_t Index>
using parameter_type_at = [:std::meta::type_of(std::meta::parameters_of(Member)[Index]):];

template <typename T, std::meta::info Slot, std::meta::info Concrete, std::size_t... ParameterIndex>
void assign_slot(auto& table, std::index_sequence<ParameterIndex...>)
{
    using Result = [:std::meta::return_type_of(Concrete):];

    if constexpr (std::is_void_v<Result>) {
        table.[:Slot:] = [](void* object, parameter_type_at<Concrete, ParameterIndex>... args) {
            static_cast<T*>(object)->[:Concrete:](
                std::forward<parameter_type_at<Concrete, ParameterIndex>>(args)...);
        };
    }
    else {
        table.[:Slot:] =
            [](void* object, parameter_type_at<Concrete, ParameterIndex>... args) -> Result {
            return static_cast<T*>(object)->[:Concrete:](
                std::forward<parameter_type_at<Concrete, ParameterIndex>>(args)...);
        };
    }
}

template <typename T, std::meta::info Slot, std::meta::info Concrete> void assign_slot(auto& table)
{
    constexpr auto count = std::meta::parameters_of(Concrete).size();
    assign_slot<T, Slot, Concrete>(table, std::make_index_sequence<count>{});
}

template <typename Model, typename T> VTable<Model> make_vtable()
{
    VTable<Model> table{};
    template for (constexpr auto slot :
                  std::define_static_array(std::meta::nonstatic_data_members_of(
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
    template <std::meta::info Surface, std::meta::info Slot, std::meta::info Method,
              std::size_t... ParameterIndex>
    void bind_method(std::index_sequence<ParameterIndex...>)
    {
        using Result = [:std::meta::return_type_of(Method):];
        auto& target = static_cast<Interface<Model>&>(*this).[:Surface:];
        if constexpr (std::is_void_v<Result>) {
            target.fn_ = [](void* self, parameter_type_at<Method, ParameterIndex>... args) {
                auto* poly = static_cast<Poly*>(self);
                if (poly->object_ != nullptr) {
                    poly->vtable_->[:Slot:](
                        poly->object_,
                        std::forward<parameter_type_at<Method, ParameterIndex>>(args)...);
                }
            };
        }
        else {
            target.fn_ =
                [](void* self, parameter_type_at<Method, ParameterIndex>... args) -> Result {
                auto* poly = static_cast<Poly*>(self);
                if (poly->object_ == nullptr) {
                    return Result{};
                }
                return poly->vtable_->[:Slot:](
                    poly->object_,
                    std::forward<parameter_type_at<Method, ParameterIndex>>(args)...);
            };
        }
        target.self_ = this;
    }

    template <std::meta::info Surface, std::meta::info Slot, std::meta::info Method>
    void bind_method()
    {
        constexpr auto count = std::meta::parameters_of(Method).size();
        bind_method<Surface, Slot, Method>(std::make_index_sequence<count>{});
    }

    void bind()
    {
        if (vtable_ == nullptr) {
            static_cast<Interface<Model>&>(*this) = {};
            return;
        }

        template for (constexpr auto surface :
                      std::define_static_array(std::meta::nonstatic_data_members_of(
                          ^^Interface<Model>, std::meta::access_context::current())))
        {
            template for (constexpr auto slot :
                          std::define_static_array(std::meta::nonstatic_data_members_of(
                              ^^VTable<Model>, std::meta::access_context::current())))
            {
                template for (constexpr auto method :
                              std::define_static_array(std::meta::members_of(
                                  ^^Model, std::meta::access_context::current())))
                {
                    if constexpr (is_interface_method(method) &&
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
        requires(implements_model<std::decay_t<T>, Model>() && !std::same_as<std::decay_t<T>, Poly>)
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
