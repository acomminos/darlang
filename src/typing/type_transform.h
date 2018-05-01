#ifndef DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
#define DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_

#include <memory>
#include <unordered_map>
#include "ast/types.h"

namespace darlang {
namespace typing {

// Traverses the given AST node, solving for a valid type assignment.
class TypeTransform : public ast::Visitor {
 public:
  static TypeAssignment InferTypes(ast::NodePtr node);

  void Module(ast::ModuleNode& node) override;

 private:
  TypeSystem type_system_;
  // Module-level declarations
  std::unordered_map<std::string, Typeable*> globals_;
};

// Computes typeable constraints from a declaration.
// Writes the traversed declaration to the given map of globals to typeables,
// and recurses on function bodies.
class DeclarationTypeTransform : public ast::Visitor {
 public:
  DeclarationTypeTransform(TypeSystem& type_system,
                           std::unordered_map<std::string, Typeable*>& globals)
    : type_system_(type_system), globals_(globals) {}

  void Declaration(ast::DeclarationNode& node) override;

 private:
  TypeSystem& type_system_;
  std::unordered_map<std::string, Typeable*>& globals_;
};

// Produces a typeable from the expression represented by the given AST node.
// TODO(acomminos): implement scoping rather than args/global duality
class ExpressionTypeTransform : public ast::Visitor {
 public:
  ExpressionTypeTransform(std::unordered_map<std::string, Typeable*>& args,
                          std::unordered_map<std::string, Typeable*>& globals)
    : args_(args), globals_(globals), result_(nullptr) {}

  void Invocation(ast::InvocationNode& node) override;
  void Guard(ast::GuardNode& node) override;

  Typeable* result() { return result_; }

 private:
  std::unordered_map<std::string, Typeable*>& args_;
  std::unordered_map<std::string, Typeable*>& globals_;

  Typeable* result_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
