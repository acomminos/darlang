#include "catch.hpp"

#include "typing/tuple_solver.h"
#include "typing/primitive_solver.h"
#include "typing/types.h"

using Catch::Matchers::Equals;

namespace darlang {
namespace typing {

TEST_CASE("tuples with different cardinality do not unify", "[tuplesolver]") {
  TupleSolver solver_a(1);
  TupleSolver solver_b(2);
  REQUIRE_FALSE(solver_a.Merge(solver_b));
}

TEST_CASE("tagged items can unify with untagged items", "[tuplesolver]") {
  TupleSolver solver(2);
  // In order to solve for a concrete type, we need to materialize subtypes.
  auto int_typeable = Typeable::Create(std::make_unique<PrimitiveSolver>(PrimitiveType::Int64));
  for (auto& item : solver.items()) {
    REQUIRE(std::get<TypeablePtr>(item)->Unify(int_typeable));
  }

  // Define a new solver to take a tag from for the second element.
  TupleSolver other_solver(2);
  REQUIRE(solver.TagItem(0, "hello"));
  REQUIRE(other_solver.TagItem(1, "goodbye"));
  REQUIRE(solver.Merge(other_solver));

  std::unique_ptr<Type> type;
  REQUIRE(solver.Solve(type));

  auto tuple_type = dynamic_cast<Tuple*>(type.get());
  REQUIRE(tuple_type);

  REQUIRE_THAT(std::get<std::string>(tuple_type->types()[0]), Equals("hello"));
  REQUIRE_THAT(std::get<std::string>(tuple_type->types()[1]), Equals("goodbye"));
}

TEST_CASE("items with different tags cannot unify", "[tuplesolver]") {
  TupleSolver solver_a(1);
  REQUIRE(solver_a.TagItem(0, "hello"));

  TupleSolver solver_b(1);
  REQUIRE(solver_b.TagItem(0, "goodbye"));

  REQUIRE_FALSE(solver_a.Merge(solver_b));
}

}  // namespace typing
}  // namespace darlang
