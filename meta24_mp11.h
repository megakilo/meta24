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

template <typename OP1, typename OP2> struct Combine {
  using type = mp11::mp_list<Expression<OP1, OP2, OpType::Add>,
                             Expression<OP1, OP2, OpType::Substract>,
                             Expression<OP2, OP1, OpType::Substract>,
                             Expression<OP1, OP2, OpType::Multiply>,
                             Expression<OP1, OP2, OpType::Divide>,
                             Expression<OP2, OP1, OpType::Divide>>;
};

template <typename L> struct Build;

template <typename L, typename IndexPair> struct Reduce {
  using I = mp11::mp_at_c<IndexPair, 0>;
  using J = mp11::mp_at_c<IndexPair, 1>;
  static_assert(mp11::mp_less<I, J>::value);

  template <typename T> using mp_inc = mp11::mp_size_t<T::value + 1>;
  using rest = mp11::mp_erase<mp11::mp_erase<L, J, mp_inc<J>>, I, mp_inc<I>>;
  using combined = typename Combine<mp11::mp_at<L, I>, mp11::mp_at<L, J>>::type;
  using type = mp11::mp_flatten<mp11::mp_transform_q<
      mp11::mp_quote_trait<Build>,
      mp11::mp_transform_q<mp11::mp_bind_front<mp11::mp_push_front, rest>,
                           combined>>>;
};

template <typename L> struct Build {
  template <typename IndexPair>
  using dedup = mp11::mp_not<
      mp11::mp_less<mp11::mp_at_c<IndexPair, 0>, mp11::mp_at_c<IndexPair, 1>>>;

  using indexes = mp11::mp_iota<mp11::mp_size<L>>;
  using index_pairs =
      mp11::mp_remove_if<mp11::mp_product<mp11::mp_list, indexes, indexes>,
                         dedup>;
  using result = mp11::mp_flatten<mp11::mp_transform_q<
      mp11::mp_bind_front_q<mp11::mp_quote_trait<Reduce>, L>, index_pairs>>;
  using type = mp11::mp_if_c < mp11::mp_size<L>::value<2, L, result>;
};

template <typename L, std::size_t N, std::size_t I = 0>
std::optional<std::string> calc24_impl(const std::array<double, N> &a) {
  using E = mp11::mp_at_c<L, I>;
  if (E::eval(a) == 24) {
    return E::print(a);
  }
  if constexpr (I + 1 < mp11::mp_size<L>::value) {
    return calc24_impl<L, N, I + 1>(a);
  } else {
    return {};
  }
}

template <std::size_t N>
std::optional<std::string> calc24(const std::array<double, N> &a) {
  using Values = mp11::mp_transform<Value, mp11::mp_iota_c<N>>;
  return calc24_impl<typename Build<Values>::type>(a);
}