#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

// ============================================================================
// meta24_constexpr.h — C++20 constexpr metaprogramming rewrite of meta24.h
//
// All expression-tree enumeration happens at compile time via constexpr
// functions. No Boost.Mp11 or template-type-list machinery is needed.
//
// Expression trees are represented as flat node arrays: each node is either
// a leaf (referencing an input index) or a binary op (referencing two child
// nodes). No stack is used for evaluation or printing.
// ============================================================================

// ---------------------------------------------------------------------------
// Operator enum
// ---------------------------------------------------------------------------
enum class Op : uint8_t { Add, Sub, Mul, Div };

// ---------------------------------------------------------------------------
// Expression node: either a leaf (input value) or a binary operation.
// ---------------------------------------------------------------------------
struct ExprNode {
  bool is_leaf = true;
  Op op = Op::Add;       // meaningful only when !is_leaf
  uint8_t input = 0;     // meaningful only when is_leaf (input index 0–3)
  uint8_t left = 0;      // meaningful only when !is_leaf (child index)
  uint8_t right = 0;     // meaningful only when !is_leaf (child index)
};

// ---------------------------------------------------------------------------
// ExprTree: a complete expression tree stored as a flat node array.
//
// For N inputs we have at most N leaves + (N-1) interior nodes = 2N-1 nodes.
// Max N=4 → max 7 nodes.
// ---------------------------------------------------------------------------
constexpr std::size_t kMaxNodes = 7;

struct ExprTree {
  std::array<ExprNode, kMaxNodes> nodes{};
  uint8_t count = 0;   // number of nodes used
  uint8_t root = 0;    // index of the root node
};

// ---------------------------------------------------------------------------
// Counting expression trees
//
// count(1) = 1
// count(N) = C(N,2) * 6 * count(N-1)
// ---------------------------------------------------------------------------
constexpr std::size_t count_programs(std::size_t n) {
  if (n <= 1) return 1;
  return (n * (n - 1) / 2) * 6 * count_programs(n - 1);
}

// ---------------------------------------------------------------------------
// Compile-time generation
//
// Strategy: mirror the original algorithm's recursive "pick any pair,
// combine with all 6 op-variants, recurse on the reduced list" approach.
//
// An Item is a partial ExprTree (subtree). Initially each item is a single
// leaf node. At each level we pick two items, combine them under a binary
// op (6 variants: a+b, a-b, b-a, a*b, a/b, b/a), and recurse.
// When one item remains, we have a complete ExprTree.
// ---------------------------------------------------------------------------

namespace detail {

// A partial subtree used during generation.
struct Item {
  std::array<ExprNode, kMaxNodes> nodes{};
  uint8_t count = 0;  // how many nodes in this subtree
  uint8_t root = 0;   // root node index within this subtree

  constexpr Item() = default;

  // Construct a leaf item for input index `i`.
  constexpr explicit Item(uint8_t input_idx) : count(1), root(0) {
    nodes[0].is_leaf = true;
    nodes[0].input = input_idx;
  }
};

// Combine two subtrees under a binary operator to form a new subtree.
constexpr Item combine(const Item& a, const Item& b, Op op) {
  Item result;
  // Copy nodes from a.
  for (uint8_t i = 0; i < a.count; ++i) {
    result.nodes[result.count] = a.nodes[i];
    // Remap child indices: they're already relative to 0, and we place 'a'
    // starting at index 0, so no remapping needed.
    result.count++;
  }
  uint8_t b_offset = result.count;
  // Copy nodes from b, remapping child indices.
  for (uint8_t i = 0; i < b.count; ++i) {
    ExprNode n = b.nodes[i];
    if (!n.is_leaf) {
      n.left += b_offset;
      n.right += b_offset;
    }
    result.nodes[result.count++] = n;
  }
  // Add the new binary op node.
  ExprNode op_node;
  op_node.is_leaf = false;
  op_node.op = op;
  op_node.left = a.root;             // root of 'a' subtree (offset 0)
  op_node.right = b.root + b_offset; // root of 'b' subtree (shifted)
  result.nodes[result.count] = op_node;
  result.root = result.count;
  result.count++;
  return result;
}

// Convert a completed Item to an ExprTree.
constexpr ExprTree to_tree(const Item& item) {
  ExprTree t;
  for (uint8_t i = 0; i < item.count; ++i) t.nodes[i] = item.nodes[i];
  t.count = item.count;
  t.root = item.root;
  return t;
}

// The 6 combined ops: a+b, a-b, b-a, a*b, a/b, b/a.
struct CombinedOp {
  bool swap;  // if true, operand order is (b, a) instead of (a, b)
  Op op;
};

constexpr CombinedOp kCombinedOps[] = {
  {false, Op::Add},  // a + b
  {false, Op::Sub},  // a - b
  { true, Op::Sub},  // b - a
  {false, Op::Mul},  // a * b
  {false, Op::Div},  // a / b
  { true, Op::Div},  // b / a
};

// Recursive generation.
template <std::size_t Total>
constexpr void generate(
    std::array<ExprTree, Total>& out,
    std::size_t& idx,
    std::array<Item, 4>& items,  // max N=4
    std::size_t n)
{
  if (n == 1) {
    out[idx++] = to_tree(items[0]);
    return;
  }

  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = i + 1; j < n; ++j) {
      for (const auto& cop : kCombinedOps) {
        Item combined_item;
        if (cop.swap) {
          combined_item = combine(items[j], items[i], cop.op);
        } else {
          combined_item = combine(items[i], items[j], cop.op);
        }

        // Build the reduced item list: remove i and j, add combined.
        std::array<Item, 4> reduced{};
        std::size_t ri = 0;
        for (std::size_t k = 0; k < n; ++k) {
          if (k != i && k != j) reduced[ri++] = items[k];
        }
        reduced[ri++] = combined_item;

        generate(out, idx, reduced, n - 1);
      }
    }
  }
}

}  // namespace detail

