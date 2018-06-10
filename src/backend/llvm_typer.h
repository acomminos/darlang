#ifndef DARLANG_SRC_BACKEND_LLVM_TYPER_H_
#define DARLANG_SRC_BACKEND_LLVM_TYPER_H_

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

#include <unordered_map>

#include "typing/typeable.h"
#include "typing/types.h"

namespace darlang {
namespace backend {

// In order to implement recursive types, structs must not be defined literally.
// We still want to unique these structs however, which can be done by storing a
// mapping of type hashes to llvm::Type* instances for a context.
class LLVMTypeCache {
 public:
  llvm::Type* Lookup(const typing::Type& type) {
    return types_[type.Hash()];
  }
  void Insert(const typing::Type& type, llvm::Type* llvm_type) {
    types_[type.Hash()] = llvm_type;
  }

 private:
  // A mapping from type hashes to llvm::Type* instances.
  std::unordered_map<std::string, llvm::Type*> types_;
};

// Synthesizes an LLVM type from the given darlang-internal type.
class LLVMTypeGenerator : public typing::Type::Visitor {
 public:
  static llvm::Type* Generate(llvm::LLVMContext& context, typing::Type& type, LLVMTypeCache& cache);
  static llvm::Type* Generate(llvm::LLVMContext& context, typing::TypeablePtr& typeable, LLVMTypeCache& cache);

  LLVMTypeGenerator(llvm::LLVMContext& context, LLVMTypeCache& cache);

  llvm::Type* result() { return result_; }

 private:
  void Type(typing::Primitive& prim) override;
  void Type(typing::Tuple& tuple) override;
  void Type(typing::Function& func) override;
  void Type(typing::DisjointUnion& disjoint) override;
  void Type(typing::Recurrence& recurrence) override;

  llvm::LLVMContext& context_;
  LLVMTypeCache& cache_;
  llvm::Type* result_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_TYPER_H_
