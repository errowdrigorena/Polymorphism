import std;
import polymorphic;

int main()
{
    std::vector<PolyShape> shapes;
    shapes.emplace_back(std::in_place_type<Circle>);
    shapes.emplace_back(std::in_place_type<Triangle>);
    shapes.emplace_back(std::in_place_type<Rectangle>);

    for (const auto& shape : shapes) {
        std::cout << shape->draw() << '\n';
    }

    const PolyShape copy = shapes.front();
    std::cout << "Copied: " << copy->draw() << '\n';
}
