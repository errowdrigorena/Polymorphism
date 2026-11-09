#pragma once

#include <string>

#include "Shape.hpp"

class Circle final : public Shape {
public:
    std::string draw() const override;
};

inline std::string Circle::draw() const {
    return "Drawing Circle";
}
