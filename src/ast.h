#ifndef DARLANG_SRC_AST_H_
#define DARLANG_SRC_AST_H_

#include <memory>
#include <vector>

namespace darlang {
namespace ast {

struct AssignmentNode;
struct UnaryOperatorNode;
struct BinaryOperatorNode;
struct IntegralLiteralNode;
struct StringLiteralNode;
struct InvocationNode;

struct Visitor {
  virtual void Assignment(AssignmentNode& node) {}
  virtual void UnaryOperator(UnaryOperatorNode& node) {}
  virtual void BinaryOperator(BinaryOperatorNode& node) {}
  virtual void IntegralLiteral(IntegralLiteralNode& node) {}
  virtual void StringLiteral(StringLiteralNode& node) {}
  virtual void Invocation(InvocationNode& node) {}
};

struct Node {
  // Invokes the visitor on this node, and all child nodes.
  virtual void Visit(Visitor& visitor) = 0;
};

struct AssignmentNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.Assignment(*this);
  }

  std::string id;
  std::unique_ptr<Node> expr;
};

struct BinaryOperatorNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.BinaryOperator(*this);
  }

  enum Op {
    MOD
  } op;
  std::unique_ptr<Node> lhs;
  std::unique_ptr<Node> rhs;
};

struct UnaryOperatorNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.UnaryOperator(*this);
  }

  enum Op {
    NOT
  } op;
  std::unique_ptr<Node> lhs;
  std::unique_ptr<Node> rhs;
};

struct IntegralLiteralNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.IntegralLiteral(*this);
  }

  int64_t literal;
};

struct StringLiteralNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.StringLiteral(*this);
  }

  std::string literal;
};

struct InvocationNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.Invocation(*this);
  }

  std::string callee;
  std::vector<std::unique_ptr<Node>> args;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_H_
