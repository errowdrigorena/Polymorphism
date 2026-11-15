#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

using ::testing::Eq;

std::string circle_draw();
std::string copied_circle_draw();
std::string triangle_draw_through_base();
std::string triangle_draw();
std::string rectangle_draw();

TEST(ShapesTest, CircleDraw)
{
    EXPECT_THAT(circle_draw(), Eq("Drawing Circle"));
}

TEST(ShapesTest, CopiesConcreteShape)
{
    EXPECT_THAT(copied_circle_draw(), Eq("Drawing Circle"));
}

TEST(ShapesTest, DestroysThroughBase)
{
    EXPECT_THAT(triangle_draw_through_base(), Eq("Drawing Triangle"));
}

TEST(ShapesTest, TriangleDraw)
{
    EXPECT_THAT(triangle_draw(), Eq("Drawing Triangle"));
}

TEST(ShapesTest, RectangleDraw)
{
    EXPECT_THAT(rectangle_draw(), Eq("Drawing Rectangle"));
}
