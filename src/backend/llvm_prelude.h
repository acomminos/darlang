#ifndef DARLANG_SRC_BACKEND_LLVM_PRELUDE_H_
#define DARLANG_SRC_BACKEND_LLVM_PRELUDE_H_

#include "llvm/IR/IRBuilder.h"

namespace darlang::backend {

// The prelude provides a simple interface to produce call instructions into
// glibc. Its primary use currently is to provide access to glibc's heap.
class LLVMPrelude {
 public:
  LLVMPrelude(llvm::Module* const module, const llvm::DataLayout& layout);

  // Inserts an instruction at the location of an IRBuilder to allocate a value
  // of a sized type on the heap, and return a pointer to it.
  llvm::Value* CreateHeapAlloc(llvm::IRBuilder<>& builder, llvm::Type* type);
 private:
  void LoadFunctions();

  llvm::Module* const module_;
  const llvm::DataLayout& layout_;
  llvm::IntegerType* size_type_;

  llvm::Constant* malloc_func_;
};

}  // namespace darlang::backend

#endif  // DARLANG_SRC_BACKEND_LLVM_PRELUDE_H_
