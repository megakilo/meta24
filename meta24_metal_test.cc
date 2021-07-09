#include "meta24_metal.h"

#include <boost/type_index.hpp>
#include <gtest/gtest.h>

namespace mtl = metal;

TEST(Calc24MetalTest, Basic) {
  using T = IndexList<2>;
  using L = Build<T>::type;
  // EXPECT_TRUE(false) << boost::typeindex::type_id<L>().pretty_name();
  EXPECT_EQ(metal::number<5>::value, 5);
  using result = mtl::less<mtl::number<3>, mtl::number<4>>;
  EXPECT_TRUE(result::value);
}

TEST(Calc24Test, Basic3Numbers) {
  constexpr int N = 3;
  const auto result = calc24<Build<IndexList<N>>::type>(std::array<double, N> {2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24Test, Basic4Numbers) {
  constexpr int N = 4;
  const auto result = calc24<Build<IndexList<N>>::type>(std::array<double, N> {1, 2, 3, 4});
  EXPECT_TRUE(result.has_value());
}

TEST(Calc24Test, Invalid3Numbers) {
  constexpr int N = 3;
  const auto result = calc24<Build<IndexList<N>>::type>(std::array<double, N> {1, 3, 4});
  EXPECT_FALSE(result.has_value());
}

// 7 * (3 + 3 / 7) = 24
TEST(Calc24Test, FractionalCalc) {
  constexpr int N = 4;
  const auto result = calc24<Build<IndexList<N>>::type>(std::array<double, N> {3, 3, 7, 7});
  EXPECT_TRUE(result.has_value());
}