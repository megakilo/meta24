#ifdef MP11
#include "meta24_mp11.h"
#elif METAL
#include "meta24_metal.h"
#endif

#include <array>
#include <iostream>
#include <optional>

int main() {
  srand(123);

  constexpr int N = 4;
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
    const auto result = calc24(nums);
    if (result.has_value()) {
      challenge.append(result.value());
    } else {
      challenge.append("No Solution");
    }
    std::cout << challenge << std::endl;
  }
  return 0;
}
