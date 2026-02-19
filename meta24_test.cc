#include "meta24.h"

#include <gtest/gtest.h>

TEST(Calc24Test, Basic3Numbers) {
  const auto result = calc24(std::array<double, 3> {2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24Test, Invalid3Numbers) {
  const auto result = calc24(std::array<double, 3> {1, 3, 4});
  EXPECT_FALSE(result.has_value());
}

TEST(Calc24Test, Basic4Numbers) {
  const auto result = calc24(std::array<double, 4> {1, 2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

// 7 * (3 + 3 / 7) = 24
TEST(Calc24Test, FractionalCalc) {
  const auto result = calc24(std::array<double, 4> {3, 3, 7, 7});
  EXPECT_TRUE(result.has_value());
}