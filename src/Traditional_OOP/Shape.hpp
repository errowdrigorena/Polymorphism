#pragma once

#include <string>

class Shape {
public:
    virtual ~Shape() = default;
    virtual std::string draw() const = 0;
};
