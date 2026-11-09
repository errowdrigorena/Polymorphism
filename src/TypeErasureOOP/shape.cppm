module;

#include <concepts>
#include <string>
#include <type_traits>
#include <utility>

export module type_erasure:shape;

import :drawable;

export class Shape_TE {
public:
    Shape_TE() = default;

    template <typename T>
        requires DrawableShape_TE<std::decay_t<T>> && (!std::same_as<std::decay_t<T>, Shape_TE>)
    Shape_TE(T&& shape)
        : object_(new std::decay_t<T>(std::forward<T>(shape))),
          vtable_(&vtable_for<std::decay_t<T>>())
    {
    }

    Shape_TE(const Shape_TE& other)
        : object_(other.object_ == nullptr ? nullptr : other.vtable_->copy(other.object_)),
          vtable_(other.vtable_)
    {
    }

    Shape_TE(Shape_TE&& other) noexcept
        : object_(std::exchange(other.object_, nullptr)),
          vtable_(std::exchange(other.vtable_, nullptr))
    {
    }

    Shape_TE& operator=(const Shape_TE& other)
    {
        if (this != &other) {
            Shape_TE copy(other);
            swap(copy);
        }
        return *this;
    }

    Shape_TE& operator=(Shape_TE&& other) noexcept
    {
        if (this != &other) {
            reset();
            object_ = std::exchange(other.object_, nullptr);
            vtable_ = std::exchange(other.vtable_, nullptr);
        }
        return *this;
    }

    ~Shape_TE()
    {
        reset();
    }

    std::string draw() const
    {
        if (!has_value()) {
            return "<empty>";
        }
        return vtable_->draw(object_);
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
    }

    void swap(Shape_TE& other) noexcept
    {
        std::swap(object_, other.object_);
        std::swap(vtable_, other.vtable_);
    }

    struct VTable {
        std::string (*draw)(const void*);
        void* (*copy)(const void*);
        void (*destroy)(void*) noexcept;
    };

    template <typename T> static const VTable& vtable_for() noexcept
    {
        static const VTable table{
            [](const void* object) -> std::string { return static_cast<const T*>(object)->draw(); },
            [](const void* object) -> void* { return new T(*static_cast<const T*>(object)); },
            [](void* object) noexcept { delete static_cast<T*>(object); },
        };
        return table;
    }

    void* object_ = nullptr;
    const VTable* vtable_ = nullptr;
};
