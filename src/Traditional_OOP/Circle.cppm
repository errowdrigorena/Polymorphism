module;

#include <string>

export module shapes:circle;

import :shape;

export class Circle final : public Shape {
public:
    std::string draw() const override;
};

std::string Circle::draw() const
{
    return "Drawing Circle";
}
