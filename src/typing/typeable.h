#ifndef DARLANG_SRC_TYPING_TYPEABLE_H_
#define DARLANG_SRC_TYPING_TYPEABLE_H_

#include <memory>
#include <vector>
#include "errors.h"

namespace darlang {
namespace typing {

class Typeable;
class Type;
class Recurrence;
class Solver;

// A reference-counted typeable.
typedef std::shared_ptr<Typeable> TypeablePtr;

// A constrainable handle expected to resolve to a type after application of
// union-find to referencing AST nodes.
//
// shared_ptr is used rather than a single ownership hierarchy of unique_ptrs to
// handle typeables allocated by solvers. If two solvers get unified and one
// gets destroyed, the members/arguments/yield Typeables owned by the destroyed
// solver become invalid.
class Typeable : public std::enable_shared_from_this<Typeable> {
 public:
  static TypeablePtr Create(std::unique_ptr<Solver> solver = nullptr);
  // Instantiates a new typeable with the given solver.
  Typeable(std::unique_ptr<Solver> solver);
  // Unifies a typeable into this typeable, intersecting their type solvers.
  Result Unify(const TypeablePtr& other);
  // Attempts to solve for a concrete type using the underlying solver.
  // If the type is recursive, self-references are automatically stubbed out.
  Result Solve(std::unique_ptr<Type>& out_type);
  // XXX: an "unsafe" prototype of a cleaner solve API, under the expectation
  // that all typeables are solvable. This may be the case one day, in which
  // case this should return "UnboundType" for all invalid cases.
  std::unique_ptr<Type> Solve();
  // Asks the solver to synthesize a type, returning true on success.
  bool IsSolvable();

 private:
  // null if the typeable is completely unbound.
  std::unique_ptr<Solver> solver_;
  // A collection of fields related to the currently active solve run.
  // Only valid for the lifetime of a Solve() call.
  struct {
    bool active; // true iff a call to Solve() is in-progress.
    std::vector<Recurrence*> recurrences;
  } solve_run_;

  TypeablePtr parent_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPEABLE_H_
