#include "typing/tuple_solver.h"
#include "typing/types.h"
#include <unordered_set>

namespace darlang {
namespace typing {

TupleSolver::TupleSolver(int num_items) : items_(num_items) {
  for (auto& item : items_) {
    item = {"", Typeable::Create()};
  }
}

Result TupleSolver::MergeInto(TupleSolver& other) {
  if (other.num_items() != num_items()) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "tuple cardinality mismatch");
  }

  auto self_it = items_.begin();
  auto other_it = other.items_.begin();

  for (int i = 0; i < num_items(); i++) {
    std::string self_item_tag;
    TypeablePtr self_item_typeable;
    std::tie(self_item_tag, self_item_typeable) = *self_it;

    std::string other_item_tag;
    TypeablePtr other_item_typeable;
    std::tie(other_item_tag, other_item_typeable) = *other_it;

    // TODO(acomminos): consider failing unifying an empty string against a
    // non-empty string? requires addition of wildcard constant.
    if (self_item_tag.size() != 0 &&
        other_item_tag.size() != 0 &&
        self_item_tag.compare(other_item_tag) != 0)
    {
      return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "tuple tags differ at index " + i);
    }

    auto item_result = other_item_typeable->Unify(self_item_typeable);
    if (!item_result) {
      return item_result;
    }

    // If the item in the other solver's tag is unset, set it.
    std::string unified_tag = other_item_tag;
    if (other_item_tag.size() == 0) {
      unified_tag = self_item_tag;
    }
    *other_it = {unified_tag, other_item_typeable};

    self_it++;
    other_it++;
  }

  return Result::Ok();
}

Result TupleSolver::Solve(std::unique_ptr<Type>& out_type) {
  std::vector<Tuple::TaggedType> item_types;
  std::unordered_set<std::string> used_tags; // tags assigned to an ordered item

  for (auto& item : items_) {
    std::string tag;
    TypeablePtr typeable;
    std::tie(tag, typeable) = item;

    // Throw an error for a duplicate tag.
    if (used_tags.find(tag) != used_tags.end()) {
      return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "duplicate tag '" + tag + "'");
    }
    used_tags.insert(tag);

    // If this item has been accessed via a tag, unify against the tag usages.
    auto tag_it = tagged_items_.find(tag);
    if (tag_it != tagged_items_.end()) {
      Result tag_result = typeable->Unify(tag_it->second);
      if (!tag_result) {
        return tag_result;
      }
    }

    std::unique_ptr<Type> item_type;
    Result item_result;
    if (!(item_result = typeable->Solve(item_type))) {
      return item_result;
    }
    item_types.push_back({tag, std::move(item_type)});
  }

  // ensure that all items referenced by tags are present in the output type
  for (auto& tag_pair : tagged_items_) {
    if (used_tags.find(tag_pair.first) == used_tags.end()) {
      return Result::Error(ErrorCode::TYPE_INCOMPATIBLE,
                           "tag '" + tag_pair.first + "' not declared");
    }
  }

  out_type = std::make_unique<Tuple>(std::move(item_types));
  return Result::Ok();
}

Result TupleSolver::TagItem(int index, const std::string tag) {
  auto& item_pair = items_[index];
  auto& existing_tag = std::get<std::string>(item_pair);
  if (existing_tag.size() > 0 && existing_tag.compare(tag) != 0) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "conflicting tag at " + index);
  }
  existing_tag = tag;
  return Result::Ok();
}

TypeablePtr TupleSolver::ItemWithTag(const std::string tag) {
  assert(tag.size() > 0);
  auto it = tagged_items_.find(tag);
  if (it != tagged_items_.end()) {
    return it->second;
  }
  auto typeable = Typeable::Create();
  tagged_items_[tag] = typeable;
  return typeable;
}

}  // namespace typing
}  // namespace darlang
