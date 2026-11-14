module;

#include <string>

export module shapes:triangle;

import :shape;

export class Triangle final : public Shape {
public:
    std::string draw() const override;
};
