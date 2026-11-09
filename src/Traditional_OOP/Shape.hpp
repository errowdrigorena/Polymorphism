#pragma once

#include <string>

class Shape {
public:
    virtual ~Shape() = default;
    Shape() = default;
    Shape(const Shape&) = default;
    Shape& operator=(const Shape&) = default;
    Shape(Shape&&) noexcept = default;
    Shape& operator=(Shape&&) noexcept = default;

    virtual std::string draw() const = 0;
};
