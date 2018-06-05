#ifndef DARLANG_SRC_BACKEND_LLVM_TYPER_H_
#define DARLANG_SRC_BACKEND_LLVM_TYPER_H_

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

#include "typing/typeable.h"
#include "typing/types.h"

namespace darlang {
namespace backend {

// Synthesizes an LLVM type from the given darlang-internal type.
class LLVMTypeGenerator : public typing::Type::Visitor {
 public:
  static llvm::Type* Generate(llvm::LLVMContext& context, typing::Type& type);
  static llvm::Type* Generate(llvm::LLVMContext& context, typing::TypeablePtr& typeable);

  LLVMTypeGenerator(llvm::LLVMContext& context);

  llvm::Type* result() { return result_; }

 private:
  void Type(typing::Primitive& prim) override;
  void Type(typing::Tuple& tuple) override;
  void Type(typing::Function& func) override;

  llvm::LLVMContext& context_;
  llvm::Type* result_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_TYPER_H_
