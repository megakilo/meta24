#include <array>
#include <iostream>
#include <optional>
#include <type_traits>

#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/bind.hpp"
#include "boost/mp11/function.hpp"
#include "boost/mp11/list.hpp"
#include "boost/mp11/utility.hpp"

namespace mp11 = ::boost::mp11;

template <typename T, int N>
using mp_plus_c = mp11::mp_plus<T, mp11::mp_int<N>>;

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

template <typename OP1, typename OP2>
struct Combine {
  using type = mp11::mp_list<Expression<OP1, OP2, OpType::Add>,
                             Expression<OP1, OP2, OpType::Substract>,
                             Expression<OP2, OP1, OpType::Substract>,
                             Expression<OP1, OP2, OpType::Multiply>,
                             Expression<OP1, OP2, OpType::Divide>,
                             Expression<OP2, OP1, OpType::Divide>>;
};

template <typename L, typename I = mp11::mp_int<0>,
          typename J = mp11::mp_int<0>,
          typename N = mp11::mp_int<mp11::mp_size<L>::value>>
struct Calc {
  static_assert(mp11::mp_less<I, J>::value);
  using rest =
      mp11::mp_erase<mp11::mp_erase<L, J, mp_plus_c<J, 1>>, I, mp_plus_c<I, 1>>;

  using combined = typename Combine<mp11::mp_at<L, I>, mp11::mp_at<L, J>>::type;
  using reduced_list = mp11::mp_apply<
      mp11::mp_append,
      mp11::mp_transform_q<
          mp11::mp_quote_trait<Calc>,
          mp11::mp_transform_q<mp11::mp_bind_front<mp11::mp_push_front, rest>,
                               combined>>>;

  using next_type = typename Calc<L, I, mp_plus_c<J, 1>, N>::type;
  using type = mp11::mp_append<reduced_list, next_type>;
};

template <typename L, typename I, typename N>
struct Calc<L, I, I, N> {
  using type =
      typename mp11::mp_eval_if_c<N::value == 1, mp11::mp_identity<L>, Calc, L,
                                  I, mp_plus_c<I, 1>, N>::type;
};

template <typename L, typename I, typename N>
struct Calc<L, I, N, N> {
  using type = typename Calc<L, mp_plus_c<I, 1>, mp_plus_c<I, 2>, N>::type;
};

template <typename L, typename N>
struct Calc<L, mp_plus_c<N, -1>, N, N> {
  using type = mp11::mp_list<>;
};

template <int N>
using ValueList = mp11::mp_transform<Value, mp11::mp_iota<mp11::mp_int<N>>>;

template <typename L, std::size_t N, std::size_t I = 0>
std::optional<std::string> calc24(const std::array<double, N>& a) {
  using E = mp11::mp_at_c<L, I>;
  if (E::eval(a) == 24) {
    // std::cout << typeid(std::declval<E>()).name() << std::endl;
    return E::print(a);
  }
  if constexpr (I + 1 < mp11::mp_size<L>::value) {
    return calc24<L, N, I + 1>(a);
  } else {
    return {};
  }
}

int main() {
  srand(time(NULL));

  constexpr int N = 4;
  using all_calculations = Calc<ValueList<N>>::type;

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
