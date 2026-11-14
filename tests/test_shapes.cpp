#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

import shapes;

using ::testing::Eq;

TEST(ShapesTest, CircleDraw)
{
    Circle circle;
    EXPECT_THAT(circle.draw(), Eq("Drawing Circle"));
}

TEST(ShapesTest, CopiesConcreteShape)
{
    const Circle original;
    const Circle copy = original;

    EXPECT_THAT(copy.draw(), Eq("Drawing Circle"));
}

TEST(ShapesTest, DestroysThroughBase)
{
    const std::unique_ptr<Shape> triangle = std::make_unique<Triangle>();

    EXPECT_THAT(triangle->draw(), Eq("Drawing Triangle"));
}

TEST(ShapesTest, TriangleDraw)
{
    Triangle triangle;
    EXPECT_THAT(triangle.draw(), Eq("Drawing Triangle"));
}

TEST(ShapesTest, RectangleDraw)
{
    Rectangle rectangle;
    EXPECT_THAT(rectangle.draw(), Eq("Drawing Rectangle"));
}
