#pragma once

#include <string>

#include "Shape.hpp"

class Rectangle final : public Shape {
public:
    std::string draw() const override;
};

inline std::string Rectangle::draw() const {
    return "Drawing Rectangle";
}
