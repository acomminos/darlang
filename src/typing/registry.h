#ifndef DARLANG_SRC_TYPING_REGISTRY_H_
#define DARLANG_SRC_TYPING_REGISTRY_H_

#include "typing/types.h"

namespace darlang {
namespace typing {

// A simple collection of types generated during the type analysis phase of
// compilation.
//
// TODO(acomminos): leverage this store in the future to produce LLVM type
//                  definitions?
class TypeRegistry {
 public:
  void Insert(std::unique_ptr<Type> type) {
    types_.push_back(std::move(type));
  }

 private:
  std::vector<std::unique_ptr<Type>> types_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_REGISTRY_H_
