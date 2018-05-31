#ifndef DARLANG_SRC_TYPING_TUPLE_SOLVER_H_
#define DARLANG_SRC_TYPING_TUPLE_SOLVER_H_

#include "typing/solver.h"
#include <unordered_map>
#include <vector>

namespace darlang {
namespace typing {

// Solves for an ordered list of types.
class TupleSolver : public Solver {
 public:
  TupleSolver(int num_items);

  Result Merge(Solver& solver) override { return solver.MergeInto(*this); }
  Result MergeInto(TupleSolver& other) override;
  Result Solve(std::unique_ptr<Type>& out_type) override;

  // Assigns a tag to the item at the provided index.
  // Returns an error if the item has been assigned a conflicting tag.
  Result TagItem(int index, const std::string tag);

  // Returns a typeable for the item with the given tag.
  // Implicitly declares the existence of an item with the tag.
  TypeablePtr ItemWithTag(const std::string tag);

  int num_items() const { return items_.size(); }
  const std::vector<std::tuple<std::string, TypeablePtr>>& items() const { return items_; }

 private:
  // An ordered list of tuple items, with optional tags.
  std::vector<std::tuple<std::string, TypeablePtr>> items_;

  // A mapping of tag names to typeables. Populated by accessing tags for
  // fields. All tags in this map must exist in `items_` for a valid type
  // solution to be possible.
  std::unordered_map<std::string, TypeablePtr> tagged_items_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TUPLE_SOLVER_H_
