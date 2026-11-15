export module shapes:rectangle;

import std;
import :shape;

export class Rectangle final : public Shape {
public:
    std::string draw() const override;
};
