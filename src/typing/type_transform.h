#ifndef DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
#define DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_

#include <memory>
#include <unordered_map>
#include "ast/types.h"
#include "ast/util.h"
#include "typing/solver.h"
#include "util/scoped_map.h"

namespace darlang {
namespace typing {

typedef util::ScopedMap<std::string, Typeable*> TypeableMap;

// Traverses the given AST node, solving for a valid type assignment.
class TypeTransform : public ast::Reducer<TypeableMap> {
  void Module(ast::ModuleNode& node) override;
};

// Computes typeable constraints from a declaration.
// Writes the traversed declaration to the given map of globals to typeables,
// and recurses on function bodies.
class DeclarationTypeTransform : public ast::Visitor {
 public:
  DeclarationTypeTransform(TypeableMap& globals)
    : global_scope_(globals) {}

  void Declaration(ast::DeclarationNode& node) override;

 private:
  TypeableMap& global_scope_;
};

// Produces a typeable from the expression represented by the given AST node.
class ExpressionTypeTransform : public ast::Reducer<std::unique_ptr<Typeable>> {
 public:
  ExpressionTypeTransform(const TypeableMap& scope)
    : scope_(scope), result_(nullptr) {}

 private:
  void Invocation(ast::InvocationNode& node) override;
  void Guard(ast::GuardNode& node) override;

  const TypeableMap& scope_;
  std::unique_ptr<Typeable> result_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
