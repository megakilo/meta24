#include <array>
#include <iostream>
#include <optional>
#include <type_traits>

enum class OpType { Add, Substract, Multiply, Divide };

template <typename Index>
struct Value {
  template <std::size_t N>
  static double eval(const std::array<double, N>& a) {
    static_assert(Index::value < N);
    return a[Index::value];
  }

  template <std::size_t N>
  static std::string print(const std::array<double, N>& a,
                           const bool is_denominator = false) {
    return std::to_string(int(a[Index::value]));
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
