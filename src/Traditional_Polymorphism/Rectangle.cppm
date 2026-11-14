module;

#include <string>

export module shapes:rectangle;

import :shape;

export class Rectangle final : public Shape {
public:
    std::string draw() const override;
};
