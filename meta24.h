#include <array>
#include <iostream>
#include <optional>
#include <type_traits>

#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/bind.hpp"
#include "boost/mp11/function.hpp"
#include "boost/mp11/list.hpp"
#include "boost/mp11/utility.hpp"
#include "common.h"

namespace mp11 = ::boost::mp11;

template <typename T, int N>
using mp_plus_c = mp11::mp_size_t<T::value + N>;

template <typename OP1, typename OP2>
struct Combine {
  using type = mp11::mp_list<Expression<OP1, OP2, OpType::Add>,
                             Expression<OP1, OP2, OpType::Substract>,
                             Expression<OP2, OP1, OpType::Substract>,
                             Expression<OP1, OP2, OpType::Multiply>,
                             Expression<OP1, OP2, OpType::Divide>,
                             Expression<OP2, OP1, OpType::Divide>>;
};

template <typename L, typename I = mp11::mp_size_t<0>,
          typename J = mp11::mp_size_t<0>>
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

  using next_list = typename Calc<L, I, mp_plus_c<J, 1>>::type;

  using type = mp11::mp_append<reduced_list, next_list>;
};

// Terminal state.
template <typename L>
struct Calc<L, mp_plus_c<mp11::mp_size<L>, -1>, mp11::mp_size<L>> {
  using type = mp11::mp_list<>;
};

// Move to next I.
template <typename L, typename I>
struct Calc<L, I, mp11::mp_size<L>> {
  using type = typename Calc<L, mp_plus_c<I, 1>, mp_plus_c<I, 2>>::type;
};

// Skip if I == J.
template <typename L, typename I>
struct Calc<L, I, I> {
  using type = typename mp11::mp_eval_if_c<mp11::mp_size<L>::value == 1,
                                           mp11::mp_identity<L>, Calc, L, I,
                                           mp_plus_c<I, 1>>::type;
};

template <int N>
using ValueList = mp11::mp_transform<Value, mp11::mp_iota_c<N>>;

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
