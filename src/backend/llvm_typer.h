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

// Synthesizes an LLVM type from the given darlang-internal type.
class LLVMTypeGenerator : public typing::Type::Visitor {
 public:
  // A data structure storing intermediate types generated during the synthesis
  // of a composite type. Useful for materializing recursive types.
  struct TypedPath {
    // FIXME(acomminos): should we really be using a pointer to a synthesized
    // type to unique types on? I like the idea of having types hashable.
    std::unordered_map<const typing::Type*, llvm::Type*> generated_types;
  };

  static llvm::Type* Generate(llvm::LLVMContext& context, typing::Type& type, TypedPath path = {});
  static llvm::Type* Generate(llvm::LLVMContext& context, typing::TypeablePtr& typeable, TypedPath path = {});

  LLVMTypeGenerator(llvm::LLVMContext& context, TypedPath path);

  llvm::Type* result() { return result_; }

 private:
  void Type(typing::Primitive& prim) override;
  void Type(typing::Tuple& tuple) override;
  void Type(typing::Function& func) override;
  void Type(typing::DisjointUnion& disjoint) override;
  void Type(typing::Recurrence& recurrence) override;

  llvm::LLVMContext& context_;
  TypedPath path_;
  llvm::Type* result_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_TYPER_H_
