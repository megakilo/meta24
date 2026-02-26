#pragma once

#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "boost/hana.hpp"

namespace hana = boost::hana;

// ============================================================================
// meta24_hana.h — Boost.Hana metaprogramming rewrite of meta24.h
//
// All expression-tree enumeration happens at compile time using Hana's
// heterogeneous type-level containers and algorithms.  The runtime types
// (Value, Expression) are identical to the original.
//
// Architecture mirrors the original mp11 version:
//   - Combine<A,B>       → 6 expression types from two operands
//   - Reduce<L,I,J>      → pick pair (i,j), combine, recurse on rest
//   - Build<L>           → enumerate all pair-selections recursively
//   - calc24()           → iterate generated expressions with hana::for_each
// ============================================================================

// ---------------------------------------------------------------------------
// OpType enum
// ---------------------------------------------------------------------------
enum class OpType { Add, Substract, Multiply, Divide };

// ---------------------------------------------------------------------------
// Value<Index> — leaf: reads input[Index::value]
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Expression<OP1, OP2, OpType> — binary operation node
// ---------------------------------------------------------------------------
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

// ============================================================================
// Compile-time formula generation using Hana type-level operations
// ============================================================================

// ---------------------------------------------------------------------------
// concat_tuples — concatenate N hana::tuples into one (hana::concat is binary)
// ---------------------------------------------------------------------------
template <typename... Tuples>
struct ConcatTuples;

template <>
struct ConcatTuples<> {
  using type = hana::tuple<>;
};

template <typename T>
struct ConcatTuples<T> {
  using type = T;
};

template <typename T, typename U, typename... Rest>
struct ConcatTuples<T, U, Rest...> {
  using type = typename ConcatTuples<
      decltype(hana::concat(std::declval<T>(), std::declval<U>())),
      Rest...>::type;
};

template <typename... Ts>
using concat_tuples_t = typename ConcatTuples<Ts...>::type;

// ---------------------------------------------------------------------------
// Combine<OP1, OP2> — produce 6 expression types from two operand types:
//   a+b, a-b, b-a, a*b, a/b, b/a
// ---------------------------------------------------------------------------
template <typename OP1, typename OP2>
struct Combine {
  using type = hana::tuple<
      hana::type<Expression<OP1, OP2, OpType::Add>>,
      hana::type<Expression<OP1, OP2, OpType::Substract>>,
      hana::type<Expression<OP2, OP1, OpType::Substract>>,
      hana::type<Expression<OP1, OP2, OpType::Multiply>>,
      hana::type<Expression<OP1, OP2, OpType::Divide>>,
      hana::type<Expression<OP2, OP1, OpType::Divide>>>;
};

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
template <typename L, typename = void> struct Build;

// ---------------------------------------------------------------------------
// RemoveTwo<Tuple, I, J> — remove elements at compile-time indices I and J
// (I < J) from a hana::tuple of hana::type<>s.
// Remove J first (larger index) to keep I valid.
// ---------------------------------------------------------------------------
template <typename Tuple, typename I, typename J>
struct RemoveTwo {
  using after_j = decltype(hana::remove_at(std::declval<Tuple>(), J{}));
  using type = decltype(hana::remove_at(std::declval<after_j>(), I{}));
};

// ---------------------------------------------------------------------------
// Reduce<L, IndexPair> — pick pair (I, J) from list L, combine into 6
// expressions, prepend each to the remainder, and recurse via Build.
// ---------------------------------------------------------------------------
template <typename L, typename IndexPair>
struct Reduce {
  using I = typename IndexPair::first_type;
  using J = typename IndexPair::second_type;
  static_assert(I::value < J::value);

  // The two elements to combine
  using ElemI = typename decltype(+hana::at(std::declval<L>(), I{}))::type;
  using ElemJ = typename decltype(+hana::at(std::declval<L>(), J{}))::type;

  // Remainder after removing I and J
  using rest = typename RemoveTwo<L, I, J>::type;

