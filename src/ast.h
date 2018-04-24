#ifndef DARLANG_SRC_AST_H_
#define DARLANG_SRC_AST_H_

#include <memory>
#include <vector>

namespace darlang {
namespace ast {

struct Node;
struct ModuleNode;
struct DeclarationNode;
struct ConstantNode;
struct IntegralLiteralNode;
struct StringLiteralNode;
struct InvocationNode;
struct GuardNode;

typedef std::unique_ptr<Node> NodePtr;

struct Visitor {
  virtual void Module(ModuleNode& node) {}
  virtual void Declaration(DeclarationNode& node) {}
  virtual void Constant(ConstantNode& node) {}
  virtual void IntegralLiteral(IntegralLiteralNode& node) {}
  virtual void StringLiteral(StringLiteralNode& node) {}
  virtual void Invocation(InvocationNode& node) {}
  virtual void Guard(GuardNode& node) {}
};

struct Node {
  // Invokes the visitor on this node, and all child nodes.
  virtual void Visit(Visitor& visitor) = 0;
};

struct ModuleNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.Module(*this);
  }

  std::vector<std::unique_ptr<Node>> body;
};

struct DeclarationNode : public Node {
  DeclarationNode(std::string id, std::vector<std::string> args, std::unique_ptr<Node> expr) : id(id), args(args), expr(std::move(expr)) {}

  void Visit(Visitor& visitor) override {
    visitor.Declaration(*this);
  }

  std::string id;
  std::vector<std::string> args;
  std::unique_ptr<Node> expr;
};

struct ConstantNode : public Node {
  ConstantNode(std::string id, std::unique_ptr<Node> expr) : id(id), expr(std::move(expr)) {}

  void Visit(Visitor& visitor) override {
    visitor.Constant(*this);
  }

  std::string id;
  std::unique_ptr<Node> expr;
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

struct GuardNode : public Node {
  void Visit(Visitor& visitor) override {
    visitor.Guard(*this);
  }

  // (condition, value) expression tuples.
  std::vector<std::pair<NodePtr, NodePtr>> cases;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_H_
