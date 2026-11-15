import std;
import shapes;

// Owning handle for the classic hierarchy. The vector is move-only: copying it
// would have to clone each derived object, and Shape provides no virtual clone.
int main()
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.emplace_back(std::make_unique<Circle>());
    shapes.emplace_back(std::make_unique<Triangle>());
    shapes.emplace_back(std::make_unique<Rectangle>());

    // *shape has static type Shape. draw() is virtual, so the derived body runs.
    for (const auto& shape : shapes) {
        std::cout << shape->draw() << '\n';
    }
}
