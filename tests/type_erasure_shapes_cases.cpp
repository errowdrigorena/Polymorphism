// GoogleTest's headers and `import std;` cannot share a file with this GCC, so
// the cases that touch Shape_TE live here and test_type_erasure_shapes.cpp
// checks the results.
import std;
import shapes;
import type_erasure;

namespace {

struct TextShape {
    std::string text;

    std::string draw() const
    {
        return text;
    }
};

} // namespace

std::string erased_circle_draw()
{
    return Shape_TE{Circle{}}.draw();
}

std::string erased_triangle_draw()
{
    return Shape_TE{Triangle{}}.draw();
}

std::string erased_rectangle_draw()
{
    return Shape_TE{Rectangle{}}.draw();
}

std::string erased_text_shape_draw()
{
    const Shape_TE shape{TextShape{"Drawing TextShape"}};
    return shape.draw();
}

std::pair<std::string, std::string> copy_text_shape()
{
    const Shape_TE original{TextShape{"Drawing Copy"}};
    const Shape_TE copy = original;
    return {original.draw(), copy.draw()};
}

std::tuple<bool, std::string, std::string> move_text_shape()
{
    Shape_TE original{TextShape{"Drawing Move"}};
    const Shape_TE moved = std::move(original);
    return {original.has_value(), original.draw(), moved.draw()};
}

std::pair<std::string, std::string> copy_assign_text_shape()
{
    const Shape_TE original{TextShape{"Drawing Assigned"}};
    Shape_TE target{TextShape{"Drawing Previous"}};
    target = original;
    return {original.draw(), target.draw()};
}

std::tuple<bool, std::string, std::string> move_assign_text_shape()
{
    Shape_TE source{TextShape{"Drawing MoveAssigned"}};
    Shape_TE target{TextShape{"Drawing Previous"}};
    target = std::move(source);
    return {source.has_value(), source.draw(), target.draw()};
}

std::pair<bool, std::string> copy_assign_from_empty()
{
    const Shape_TE empty;
    Shape_TE value{TextShape{"Drawing Kept"}};
    value = empty;
    return {value.has_value(), value.draw()};
}

std::string copy_assign_self()
{
    Shape_TE self{TextShape{"Drawing Self"}};
    const Shape_TE& same = self;
    self = same;
    return self.draw();
}

std::pair<bool, bool> move_assign_from_empty()
{
    Shape_TE empty;
    Shape_TE value{TextShape{"Drawing Cleared"}};
    value = std::move(empty);
    return {value.has_value(), empty.has_value()};
}

std::string move_assign_self()
{
    Shape_TE self{TextShape{"Drawing SelfMove"}};
    Shape_TE& same = self;
    self = std::move(same);
    return self.draw();
}

std::pair<std::string, std::string> copy_circle()
{
    const Shape_TE circle{Circle{}};
    const Shape_TE copy = circle;
    return {circle.draw(), copy.draw()};
}

std::pair<std::string, std::string> copy_triangle()
{
    const Shape_TE triangle{Triangle{}};
    const Shape_TE copy = triangle;
    return {triangle.draw(), copy.draw()};
}

std::pair<std::string, std::string> copy_rectangle()
{
    const Shape_TE rectangle{Rectangle{}};
    const Shape_TE copy = rectangle;
    return {rectangle.draw(), copy.draw()};
}
