#ifndef DARLANG_SRC_TYPING_FUNCTION_SPECIALIZER_H_
#define DARLANG_SRC_TYPING_FUNCTION_SPECIALIZER_H_

#include "typing/type_transform.h"
#include "typing/typeable.h"

namespace darlang::typing {

// A specialization consists of a complete type materialization of a
// (potentially) polymorphic function. It consists of a mapping of function
// nodes to types, as well as typeables that map to the specialization's
// argument and yield types.
struct Specialization {
  TypeableMap typeables;

  std::vector<TypeablePtr> args;
  TypeablePtr yield;
};

class Specializer {
 public:
  Result Specialize(std::string callee,
                    const std::vector<TypeablePtr>& args,
                    const TypeablePtr& yield);
 private:
  util::DeclarationMap decl_nodes_;
};

// A annotator that attempts to materialize the call graph rooted at a given
// function by type inference, populating polymorphic callees as necessary.
//
// The correctness of this technique is leveraged on the theorem that a
// depth-first traversal of the call graph will have every call possess bound
// function arguments, starting from a solved root.
class FunctionSpecializer : public ast::Visitor {
 public:
  // Specializes the module-level function with the given identifier, unifying
  // against the provided argument and yield typeables. Recurses into callees as
  // appropriate.
  static Result Specialize(std::string callee,
                           const std::vector<TypeablePtr> args,
                           const TypeablePtr yield,
                           Logger& log,
                           const util::DeclarationMap& decl_nodes,
                           std::unordered_map<std::string, std::vector<Specialization>>& specs);

  // Instantiates a new function specializer to populate typeables based on the
  // provided specialization.
  FunctionSpecializer(Specialization& spec,
                      Logger& log,
                      const util::DeclarationMap& decl_nodes,
                      std::unordered_map<std::string, std::vector<Specialization>>& specs);


  bool Declaration(DeclarationNode& node) override;

  Result result() const { return result_; }

 private:
  Logger& log_;
  // The specialization currently being populated.
  Specialization current_spec_;
  // A mapping from function identifiers to AST nodes within a module.
  const util::DeclarationMap& decl_nodes_;
  // The set of all known specializations for each declared function.
  std::unordered_map<std::string, std::vector<Specialization>> specs_;
  // Result of function typeable construction/unification/specialization.
  Result result_;
};

}  // namespace darlang::typing

#endif  // DARLANG_SRC_TYPING_FUNCTION_SPECIALIZER_H_
