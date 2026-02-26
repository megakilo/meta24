#include "meta24_hana.h"

#include <gtest/gtest.h>

TEST(Calc24HanaTest, Basic3Numbers) {
  const auto result = calc24(std::array<double, 3>{2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24HanaTest, Invalid3Numbers) {
  const auto result = calc24(std::array<double, 3>{1, 3, 4});
  EXPECT_FALSE(result.has_value());
}

TEST(Calc24HanaTest, Basic4Numbers) {
  const auto result = calc24(std::array<double, 4>{1, 2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

// 7 * (3 + 3 / 7) = 24
TEST(Calc24HanaTest, FractionalCalc) {
  const auto result = calc24(std::array<double, 4>{3, 3, 7, 7});
  EXPECT_TRUE(result.has_value());
}
