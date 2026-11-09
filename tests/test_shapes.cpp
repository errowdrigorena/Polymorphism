#include <gtest/gtest.h>

import shapes;

TEST(ShapesTest, CircleDraw)
{
    Circle circle;
    EXPECT_EQ(circle.draw(), "Drawing Circle");
}

TEST(ShapesTest, TriangleDraw)
{
    Triangle triangle;
    EXPECT_EQ(triangle.draw(), "Drawing Triangle");
}

TEST(ShapesTest, RectangleDraw)
{
    Rectangle rectangle;
    EXPECT_EQ(rectangle.draw(), "Drawing Rectangle");
}