  // 6 combined expression types
  using combined = typename Combine<ElemI, ElemJ>::type;

  // For each combined expr, prepend to rest and recursively build.
  template <typename... CombinedTypes>
  struct Expander {
    using type = concat_tuples_t<
        typename Build<
            decltype(hana::prepend(std::declval<rest>(), CombinedTypes{}))
        >::type...>;
  };

  // Unpack combined tuple into Expander's template parameter pack
  template <typename Combined>
  struct UnpackCombined;

  template <typename... Ts>
  struct UnpackCombined<hana::tuple<Ts...>> {
    using type = typename Expander<Ts...>::type;
  };

  using type = typename UnpackCombined<combined>::type;
};

// ---------------------------------------------------------------------------
// IndexPairs<N> — generate all (I, J) with I < J as std::pair types.
// ---------------------------------------------------------------------------
namespace detail {

template <std::size_t N>
struct IndexPairs {
  template <std::size_t I, std::size_t... Js>
  static constexpr auto pairs_for_i(std::index_sequence<Js...>) {
    return std::make_tuple(
        std::pair<hana::size_t<I>, hana::size_t<I + 1 + Js>>{}...);
  }

  template <std::size_t... Is>
  static constexpr auto all_pairs(std::index_sequence<Is...>) {
    return std::tuple_cat(
        pairs_for_i<Is>(std::make_index_sequence<N - Is - 1>{})...);
  }

  using type = decltype(all_pairs(std::make_index_sequence<N>{}));
};

// Helper to construct the initial Values type without lambdas in decltype.
template <typename... Idxs>
auto make_values(hana::tuple<Idxs...>) ->
    hana::tuple<hana::type<Value<typename Idxs::type>>...>;

template <std::size_t N>
using MakeValues = decltype(
    make_values(hana::to_tuple(hana::range_c<std::size_t, 0, N>)));

}  // namespace detail

// ---------------------------------------------------------------------------
// Build<L> — the main recursive enumeration.
//
// General case (size >= 2): for each pair (I, J), call Reduce and concat.
// Base case (size < 2): return L as-is.
// ---------------------------------------------------------------------------

// Base case: list with < 2 elements
template <typename L, typename>
struct Build {
  using type = L;
};

// General case: size >= 2
template <typename L>
struct Build<L, std::enable_if_t<(
    decltype(hana::size(std::declval<L>()))::value >= 2)>> {
  static constexpr auto n = decltype(hana::size(std::declval<L>()))::value;

  using pairs_tuple = typename detail::IndexPairs<n>::type;

  template <typename... Pairs>
  struct ReduceAll {
    using type = concat_tuples_t<typename Reduce<L, Pairs>::type...>;
  };

  template <typename PairsTuple>
  struct UnpackPairs;

  template <typename... Pairs>
  struct UnpackPairs<std::tuple<Pairs...>> {
    using type = typename ReduceAll<Pairs...>::type;
  };

  using type = typename UnpackPairs<pairs_tuple>::type;
};

// ---------------------------------------------------------------------------
// calc24 — public API matching the original signature.
//
// Expression types are generated entirely at compile time as a
// hana::tuple of hana::type<Expr>.  At runtime we iterate over them
// with hana::for_each to find one evaluating to 24.
// ---------------------------------------------------------------------------
template <std::size_t N>
std::optional<std::string> calc24(const std::array<double, N>& a) {
  // Build initial Values: hana::tuple<hana::type<Value<size_t<0>>>, ...>
  using Values = detail::MakeValues<N>;

  // Generate all expression trees at compile time
  using Exprs = typename Build<Values>::type;

  std::optional<std::string> result;
  hana::for_each(Exprs{}, [&](auto expr_type) {
    if (result.has_value()) return;
    using E = typename decltype(expr_type)::type;
    if (E::eval(a) == 24) {
      result = E::print(a);
    }
  });
  return result;
}
