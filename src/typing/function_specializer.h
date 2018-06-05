#ifndef DARLANG_SRC_TYPING_FUNCTION_SPECIALIZER_H_
#define DARLANG_SRC_TYPING_FUNCTION_SPECIALIZER_H_

#include "errors.h"
#include "typing/type_transform.h"
#include "typing/typeable.h"
#include "util/declaration_mapper.h"

namespace darlang::typing {

// A specialization consists of a complete type materialization of a
// (potentially) polymorphic function. It consists of a mapping of function
// nodes to types, as well as a typeable backed by a FunctionSolver with
// concrete args.
struct Specialization {
  TypeableMap typeables;
  TypeablePtr func_typeable;
};

// A polymorphic solver for functions in a module.
class Specializer {
 public:
  Specializer(Logger& log, const util::DeclarationMap decl_nodes);

  // Attempts to synthesize a specialization of a callee based on materialized
  // argument types. Unifies all parameters against the created implementation.
  // Returns a typeable representing the type of the function's return value.
  Result Specialize(std::string callee,
                    std::vector<TypeablePtr> args,
                    TypeablePtr& out_yield);

  // Declares the existence of an externally-implemented function that satisfies
  // the provided typeable values. Provided typeable should be backed by a
  // FunctionSolver, and fully materializable (constrained).
  Result AddExternal(std::string callee, TypeablePtr func_typeable);

  std::unordered_map<std::string, std::vector<Specialization>> specs() const {
    return specs_;
  }

 private:
  Logger& log_;
  // A mapping from function identifiers to AST nodes within a module.
  const util::DeclarationMap decl_nodes_;
  // The set of all known specializations for each declared function.
  std::unordered_map<std::string, std::vector<Specialization>> specs_;
};

// A annotator that attempts to materialize the call graph rooted at a given
// function by type inference, populating polymorphic callees as necessary.
//
// The correctness of this technique is leveraged on the theorem that a
// depth-first traversal of the call graph will have every call possess bound
// function arguments, starting from a solved root.
class FunctionSpecializer : public ast::Visitor {
 public:
  // Instantiates a new function specializer to populate typeables based on the
  // provided specialization.
  FunctionSpecializer(Logger& log, Specializer& specializer, Specialization& spec);

  Result result() { return result_; }

  bool Declaration(ast::DeclarationNode& node) override;

 private:
  Logger& log_;
  Specializer& specializer_;
  // The specialization currently being populated.
  Specialization& spec_;
  // Result of function typeable construction/unification/specialization.
  Result result_;
};

}  // namespace darlang::typing

#endif  // DARLANG_SRC_TYPING_FUNCTION_SPECIALIZER_H_
