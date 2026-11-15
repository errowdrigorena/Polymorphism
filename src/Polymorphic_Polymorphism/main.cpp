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

    // A copy owns a new derived object. With std::shared_ptr both would point at one.
    const PolyShape copy = shapes.front();
    std::cout << "Copied: " << copy->draw() << '\n';
    std::cout << "Same object as the original? " << std::boolalpha
              << (&*copy == &*shapes.front()) << '\n';

    // With std::unique_ptr this line does not compile.
    std::vector<PolyShape> backup = shapes;
    std::cout << "Backup holds " << backup.size() << " shapes\n";

    // Assignment replaces the derived object. A pointer would only change its target.
    shapes.front() = shapes.back();
    std::cout << "After assignment: " << shapes.front()->draw() << '\n';
    std::cout << "Backup still has: " << backup.front()->draw() << '\n';

    // Moving leaves the source valueless, the way a moved unique_ptr is null.
    PolyShape moved = std::move(shapes.back());
    std::cout << "Moved: " << moved->draw() << '\n';
    std::cout << "Source valueless? " << shapes.back().valueless_after_move() << '\n';
}
