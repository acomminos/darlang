#include "backend/llvm_typer.h"

namespace darlang {
namespace backend {

/* static */
llvm::Type* LLVMTypeGenerator::Generate(llvm::LLVMContext& context, typing::Type& type) {
  LLVMTypeGenerator generator(context);
  type.Visit(generator);
  return generator.result();
}

llvm::Type* LLVMTypeGenerator::Generate(llvm::LLVMContext& context, typing::TypeablePtr& typeable) {
  std::unique_ptr<typing::Type> type;
  assert(typeable->Solve(type));
  return Generate(context, *type);
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
      // Treat all strings as a pointer to null-terminated 8-bit character data.
      // We may want to represent this as a (ptr, len) tuple later.
      result_ = llvm::Type::getInt8PtrTy(context_);
      break;
    default:
      assert(false);
      break;
  }
}

void LLVMTypeGenerator::Type(typing::Tuple& tuple) {
  std::vector<llvm::Type*> item_types;
  for (auto& tuple_item : tuple.types()) {
    auto& type = std::get<std::unique_ptr<typing::Type>>(tuple_item);
    item_types.push_back(LLVMTypeGenerator::Generate(context_, *type));
  }
  result_ = llvm::StructType::get(context_, item_types);
}

void LLVMTypeGenerator::Type(typing::Function& func) {
  llvm::Type* yield_type = LLVMTypeGenerator::Generate(context_, *func.yields());
  std::vector<llvm::Type*> arg_types(func.arguments().size());
  for (unsigned int i = 0; i < func.arguments().size(); i++) {
    arg_types[i] = LLVMTypeGenerator::Generate(context_, *func.arguments()[i]);
  }
  result_ = llvm::FunctionType::get(yield_type, arg_types, false);
}

}  // namespace backend
}  // namespace darlang
