#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <utility>

using ::testing::Eq;
using ::testing::IsFalse;

std::string erased_circle_draw();
std::string erased_triangle_draw();
std::string erased_rectangle_draw();
std::string erased_text_shape_draw();
std::pair<std::string, std::string> copy_text_shape();
std::tuple<bool, std::string, std::string> move_text_shape();
std::pair<std::string, std::string> copy_assign_text_shape();
std::tuple<bool, std::string, std::string> move_assign_text_shape();
std::pair<bool, std::string> copy_assign_from_empty();
std::string copy_assign_self();
std::pair<bool, bool> move_assign_from_empty();
std::string move_assign_self();
std::pair<std::string, std::string> copy_circle();
std::pair<std::string, std::string> copy_triangle();
std::pair<std::string, std::string> copy_rectangle();

TEST(TypeErasureShapesTest, DrawDispatchesTraditionalShapes)
{
    EXPECT_THAT(erased_circle_draw(), Eq("Drawing Circle"));
    EXPECT_THAT(erased_triangle_draw(), Eq("Drawing Triangle"));
    EXPECT_THAT(erased_rectangle_draw(), Eq("Drawing Rectangle"));
}

TEST(TypeErasureShapesTest, AcceptsNonInheritingDrawableType)
{
    EXPECT_THAT(erased_text_shape_draw(), Eq("Drawing TextShape"));
}

TEST(TypeErasureShapesTest, CopiesUnderlyingObjectThroughVTable)
{
    const auto [original, copy] = copy_text_shape();

    EXPECT_THAT(original, Eq("Drawing Copy"));
    EXPECT_THAT(copy, Eq("Drawing Copy"));
}

TEST(TypeErasureShapesTest, MovesUnderlyingObjectThroughVTable)
{
    const auto [original_has_value, original, moved] = move_text_shape();

    EXPECT_THAT(original_has_value, IsFalse());
    EXPECT_THAT(original, Eq("<empty>"));
    EXPECT_THAT(moved, Eq("Drawing Move"));
}

TEST(TypeErasureShapesTest, AssignsByCopyReplacingPreviousValue)
{
    const auto [original, target] = copy_assign_text_shape();

    EXPECT_THAT(original, Eq("Drawing Assigned"));
    EXPECT_THAT(target, Eq("Drawing Assigned"));
}

TEST(TypeErasureShapesTest, AssignsByMoveReplacingPreviousValue)
{
    const auto [source_has_value, source, target] = move_assign_text_shape();

    EXPECT_THAT(source_has_value, IsFalse());
    EXPECT_THAT(source, Eq("<empty>"));
    EXPECT_THAT(target, Eq("Drawing MoveAssigned"));
}

TEST(TypeErasureShapesTest, CopyAssignsFromEmpty)
{
    const auto [has_value, value] = copy_assign_from_empty();

    EXPECT_THAT(has_value, IsFalse());
    EXPECT_THAT(value, Eq("<empty>"));
}

TEST(TypeErasureShapesTest, CopyAssignsSelf)
{
    EXPECT_THAT(copy_assign_self(), Eq("Drawing Self"));
}

TEST(TypeErasureShapesTest, MoveAssignsFromEmpty)
{
    const auto [value_has_value, empty_has_value] = move_assign_from_empty();

    EXPECT_THAT(value_has_value, IsFalse());
    EXPECT_THAT(empty_has_value, IsFalse());
}

TEST(TypeErasureShapesTest, MoveAssignsSelf)
{
    EXPECT_THAT(move_assign_self(), Eq("Drawing SelfMove"));
}

TEST(TypeErasureShapesTest, CopiesCircleThroughVTable)
{
    const auto [circle, copy] = copy_circle();

    EXPECT_THAT(circle, Eq("Drawing Circle"));
    EXPECT_THAT(copy, Eq("Drawing Circle"));
}

TEST(TypeErasureShapesTest, CopiesTriangleThroughVTable)
{
    const auto [triangle, copy] = copy_triangle();

    EXPECT_THAT(triangle, Eq("Drawing Triangle"));
    EXPECT_THAT(copy, Eq("Drawing Triangle"));
}

TEST(TypeErasureShapesTest, CopiesRectangleThroughVTable)
{
    const auto [rectangle, copy] = copy_rectangle();

    EXPECT_THAT(rectangle, Eq("Drawing Rectangle"));
    EXPECT_THAT(copy, Eq("Drawing Rectangle"));
}
