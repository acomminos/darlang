#include "catch.hpp"

#include "typing/tuple_solver.h"
#include "typing/types.h"

using darlang::typing::TupleSolver;
using darlang::typing::Tuple;

TEST_CASE("untagged tuple unification", "[tuplesolver]") {

  SECTION("tuples with different cardinality do not unify") {
    TupleSolver solver_a(1);
    TupleSolver solver_b(2);
    REQUIRE_FALSE(solver_a.Merge(solver_b));
  }
}

TEST_CASE("tagged tuple unification", "[tuplesolver]") {

  SECTION("tagged items can unify with untagged items") {
    TupleSolver solver_a(2);
    REQUIRE(solver_a.TagItem(0, "hello"));

    TupleSolver solver_b(2);
    REQUIRE(solver_a.TagItem(1, "goodbye"));

    REQUIRE(solver_a.Merge(solver_b));
  }
  SECTION("items with different tags cannot unify") {
    TupleSolver solver_a(1);
    REQUIRE(solver_a.TagItem(0, "hello"));

    TupleSolver solver_b(1);
    REQUIRE(solver_b.TagItem(0, "goodbye"));

    REQUIRE_FALSE(solver_a.Merge(solver_b));
  }
}

TEST_CASE("tuple solving", "[tuplesolver]") {
}
