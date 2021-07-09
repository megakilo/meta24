#include "meta24_metal.h"

#include <array>
#include <iostream>
#include <optional>

constexpr int N = 4;
using all_calculations = Build<IndexList<N>>::type;

int main() {
  srand(123);

  std::array<double, N> nums;
  for (int i = 0; i < 1000; i++) {
    std::string challenge;
    for (int j = 0; j < N; j++) {
      int x = rand() % 13 + 1;
      nums[j] = x;
      if (j > 0) challenge.append(", ");
      challenge.append(std::to_string(x));
    }
    challenge.append(" -> ");
    const auto result = calc24<all_calculations>(nums);
    if (result.has_value()) {
      challenge.append(result.value());
    } else {
      challenge.append("No Solution");
    }
    std::cout << challenge << std::endl;
  }
  return 0;
}