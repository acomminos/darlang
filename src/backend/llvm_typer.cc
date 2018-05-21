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
    default:
      assert(false);
      break;
  }
}

void LLVMTypeGenerator::Type(typing::Function& func) {
  llvm::Type* yield_type = LLVMTypeGenerator::Generate(context_, *func.yields());
  std::vector<llvm::Type*> arg_types(func.arguments().size());
  for (int i = 0; i < func.arguments().size(); i++) {
    arg_types[i] = LLVMTypeGenerator::Generate(context_, *func.arguments()[i]);
  }
  result_ = llvm::FunctionType::get(yield_type, arg_types, false);
}

}  // namespace backend
}  // namespace darlang
