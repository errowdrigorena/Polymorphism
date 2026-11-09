#pragma once

#include <string>

#include "Shape.hpp"

class Triangle final : public Shape {
public:
    std::string draw() const override;
};

inline std::string Triangle::draw() const {
    return "Drawing Triangle";
}
