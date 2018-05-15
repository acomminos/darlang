#ifndef DARLANG_SRC_AST_TRAVERSER_H_
#define DARLANG_SRC_AST_TRAVERSER_H_

#include <sstream>

namespace darlang {
namespace ast {

// Performs a depth-first traversal of an AST, pretty printing values at each
// level.
class PrettyPrinter : public Visitor {
 public:
  PrettyPrinter() : depth_(0) {}

  bool Module(ModuleNode& node) override {
    pp("Module") << std::endl;

    depth_++;
    for (auto& child : node.body) {
      child->Visit(*this);
    }
    depth_--;
    return false;
  }

  bool Declaration(DeclarationNode& node) override {
    std::stringstream args;
    args << "{";
    for (int i = 0; i < node.args.size(); i++) {
      args << node.args[i];
      if (i + 1 < node.args.size()) {
        args << ",";
      }
    }
    args << "}";

    pp("Declaration") << " ["
      << "id='" << node.id << "',"
      << "args=" << args.str()
      << "]" << std::endl;

    // TODO(acomminos): attrs

    depth_++;
    node.expr->Visit(*this);
    depth_--;
    return false;
  }

  bool Constant(ConstantNode& node) override {
    return false;
  }

  bool IdExpression(IdExpressionNode& node) override {
    pp("IdExpression") << " ["
      << "id='" << node.id << "'"
      << "]" << std::endl;
    return false;
  }

  bool IntegralLiteral(IntegralLiteralNode& node) override {
    pp("IntegralLiteral") << " ["
      << "literal='" << node.literal << "'"
      << "]" << std::endl;
    return false;
  }

  bool StringLiteral(StringLiteralNode& node) override {
    return false; // TODO
  }

  bool BooleanLiteral(BooleanLiteralNode& node) override {
    return false; // TODO
  }

  bool Invocation(InvocationNode& node) override {
    pp("Invocation") << " ["
      << "callee='" << node.callee << "'"
      << "]" << std::endl;

    // TODO(acomminos): attrs

    depth_++;
    for (auto& child : node.args) {
      child->Visit(*this);
    }
    depth_--;
    return false;
  }

  bool Guard(GuardNode& node) override {
    pp("Guard") << std::endl;

    // TODO(acomminos): attrs

    depth_++;
    for (auto& child : node.cases) {
      child.first->Visit(*this);
      child.second->Visit(*this);
    }
    depth_--;
    return false;
  }

 private:
  // TODO(acomminos): add attribute format
  std::ostream& pp(const std::string name) {
    std::stringstream ss;
    for (int i = 0; i < depth_; i++) {
      ss << "| ";
    }
    ss << name;
    return std::cout << ss.str();
  }

  int depth_;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_TRAVERSER_H_
