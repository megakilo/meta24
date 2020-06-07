#include <array>
#include <iostream>
#include <optional>

#include "boost/hana.hpp"

namespace hana = ::boost::hana;

enum class OpType { Add, Substract, Multiply, Divide };

template <std::size_t Index>
struct Number {
  template <std::size_t N>
  static double eval(const std::array<double, N>& a) {
    static_assert(Index < N);
    return a[Index];
  }

  template <std::size_t N>
  static std::string print(const std::array<double, N>& a,
                           const bool is_denominator = false) {
    return std::to_string(int(a[Index]));
  }
};

template <typename OP1, typename OP2, OpType OP>
struct Expression {
  static constexpr OpType op = OP;
  template <std::size_t N>
  static double eval(const std::array<double, N>& a) {
    if constexpr (op == OpType::Add) {
      return OP1::eval(a) + OP2::eval(a);
    } else if constexpr (op == OpType::Substract) {
      return OP1::eval(a) - OP2::eval(a);
    } else if constexpr (op == OpType::Multiply) {
      return OP1::eval(a) * OP2::eval(a);
    } else {
      return OP1::eval(a) / OP2::eval(a);
    }
  }

  template <std::size_t N>
  static std::string print(const std::array<double, N>& a,
                           const bool is_denominator) {
    std::string s;
    if (op == OpType::Add || op == OpType::Substract || is_denominator) {
      return s.append("(").append(Expression::print(a)).append(")");
    } else {
      return s.append(Expression::print(a));
    }
  }

  template <std::size_t N>
  static std::string print(const std::array<double, N>& a) {
    std::string s;
    if constexpr (op == OpType::Add) {
      s.append(OP1::print(a)).append(" + ").append(OP2::print(a));
    } else if constexpr (op == OpType::Substract) {
      s.append(OP1::print(a)).append(" - ").append(OP2::print(a, false));
    } else if constexpr (op == OpType::Multiply) {
      s.append(OP1::print(a, false)).append(" * ").append(OP2::print(a, false));
    } else {
      s.append(OP1::print(a, false)).append(" / ").append(OP2::print(a, true));
    }
    return s;
  }
};

//------------------------------

template <typename OP1, typename OP2>
constexpr auto CombineNumbers(OP1, OP2) {
  return hana::make_tuple(Expression<OP1, OP2, OpType::Add>{},
                          Expression<OP1, OP2, OpType::Substract>{},
                          Expression<OP2, OP1, OpType::Substract>{},
                          Expression<OP1, OP2, OpType::Multiply>{},
                          Expression<OP1, OP2, OpType::Divide>{},
                          Expression<OP2, OP1, OpType::Divide>{});
}

template <int I, int J, typename... Ts>
constexpr auto GenerateExpressions(const hana::tuple<Ts...>& input) {
  const int N = sizeof...(Ts);
  if constexpr (N == 1) {
    return input;
  } else if constexpr (I == N - 1) {
    return hana::make_tuple();
  } else {
    auto i = hana::size_c<I>;
    auto j = hana::size_c<J>;
    auto rest = hana::remove_at(hana::remove_at(input, j), i);
    auto reduced = hana::transform(
        CombineNumbers(hana::at(input, i), hana::at(input, j)), [&](auto&& x) {
          return GenerateExpressions<0, 1>(hana::append(rest, x));
        });

    if constexpr (J + 1 == N) {
      auto next_result = GenerateExpressions<I + 1, I + 2>(input);
      return hana::concat(hana::flatten(reduced), next_result);
    } else {
      auto next_result = GenerateExpressions<I, J + 1>(input);
      return hana::concat(hana::flatten(reduced), next_result);
    }
  }
}

template <int N>
constexpr auto CreateNumbers() {
  if constexpr (N == 0) {
    return hana::make_tuple();
  } else {
    return hana::append(CreateNumbers<N - 1>(), Number<N - 1>{});
  }
}

template <int N>
constexpr auto GenerateAllExpressions() {
  auto nums = CreateNumbers<N>();
  return GenerateExpressions<0, 1>(nums);
}

template <typename T, std::size_t N, std::size_t I = 0>
std::optional<std::string> calc24(const T& all_expr,
                                  const std::array<double, N>& a) {
  const auto M = hana::size(all_expr);
  const auto& x = hana::at_c<I>(all_expr);
  if (x.eval(a) == 24) {
    // std::cout << typeid(std::declval<E>()).name() << std::endl;
    return x.print(a);
  }
  if constexpr (hana::size_c<I + 1> < M) {
    return calc24<T, N, I + 1>(all_expr, a);
  } else {
    return {};
  }
}

int main() {
  constexpr int N = 4;
  auto all_expr = GenerateAllExpressions<N>();

  srand(time(NULL));
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
    const auto result = calc24(all_expr, nums);
    if (result.has_value()) {
      challenge.append(result.value());
    } else {
      challenge.append("No Solution");
    }
    std::cout << challenge << std::endl;
  }
  return 0;
}
