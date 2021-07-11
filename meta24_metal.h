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

template <typename L> struct Build;

template <typename L, typename IndexPair> struct Reduce {
  using I = mtl::at<IndexPair, mtl::number<0>>;
  using J = mtl::at<IndexPair, mtl::number<1>>;
  static_assert(mtl::less<I, J>::value);

  using rest = mtl::erase<mtl::erase<L, J, mtl::inc<J>>, I, mtl::inc<I>>;
  using combined = mtl::eval<Combine<mtl::at<L, I>, mtl::at<L, J>>>;
  using append_lambda =
      mtl::bind<mtl::lambda<mtl::append>, mtl::always<rest>, mtl::_1>;
  using type =
      mtl::flatten<mtl::transform<mtl::lazy<Build>,
                                  mtl::transform<append_lambda, combined>>>;
};

template <typename L> struct Build {
  template <typename IndexPair>
  using dedup = mtl::not_<mtl::less<mtl::at<IndexPair, mtl::number<0>>,
                                    mtl::at<IndexPair, mtl::number<1>>>>;

  using indexes = mtl::iota<mtl::number<0>, mtl::size<L>>;
  using index_pairs =
      mtl::remove_if<mtl::combine<indexes, mtl::number<2>>, mtl::lambda<dedup>>;
  using result = mtl::flatten<
      mtl::transform<mtl::partial<mtl::lazy<Reduce>, L>, index_pairs>>;
  using type = mtl::if_<mtl::less<mtl::size<L>, mtl::number<2>>, L, result>;
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
  using Values = mtl::transform<mtl::lambda<Value>,
                                mtl::iota<mtl::number<0>, mtl::number<N>>>;
  return calc24_impl<mtl::eval<Build<Values>>>(a);
}