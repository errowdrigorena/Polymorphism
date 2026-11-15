export module type_erasure:drawable;

import std;

export template <typename T>
concept DrawableShape_TE = requires(const T& shape) {
    { shape.draw() } -> std::convertible_to<std::string>;
};
