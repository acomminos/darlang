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
    pp("Module", node) << std::endl;

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

    pp("Declaration", node) << " ["
      << "name='" << node.name << "',"
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
    pp("IdExpression", node) << " ["
      << "id='" << node.id << "'"
      << "]" << std::endl;
    return false;
  }

  bool IntegralLiteral(IntegralLiteralNode& node) override {
    pp("IntegralLiteral", node) << " ["
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
    pp("Invocation", node) << " ["
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
    pp("Guard", node) << std::endl;

    // TODO(acomminos): attrs

    depth_++;
    for (auto& child : node.cases) {
      child.first->Visit(*this);
      child.second->Visit(*this);
    }
    node.wildcard_case->Visit(*this);
    depth_--;
    return false;
  }

  bool Bind(BindNode& node) override {
    pp("Bind", node) << "[to='" << node.identifier << "']" << std::endl;
    depth_++;

    pp("Expr", *node.expr) << std::endl;
    depth_++;
    node.expr->Visit(*this);
    depth_--;

    pp("Body", *node.body) << std::endl;
    depth_++;
    node.body->Visit(*this);
    depth_--;

    depth_--;
    return false;
  }

  bool Tuple(TupleNode& node) override {
    pp("Tuple", node) << std::endl;

    depth_++;
    for (int i = 0; i < node.items.size(); i++) {
      std::stringstream item_label;
      item_label << "[" << i << "]";
      item_label << "[label='" << std::get<std::string>(node.items[i]) << "']";

      auto& child_node = std::get<ast::NodePtr>(node.items[i]);
      pp(item_label.str(), *child_node) << std::endl;
      depth_++;
      child_node->Visit(*this);
      depth_--;
    }
    depth_--;
  }

 private:
  // TODO(acomminos): add attribute format
  std::ostream& pp(const std::string name, const ast::Node& node) {
    std::stringstream ss;
    for (int i = 0; i < depth_; i++) {
      ss << "| ";
    }
    ss << name;
    ss << " "
       << "[" << node.start.file << ":" << node.start.line << ":" << node.start.column << "]";
    return std::cout << ss.str();
  }

  int depth_;
};

}  // namespace ast
}  // namespace darlang

#endif  // DARLANG_SRC_AST_TRAVERSER_H_
