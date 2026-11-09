#include <gtest/gtest.h>
#include <string>
#include <utility>

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

TEST(TypeErasureShapesTest, DrawDispatchesTraditionalShapes)
{
    EXPECT_EQ(Shape_TE{Circle{}}.draw(), "Drawing Circle");
    EXPECT_EQ(Shape_TE{Triangle{}}.draw(), "Drawing Triangle");
    EXPECT_EQ(Shape_TE{Rectangle{}}.draw(), "Drawing Rectangle");
}

TEST(TypeErasureShapesTest, AcceptsNonInheritingDrawableType)
{
    Shape_TE shape{TextShape{"Drawing TextShape"}};

    EXPECT_EQ(shape.draw(), "Drawing TextShape");
}

TEST(TypeErasureShapesTest, CopiesUnderlyingObjectThroughVTable)
{
    Shape_TE original{TextShape{"Drawing Copy"}};
    Shape_TE copy = original;

    EXPECT_EQ(original.draw(), "Drawing Copy");
    EXPECT_EQ(copy.draw(), "Drawing Copy");
}

TEST(TypeErasureShapesTest, MovesUnderlyingObjectThroughVTable)
{
    Shape_TE original{TextShape{"Drawing Move"}};
    Shape_TE moved = std::move(original);

    EXPECT_FALSE(original.has_value());
    EXPECT_EQ(original.draw(), "<empty>");
    EXPECT_EQ(moved.draw(), "Drawing Move");
}
