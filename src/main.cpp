#include <iostream>
#include <memory>
#include <string>
#include <vector>

import shapes;

int main()
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.emplace_back(std::make_unique<Circle>());
    shapes.emplace_back(std::make_unique<Triangle>());
    shapes.emplace_back(std::make_unique<Rectangle>());

    for (const auto& shape : shapes) {
        std::cout << shape->draw() << '\n';
    }
}
