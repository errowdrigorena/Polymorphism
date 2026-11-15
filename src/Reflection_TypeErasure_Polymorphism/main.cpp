import std;
import reflection_type_erasure;

int main()
{
    AnyShape shapes[] = {
        Circle{1.0f},
        Square{2.0f},
        Triangle{3.0f, 4.0f, 5.0f},
    };

    for (auto& shape : shapes) {
        shape.draw();
        std::cout << "  area      = " << shape.area() << '\n';
        std::cout << "  perimeter = " << shape.perimeter() << '\n';
        std::cout << '\n';
    }
}
