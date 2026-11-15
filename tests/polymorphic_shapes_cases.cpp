// GoogleTest's headers and `import std;` cannot share a file with this GCC, so
// the cases that touch PolyShape live here and test_polymorphic_shapes.cpp
// checks the results.
import std;
import polymorphic;

namespace {

class Label final : public Shape {
public:
    explicit Label(std::string text) : text_(std::move(text))
    {
    }

    std::string draw() const override
    {
        return text_;
    }

private:
    std::string text_;
};

} // namespace

std::string polymorphic_circle_draw()
{
    return PolyShape{std::in_place_type<Circle>}->draw();
}

std::string polymorphic_triangle_draw()
{
    return PolyShape{std::in_place_type<Triangle>}->draw();
}

std::string polymorphic_rectangle_draw()
{
    return PolyShape{std::in_place_type<Rectangle>}->draw();
}

std::vector<std::string> polymorphic_vector_draws()
{
    std::vector<PolyShape> shapes;
    shapes.emplace_back(std::in_place_type<Circle>);
    shapes.emplace_back(std::in_place_type<Triangle>);
    shapes.emplace_back(std::in_place_type<Rectangle>);

    std::vector<std::string> draws;
    for (const auto& shape : shapes) {
        draws.push_back(shape->draw());
    }
    return draws;
}

std::tuple<std::string, std::string, bool> polymorphic_copy_label()
{
    const PolyShape original{std::in_place_type<Label>, "Drawing Copy"};
    const PolyShape copy = original;
    return {original->draw(), copy->draw(), &*original != &*copy};
}

std::pair<bool, std::string> polymorphic_move_label()
{
    PolyShape original{std::in_place_type<Label>, "Drawing Move"};
    const PolyShape moved = std::move(original);
    return {original.valueless_after_move(), moved->draw()};
}

std::pair<std::string, std::string> polymorphic_copy_assign_label()
{
    const PolyShape original{std::in_place_type<Label>, "Drawing Assigned"};
    PolyShape target{std::in_place_type<Circle>};
    target = original;
    return {original->draw(), target->draw()};
}

std::pair<bool, std::string> polymorphic_move_assign_label()
{
    PolyShape source{std::in_place_type<Label>, "Drawing MoveAssigned"};
    PolyShape target{std::in_place_type<Circle>};
    target = std::move(source);
    return {source.valueless_after_move(), target->draw()};
}

std::pair<std::string, std::string> polymorphic_swap()
{
    PolyShape first{std::in_place_type<Circle>};
    PolyShape second{std::in_place_type<Rectangle>};
    first.swap(second);
    return {first->draw(), second->draw()};
}
