#include "backend/llvm_typer.h"

namespace darlang {
namespace backend {

/* static */
llvm::Type* LLVMTypeGenerator::Generate(llvm::LLVMContext& context, typing::Type& type) {
  LLVMTypeGenerator generator(context);
  type.Visit(generator);
  return generator.result();
}

LLVMTypeGenerator::LLVMTypeGenerator(llvm::LLVMContext& context) : context_(context), result_(nullptr) {
}

void LLVMTypeGenerator::Type(typing::Primitive& prim) {
  switch (prim.type()) {
    case typing::PrimitiveType::Int64:
      result_ = llvm::Type::getInt64Ty(context_);
      break;
    case typing::PrimitiveType::Float:
      result_ = llvm::Type::getFloatTy(context_);
      break;
    case typing::PrimitiveType::Boolean:
      result_ = llvm::Type::getInt1Ty(context_);
      break;
    case typing::PrimitiveType::String:
      assert(false); // TODO(acomminos)
      break;
  }
}

void LLVMTypeGenerator::Function(typing::Function& func) {
  // TODO(acomminos)
}

}  // namespace backend
}  // namespace darlang
