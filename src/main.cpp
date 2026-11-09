#include <iostream>
#include <memory>
#include <vector>

#include "Traditional_OOP/Circle.hpp"
#include "Traditional_OOP/Rectangle.hpp"
#include "Traditional_OOP/Shape.hpp"
#include "Traditional_OOP/Triangle.hpp"

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.emplace_back(std::make_unique<Circle>());
    shapes.emplace_back(std::make_unique<Triangle>());
    shapes.emplace_back(std::make_unique<Rectangle>());

    for (const auto& shape : shapes) {
        std::cout << shape->draw() << '\n';
    }
}
