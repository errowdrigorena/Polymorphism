#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <utility>

import shapes;
import type_erasure;

using ::testing::Eq;
using ::testing::IsFalse;

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
    EXPECT_THAT(Shape_TE{Circle{}}.draw(), Eq("Drawing Circle"));
    EXPECT_THAT(Shape_TE{Triangle{}}.draw(), Eq("Drawing Triangle"));
    EXPECT_THAT(Shape_TE{Rectangle{}}.draw(), Eq("Drawing Rectangle"));
}

TEST(TypeErasureShapesTest, AcceptsNonInheritingDrawableType)
{
    Shape_TE shape{TextShape{"Drawing TextShape"}};

    EXPECT_THAT(shape.draw(), Eq("Drawing TextShape"));
}

TEST(TypeErasureShapesTest, CopiesUnderlyingObjectThroughVTable)
{
    Shape_TE original{TextShape{"Drawing Copy"}};
    Shape_TE copy = original;

    EXPECT_THAT(original.draw(), Eq("Drawing Copy"));
    EXPECT_THAT(copy.draw(), Eq("Drawing Copy"));
}

TEST(TypeErasureShapesTest, MovesUnderlyingObjectThroughVTable)
{
    Shape_TE original{TextShape{"Drawing Move"}};
    Shape_TE moved = std::move(original);

    EXPECT_THAT(original.has_value(), IsFalse());
    EXPECT_THAT(original.draw(), Eq("<empty>"));
    EXPECT_THAT(moved.draw(), Eq("Drawing Move"));
}

TEST(TypeErasureShapesTest, AssignsByCopyReplacingPreviousValue)
{
    const Shape_TE original{TextShape{"Drawing Assigned"}};
    Shape_TE target{TextShape{"Drawing Previous"}};

    target = original;

    EXPECT_THAT(original.draw(), Eq("Drawing Assigned"));
    EXPECT_THAT(target.draw(), Eq("Drawing Assigned"));
}

TEST(TypeErasureShapesTest, AssignsByMoveReplacingPreviousValue)
{
    Shape_TE source{TextShape{"Drawing MoveAssigned"}};
    Shape_TE target{TextShape{"Drawing Previous"}};

    target = std::move(source);

    EXPECT_THAT(source.has_value(), IsFalse());
    EXPECT_THAT(source.draw(), Eq("<empty>"));
    EXPECT_THAT(target.draw(), Eq("Drawing MoveAssigned"));
}

TEST(TypeErasureShapesTest, CopyAssignsFromEmpty)
{
    const Shape_TE empty;
    Shape_TE value{TextShape{"Drawing Kept"}};

    value = empty;

    EXPECT_THAT(value.has_value(), IsFalse());
    EXPECT_THAT(value.draw(), Eq("<empty>"));
}

TEST(TypeErasureShapesTest, CopyAssignsSelf)
{
    Shape_TE self{TextShape{"Drawing Self"}};
    const Shape_TE& same = self;

    self = same;

    EXPECT_THAT(self.draw(), Eq("Drawing Self"));
}

TEST(TypeErasureShapesTest, MoveAssignsFromEmpty)
{
    Shape_TE empty;
    Shape_TE value{TextShape{"Drawing Cleared"}};

    value = std::move(empty);

    EXPECT_THAT(value.has_value(), IsFalse());
    EXPECT_THAT(empty.has_value(), IsFalse());
}

TEST(TypeErasureShapesTest, MoveAssignsSelf)
{
    Shape_TE self{TextShape{"Drawing SelfMove"}};
    Shape_TE& same = self;

    self = std::move(same);

    EXPECT_THAT(self.draw(), Eq("Drawing SelfMove"));
}

TEST(TypeErasureShapesTest, CopiesCircleThroughVTable)
{
    const Shape_TE circle{Circle{}};
    const Shape_TE copy = circle;

    EXPECT_THAT(circle.draw(), Eq("Drawing Circle"));
    EXPECT_THAT(copy.draw(), Eq("Drawing Circle"));
}

TEST(TypeErasureShapesTest, CopiesTriangleThroughVTable)
{
    const Shape_TE triangle{Triangle{}};
    const Shape_TE copy = triangle;

    EXPECT_THAT(triangle.draw(), Eq("Drawing Triangle"));
    EXPECT_THAT(copy.draw(), Eq("Drawing Triangle"));
}

TEST(TypeErasureShapesTest, CopiesRectangleThroughVTable)
{
    const Shape_TE rectangle{Rectangle{}};
    const Shape_TE copy = rectangle;

    EXPECT_THAT(rectangle.draw(), Eq("Drawing Rectangle"));
    EXPECT_THAT(copy.draw(), Eq("Drawing Rectangle"));
}
