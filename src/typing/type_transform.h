#ifndef DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
#define DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_

#include <memory>
#include <unordered_map>
#include "ast/types.h"
#include "typing/solver.h"
#include "util/scoped_map.h"

namespace darlang {
namespace typing {

typedef util::ScopedMap<std::string, Typeable*> TypeableMap;

// Traverses the given AST node, solving for a valid type assignment.
class TypeTransform : public ast::Visitor {
 public:
  void Module(ast::ModuleNode& node) override;

 private:
  TypeSystem type_system_;
  // Module-level declarations
  TypeableMap global_scope_;
};

// Computes typeable constraints from a declaration.
// Writes the traversed declaration to the given map of globals to typeables,
// and recurses on function bodies.
class DeclarationTypeTransform : public ast::Visitor {
 public:
  DeclarationTypeTransform(TypeSystem& type_system,
                           TypeableMap& globals)
    : type_system_(type_system), global_scope_(globals) {}

  void Declaration(ast::DeclarationNode& node) override;

 private:
  TypeSystem& type_system_;
  TypeableMap& global_scope_;
};

// Produces a typeable from the expression represented by the given AST node.
// TODO(acomminos): implement scoping rather than args/global duality
class ExpressionTypeTransform : public ast::Visitor {
 public:
  // Computes a typeable from the given expression, and recursively adds
  // constraints to appropriate subexpressions.
  static Typeable* Transform(TypeSystem& type_system,
                             const TypeableMap& scope,
                             ast::Node& node);

  void Invocation(ast::InvocationNode& node) override;
  void Guard(ast::GuardNode& node) override;

  Typeable* result() { return result_; }

 private:
  ExpressionTypeTransform(TypeSystem& type_system, const TypeableMap& scope)
    : type_system_(type_system), scope_(scope), result_(nullptr) {}

  TypeSystem& type_system_;
  const TypeableMap& scope_;
  Typeable* result_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
