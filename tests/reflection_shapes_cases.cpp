// GoogleTest's headers and `import std;` cannot share a file with this GCC, so
// the cases that touch AnyShape live here and test_reflection_shapes.cpp checks
// the results.
import std;
import reflection_type_erasure;

namespace {

struct Dot {
    void draw()
    {
    }
    float area()
    {
        return 1.0f;
    }
    float perimeter()
    {
        return 4.0f;
    }
};

} // namespace

float circle_area()
{
    return AnyShape{Circle{1.0f}}.area();
}

float circle_perimeter()
{
    return AnyShape{Circle{1.0f}}.perimeter();
}

float square_area()
{
    return AnyShape{Square{2.0f}}.area();
}

float square_perimeter()
{
    return AnyShape{Square{2.0f}}.perimeter();
}

float triangle_area()
{
    return AnyShape{Triangle{3.0f, 4.0f, 5.0f}}.area();
}

float triangle_perimeter()
{
    return AnyShape{Triangle{3.0f, 4.0f, 5.0f}}.perimeter();
}

float dot_area()
{
    return AnyShape{Dot{}}.area();
}

float copied_circle_area()
{
    AnyShape shape{Circle{1.0f}};
    const AnyShape copy = shape;
    return copy.area();
}

float moved_square_area()
{
    AnyShape shape{Square{2.0f}};
    const AnyShape moved = std::move(shape);
    return moved.area();
}

float copy_assigned_area()
{
    AnyShape shape{Square{2.0f}};
    const AnyShape circle{Circle{1.0f}};
    shape = circle;
    return shape.area();
}

float move_assigned_perimeter()
{
    AnyShape shape{Square{2.0f}};
    AnyShape triangle{Triangle{3.0f, 4.0f, 5.0f}};
    shape = std::move(triangle);
    return shape.perimeter();
}