// ---------------------------------------------------------------------------
// Generate all expression trees for N inputs at compile time.
// ---------------------------------------------------------------------------
template <std::size_t N>
constexpr auto generate_programs() {
  constexpr std::size_t Total = count_programs(N);
  std::array<ExprTree, Total> trees{};

  std::array<detail::Item, 4> items{};
  for (std::size_t i = 0; i < N; ++i) {
    items[i] = detail::Item{static_cast<uint8_t>(i)};
  }

  std::size_t idx = 0;
  detail::generate(trees, idx, items, N);
  return trees;
}

// ---------------------------------------------------------------------------
// Runtime evaluation — direct tree traversal, no stack needed.
// ---------------------------------------------------------------------------
inline double eval_node(const ExprTree& tree, uint8_t node_idx,
                        const double* inputs) {
  const auto& n = tree.nodes[node_idx];
  if (n.is_leaf) return inputs[n.input];

  double a = eval_node(tree, n.left, inputs);
  double b = eval_node(tree, n.right, inputs);

  switch (n.op) {
    case Op::Add: return a + b;
    case Op::Sub: return a - b;
    case Op::Mul: return a * b;
    case Op::Div: return a / b;
  }
  return 0.0;  // unreachable
}

inline double eval_tree(const ExprTree& tree, const double* inputs) {
  return eval_node(tree, tree.root, inputs);
}

// ---------------------------------------------------------------------------
// Runtime printing — reconstruct infix expression with minimal parens.
//
// Precedence: atom = 3, mul/div = 2, add/sub = 1.
// ---------------------------------------------------------------------------
namespace detail {

struct PrintResult {
  std::string text;
  int prec;  // 1 = add/sub, 2 = mul/div, 3 = atom
};

inline std::string parenthesize(const PrintResult& r) {
  return "(" + r.text + ")";
}

template <std::size_t N>
PrintResult print_node(const ExprTree& tree, uint8_t node_idx,
                       const std::array<double, N>& inputs) {
  const auto& n = tree.nodes[node_idx];
  if (n.is_leaf) {
    return {std::to_string(static_cast<int>(inputs[n.input])), 3};
  }

  auto a = print_node(tree, n.left, inputs);
  auto b = print_node(tree, n.right, inputs);

  std::string result;
  int prec;

  switch (n.op) {
    case Op::Add:
      prec = 1;
      result = a.text + " + " + b.text;
      break;
    case Op::Sub:
      prec = 1;
      result = a.text + " - " +
               (b.prec <= 1 ? parenthesize(b) : b.text);
      break;
    case Op::Mul:
      prec = 2;
      result = (a.prec < 2 ? parenthesize(a) : a.text) + " * " +
               (b.prec < 2 ? parenthesize(b) : b.text);
      break;
    case Op::Div:
      prec = 2;
      result = (a.prec < 2 ? parenthesize(a) : a.text) + " / " +
               (b.prec <= 2 ? parenthesize(b) : b.text);
      break;
  }
  return {std::move(result), prec};
}

}  // namespace detail

template <std::size_t N>
std::string print_tree(const ExprTree& tree,
                       const std::array<double, N>& inputs) {
  return detail::print_node(tree, tree.root, inputs).text;
}

// ---------------------------------------------------------------------------
// Public API — matches the original calc24() signature.
// ---------------------------------------------------------------------------
template <std::size_t N>
std::optional<std::string> calc24(const std::array<double, N>& a) {
  // Expression trees are generated at compile time.
  constexpr auto trees = generate_programs<N>();

  for (const auto& tree : trees) {
    double result = eval_tree(tree, a.data());
    if (std::abs(result - 24.0) < 1e-9) {
      return print_tree(tree, a);
    }
  }
  return std::nullopt;
}
