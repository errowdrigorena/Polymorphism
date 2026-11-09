#include <gtest/gtest.h>

#include "Traditional_OOP/Circle.hpp"
#include "Traditional_OOP/Rectangle.hpp"
#include "Traditional_OOP/Triangle.hpp"

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
