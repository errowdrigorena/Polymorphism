export module shapes:triangle;

import std;
import :shape;

export class Triangle final : public Shape {
public:
    std::string draw() const override;
};
