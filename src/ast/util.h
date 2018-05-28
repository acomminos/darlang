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

template <typename T>
using AnnotationMap = std::unordered_map<NodeID, T>;

// A visitor where each method receives a value associated with a node.
template <typename T>
class AnnotatedVisitor : public Visitor {
 public:
  AnnotatedVisitor(AnnotationMap<T>& annotations)
    : annotations_(annotations) {}

  virtual bool Module(ModuleNode& node, T& arg) { return false; }
  virtual bool Declaration(DeclarationNode& node, T& arg) { return false; }
  virtual bool Constant(ConstantNode& node, T& arg) { return false; }
  virtual bool IdExpression(IdExpressionNode& node, T& arg) { return false; }
  virtual bool IntegralLiteral(IntegralLiteralNode& node, T& arg) { return false; }
  virtual bool StringLiteral(StringLiteralNode& node, T& arg) { return false; }
  virtual bool BooleanLiteral(BooleanLiteralNode& node, T& arg) { return false; }
  virtual bool Invocation(InvocationNode& node, T& arg) { return false; }
  virtual bool Guard(GuardNode& node, T& arg) { return false; }
  virtual bool Bind(BindNode& node, T& arg) { return false; }
  virtual bool Tuple(TupleNode& node, T& arg) { return false; }

  AnnotationMap<T>& annotations() const { return annotations_; }

 private:
  bool Module(ModuleNode& node) override final {
    return Module(node, annotations_[node.id]);
  }

  bool Declaration(DeclarationNode& node) override final {
    return Declaration(node, annotations_[node.id]);
  }

  bool Constant(ConstantNode& node) override final {
    return Constant(node, annotations_[node.id]);
  }

  bool IdExpression(IdExpressionNode& node) override final {
    return IdExpression(node, annotations_[node.id]);
  }

  bool IntegralLiteral(IntegralLiteralNode& node) override final {
    return IntegralLiteral(node, annotations_[node.id]);
  }

  bool StringLiteral(StringLiteralNode& node) override final {
    return StringLiteral(node, annotations_[node.id]);
  }

  bool BooleanLiteral(BooleanLiteralNode& node) override final {
    return BooleanLiteral(node, annotations_[node.id]);
  }

  bool Invocation(InvocationNode& node) override final {
    return Invocation(node, annotations_[node.id]);
  }

  bool Guard(GuardNode& node) override final {
    return Guard(node, annotations_[node.id]);
  }

  bool Bind(BindNode& node) override final {
    return Bind(node, annotations_[node.id]);
  }

  bool Tuple(TupleNode& node) override final {
    return Tuple(node, annotations_[node.id]);
  }

  AnnotationMap<T>& annotations_;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_UTIL_H_
