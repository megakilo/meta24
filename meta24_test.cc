#include "meta24.h"

#include <gtest/gtest.h>

TEST(Calc24Test, Basic3Numbers) {
  constexpr int N = 3;
  const auto result = calc24<Calc<ValueList<N>>::type>(std::array<double, N> {2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24Test, Basic4Numbers) {
  constexpr int N = 4;
  const auto result = calc24<Calc<ValueList<N>>::type>(std::array<double, N> {1, 2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24Test, Invalid3Numbers) {
  constexpr int N = 3;
  const auto result = calc24<Calc<ValueList<N>>::type>(std::array<double, N> {1, 3, 4});
  EXPECT_FALSE(result.has_value());
}

// 7 * (3 + 3 / 7) = 24
TEST(Calc24Test, FractionalCalc) {
  constexpr int N = 4;
  const auto result = calc24<Calc<ValueList<N>>::type>(std::array<double, N> {3, 3, 7, 7});
  EXPECT_TRUE(result.has_value());
}