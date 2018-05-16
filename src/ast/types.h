#ifndef DARLANG_SRC_AST_TYPES_H_
#define DARLANG_SRC_AST_TYPES_H_

#include <memory>
#include <vector>

namespace darlang {
namespace ast {

struct Node;
struct ModuleNode;
struct DeclarationNode;
struct ConstantNode;
struct IdExpressionNode;
struct IntegralLiteralNode;
struct StringLiteralNode;
struct BooleanLiteralNode;
struct InvocationNode;
struct GuardNode;

class Typeable;

typedef std::unique_ptr<Node> NodePtr;

// A typical AST visitor, allowing easy traversal.
// If an implementation method returns true, the node is permitted to recurse
// into its children and invoke the visitor recursively.
struct Visitor {
  virtual bool Module(ModuleNode& node) {}
  virtual bool Declaration(DeclarationNode& node) {}
  virtual bool Constant(ConstantNode& node) {}
  virtual bool IdExpression(IdExpressionNode& node) {}
  virtual bool IntegralLiteral(IntegralLiteralNode& node) {}
  virtual bool StringLiteral(StringLiteralNode& node) {}
  virtual bool BooleanLiteral(BooleanLiteralNode& node) {}
  virtual bool Invocation(InvocationNode& node) {}
  virtual bool Guard(GuardNode& node) {}
};

struct Location {
  std::string file;
  int line;
  int column;
};

struct Node {
  // Invokes the visitor on this node, and all child nodes.
  virtual void Visit(Visitor& visitor) = 0;

  // An (optional) pointer to the node's parent.
  Node* parent;
  // Beginning of the range (inclusive) that this node was parsed from.
  Location start;
  // End of the range (inclusive) that this node was parsed from.
  Location end;
};

struct ModuleNode : public Node {
  void Visit(Visitor& visitor) override {
    bool recurse = visitor.Module(*this);
    if (recurse) {
      for (auto& body_elem : body) {
        body_elem->Visit(visitor);
      }
    }
  }

  std::string name;
  std::vector<std::unique_ptr<Node>> body;
};

struct DeclarationNode : public Node {
  DeclarationNode(std::string id, std::vector<std::string> args, std::unique_ptr<Node> expr)
    : id(id), args(args), expr(std::move(expr)) {
    this->expr->parent = this;
  }

  void Visit(Visitor& visitor) override {
    bool recurse = visitor.Declaration(*this);
    if (recurse && expr) {
      expr->Visit(visitor);
    }
  }

  std::string id;
  std::vector<std::string> args;
  std::unique_ptr<Node> expr;
};

struct IdExpressionNode : public Node {
  IdExpressionNode(std::string id) : id(id) {}

  void Visit(Visitor& visitor) override {
    visitor.IdExpression(*this);
  }

  std::string id;
};

struct ConstantNode : public Node {
  ConstantNode(std::string id, std::unique_ptr<Node> expr) : id(id), expr(std::move(expr)) {
    this->expr->parent = this;
  }

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

struct BooleanLiteralNode : public Node {
  BooleanLiteralNode(bool literal) : literal(literal) {}

  void Visit(Visitor& visitor) override {
    visitor.BooleanLiteral(*this);
  }

  bool literal;
};

struct InvocationNode : public Node {
  void Visit(Visitor& visitor) override {
    bool recurse = visitor.Invocation(*this);
    if (recurse) {
      for (auto& arg_elem : args) {
        arg_elem->Visit(visitor);
      }
    }
  }

  std::string callee;
  std::vector<std::unique_ptr<Node>> args;
};

struct GuardNode : public Node {
  void Visit(Visitor& visitor) override {
    bool recurse = visitor.Guard(*this);
    if (recurse) {
      for (auto& case_elem : cases) {
        // TODO(acomminos): add new AST node that models relationship between
        //                  condition and expressions
        case_elem.first->Visit(visitor);
        case_elem.second->Visit(visitor);
      }
      wildcard_case->Visit(visitor);
    }
  }

  // (condition, value) expression tuples.
  std::vector<std::pair<NodePtr, NodePtr>> cases;
  // Fallthrough case when all other cases fail.
  NodePtr wildcard_case;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_TYPES_H_
