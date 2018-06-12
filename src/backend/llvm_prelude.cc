#include "backend/llvm_prelude.h"

#include "llvm/IR/Constants.h"

namespace darlang::backend {

LLVMPrelude::LLVMPrelude(llvm::Module* const module, const llvm::DataLayout& layout)
    : module_(module), layout_(layout) {
  // TODO(acomminos): get platform word size
  size_type_ = llvm::Type::getInt64Ty(module->getContext());

  LoadFunctions();
}

void LLVMPrelude::LoadFunctions() {
  // Use an i8* to represent a void*.
  llvm::Type* const void_ptr_type = llvm::Type::getInt8PtrTy(module_->getContext());

  // Idempotently initialize required standard library functions.
  malloc_func_ = module_->getOrInsertFunction("malloc", void_ptr_type, size_type_);
}

llvm::Value* LLVMPrelude::CreateHeapAlloc(llvm::IRBuilder<>& builder,
                                          llvm::Type* type) {
  const uint64_t size = layout_.getTypeAllocSize(type);
  llvm::CallInst* call = builder.CreateCall(malloc_func_,
      {llvm::ConstantInt::get(size_type_, size)});
  // After the call to malloc has been performed, cast it to a pointer to the
  // desired type.
  llvm::Type* const cast_type = type->getPointerTo();
  return builder.CreateCast(
      llvm::CastInst::getCastOpcode(call, false, cast_type, false),
      call, cast_type);
}

}  // namespace darlang::backend
