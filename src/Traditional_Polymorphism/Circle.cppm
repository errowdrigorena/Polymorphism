export module shapes:circle;

import std;
import :shape;

export class Circle final : public Shape {
public:
    std::string draw() const override;
};
