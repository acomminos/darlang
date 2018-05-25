#ifndef DARLANG_SRC_AST_UTIL_H_
#define DARLANG_SRC_AST_UTIL_H_

#include <cassert>

#include "ast/types.h"

// Utilities for processing a Darlang AST.

namespace darlang {
namespace ast {

// A visitor that produces a result from an AST node.
template <typename T>
class Reducer : protected Visitor {
 public:
  Reducer() : set_(false) {}

  T&& Reduce(Node& node) {
    set_ = false;
    node.Visit(*this);
    assert(set_);
    return std::move(result_);
  }

 protected:
  // Must be invoked by implementations in their visitor methods to produce a
  // valid result.
  void set_result(T val) {
    assert(!set_); // enforce idempotency
    result_ = std::move(val);
    set_ = true;
  }

 private:
  bool set_;
  T result_;
};

// A visitor that associated each node with a value.
template <typename T>
class Annotator : protected Reducer<T> {
 public:
  // Creates a new annotator, storing results into the given map.
  Annotator(std::unordered_map<NodeID, T>& map) : map_(map) {}

  T& Annotate(Node& node) {
    return map_[node.id] = this->Reduce(node);
  }

  std::unordered_map<NodeID, T>& map() { return map_; }

 private:
  std::unordered_map<NodeID, T>& map_;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_UTIL_H_
