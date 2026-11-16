#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <numbers>

using ::testing::Eq;

float circle_area();
float circle_perimeter();
float square_area();
float square_perimeter();
float triangle_area();
float triangle_perimeter();
float dot_area();
float copied_circle_area();
float moved_square_area();
float copy_assigned_area();
float move_assigned_perimeter();
float pure_model_value();
bool implemented_model_is_rejected();

namespace {

constexpr float pi = std::numbers::pi_v<float>;

} // namespace

TEST(ReflectionShapesTest, ReportsCircleAreaAndPerimeter)
{
    EXPECT_THAT(circle_area(), Eq(pi));
    EXPECT_THAT(circle_perimeter(), Eq(2.0f * pi));
}

TEST(ReflectionShapesTest, ReportsSquareAreaAndPerimeter)
{
    EXPECT_THAT(square_area(), Eq(4.0f));
    EXPECT_THAT(square_perimeter(), Eq(8.0f));
}

TEST(ReflectionShapesTest, ReportsTriangleAreaAndPerimeter)
{
    EXPECT_THAT(triangle_area(), Eq(6.0f));
    EXPECT_THAT(triangle_perimeter(), Eq(12.0f));
}

TEST(ReflectionShapesTest, AcceptsATypeDefinedOutsideTheModule)
{
    EXPECT_THAT(dot_area(), Eq(1.0f));
}

TEST(ReflectionShapesTest, AcceptsAnInterfaceOfPureVirtualMethods)
{
    EXPECT_THAT(pure_model_value(), Eq(7.0f));
}

TEST(ReflectionShapesTest, RejectsAMethodThatHasABody)
{
    EXPECT_TRUE(implemented_model_is_rejected());
}

TEST(ReflectionShapesTest, CopiesTheStoredShape)
{
    EXPECT_THAT(copied_circle_area(), Eq(pi));
}

TEST(ReflectionShapesTest, MovesTheStoredShape)
{
    EXPECT_THAT(moved_square_area(), Eq(4.0f));
}

TEST(ReflectionShapesTest, CopyAssignsTheStoredShape)
{
    EXPECT_THAT(copy_assigned_area(), Eq(pi));
}

TEST(ReflectionShapesTest, MoveAssignsTheStoredShape)
{
    EXPECT_THAT(move_assigned_perimeter(), Eq(12.0f));
}
