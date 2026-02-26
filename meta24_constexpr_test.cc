#include "meta24_constexpr.h"

#include <gtest/gtest.h>

TEST(Calc24ConstexprTest, Basic3Numbers) {
  const auto result = calc24(std::array<double, 3>{2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24ConstexprTest, Invalid3Numbers) {
  const auto result = calc24(std::array<double, 3>{1, 3, 4});
  EXPECT_FALSE(result.has_value());
}

TEST(Calc24ConstexprTest, Basic4Numbers) {
  const auto result = calc24(std::array<double, 4>{1, 2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

// 7 * (3 + 3 / 7) = 24
TEST(Calc24ConstexprTest, FractionalCalc) {
  const auto result = calc24(std::array<double, 4>{3, 3, 7, 7});
  EXPECT_TRUE(result.has_value());
}
