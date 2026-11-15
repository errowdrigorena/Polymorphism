// GoogleTest's headers and `import std;` cannot share a file with this GCC, so
// the cases that touch the shapes live here and test_shapes.cpp checks the
// results.
import std;
import shapes;

std::string circle_draw()
{
    const Circle circle;
    return circle.draw();
}

std::string copied_circle_draw()
{
    const Circle original;
    const Circle copy = original;
    return copy.draw();
}

std::string triangle_draw_through_base()
{
    const std::unique_ptr<Shape> triangle = std::make_unique<Triangle>();
    return triangle->draw();
}

std::string triangle_draw()
{
    const Triangle triangle;
    return triangle.draw();
}

std::string rectangle_draw()
{
    const Rectangle rectangle;
    return rectangle.draw();
}
