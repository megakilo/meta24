#include <array>
#include <iostream>
#include <optional>
#include <type_traits>

#include "common.h"
#include "metal.hpp"

namespace mtl = metal;

template <typename OP1, typename OP2> struct Combine {
  using type = mtl::list<Expression<OP1, OP2, OpType::Add>,
                         Expression<OP1, OP2, OpType::Substract>,
                         Expression<OP2, OP1, OpType::Substract>,
                         Expression<OP1, OP2, OpType::Multiply>,
                         Expression<OP1, OP2, OpType::Divide>,
                         Expression<OP2, OP1, OpType::Divide>>;
};

template <typename L, typename I = mtl::number<0>, typename J = mtl::number<0>>
struct Build {
  static_assert(mtl::less<I, J>::value);

  using rest = mtl::erase<mtl::erase<L, J, mtl::inc<J>>, I, mtl::inc<I>>;
  using combined = mtl::eval<Combine<mtl::at<L, I>, mtl::at<L, J>>>;
  using append_lambda =
      mtl::bind<mtl::lambda<mtl::append>, mtl::always<rest>, mtl::_1>;
  using reduced_result =
      mtl::flatten<mtl::transform<mtl::lazy<Build>,
                                  mtl::transform<append_lambda, combined>>>;
  using next_result = mtl::eval<Build<L, I, mtl::inc<J>>>;
  using type = mtl::join<reduced_result, next_result>;
};

template <typename L> struct Build<L, mtl::dec<mtl::size<L>>, mtl::size<L>> {
  using type = mtl::list<>;
};

template <typename L, typename I> struct Build<L, I, mtl::size<L>> {
  using N = mtl::size<L>;
  static_assert(mtl::less<I, N>::value);
  using type = mtl::eval<Build<L, mtl::inc<I>, mtl::add<I, mtl::number<2>>>>;
};

template <typename L, typename I> struct Build<L, I, I> {
  using N = mtl::size<L>;
  static_assert(mtl::less<I, N>::value);
  using type = mtl::if_<mtl::number<N::value == 1>, L,
                        mtl::eval<Build<L, I, mtl::inc<I>>>>;
};

template <typename L, std::size_t N, typename I = mtl::number<0>>
std::optional<std::string> calc24_impl(const std::array<double, N> &a) {
  using E = mtl::at<L, I>;
  if (E::eval(a) == 24) {
    return E::print(a);
  }
  if constexpr (mtl::less<mtl::inc<I>, mtl::size<L>>::value) {
    return calc24_impl<L, N, mtl::inc<I>>(a);
  } else {
    return {};
  }
}

template <std::size_t N>
std::optional<std::string> calc24(const std::array<double, N> &a) {
  using IndexList = mtl::transform<mtl::lambda<Value>,
                                   mtl::iota<mtl::number<0>, mtl::number<N>>>;
  return calc24_impl<mtl::eval<Build<IndexList>>>(a);
}