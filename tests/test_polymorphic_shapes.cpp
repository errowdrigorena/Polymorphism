#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsTrue;

std::string polymorphic_circle_draw();
std::string polymorphic_triangle_draw();
std::string polymorphic_rectangle_draw();
std::vector<std::string> polymorphic_vector_draws();
std::tuple<std::string, std::string, bool> polymorphic_copy_label();
std::pair<bool, std::string> polymorphic_move_label();
std::pair<std::string, std::string> polymorphic_copy_assign_label();
std::pair<bool, std::string> polymorphic_move_assign_label();
std::pair<std::string, std::string> polymorphic_swap();

TEST(PolymorphicShapesTest, DrawDispatchesTraditionalShapes)
{
    EXPECT_THAT(polymorphic_circle_draw(), Eq("Drawing Circle"));
    EXPECT_THAT(polymorphic_triangle_draw(), Eq("Drawing Triangle"));
    EXPECT_THAT(polymorphic_rectangle_draw(), Eq("Drawing Rectangle"));
}

TEST(PolymorphicShapesTest, StoresShapesByValueInVector)
{
    EXPECT_THAT(polymorphic_vector_draws(),
                ElementsAre("Drawing Circle", "Drawing Triangle", "Drawing Rectangle"));
}

TEST(PolymorphicShapesTest, CopiesTheDerivedObject)
{
    const auto [original, copy, distinct] = polymorphic_copy_label();

    EXPECT_THAT(original, Eq("Drawing Copy"));
    EXPECT_THAT(copy, Eq("Drawing Copy"));
    EXPECT_THAT(distinct, IsTrue());
}

TEST(PolymorphicShapesTest, MoveLeavesTheSourceValueless)
{
    const auto [valueless, moved] = polymorphic_move_label();

    EXPECT_THAT(valueless, IsTrue());
    EXPECT_THAT(moved, Eq("Drawing Move"));
}

TEST(PolymorphicShapesTest, CopyAssignReplacesTheDerivedObject)
{
    const auto [original, target] = polymorphic_copy_assign_label();

    EXPECT_THAT(original, Eq("Drawing Assigned"));
    EXPECT_THAT(target, Eq("Drawing Assigned"));
}

TEST(PolymorphicShapesTest, MoveAssignLeavesTheSourceValueless)
{
    const auto [valueless, target] = polymorphic_move_assign_label();

    EXPECT_THAT(valueless, IsTrue());
    EXPECT_THAT(target, Eq("Drawing MoveAssigned"));
}

TEST(PolymorphicShapesTest, SwapExchangesTheObjects)
{
    const auto [first, second] = polymorphic_swap();

    EXPECT_THAT(first, Eq("Drawing Rectangle"));
    EXPECT_THAT(second, Eq("Drawing Circle"));
}
