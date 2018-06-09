#include "backend/llvm_typer.h"
#include "llvm/IR/DataLayout.h"

namespace darlang {
namespace backend {

/* static */
llvm::Type* LLVMTypeGenerator::Generate(llvm::LLVMContext& context, typing::Type& type, TypedPath path) {
  LLVMTypeGenerator generator(context, path);
  type.Visit(generator);
  return generator.result();
}

llvm::Type* LLVMTypeGenerator::Generate(llvm::LLVMContext& context, typing::TypeablePtr& typeable, TypedPath path) {
  std::unique_ptr<typing::Type> type;
  assert(typeable->Solve(type));
  return Generate(context, *type, path);
}

LLVMTypeGenerator::LLVMTypeGenerator(llvm::LLVMContext& context, TypedPath path)
  : context_(context), path_(path), result_(nullptr) {
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
  llvm::StructType* tuple_type = llvm::StructType::create(context_, "");
  path_.generated_types[&tuple] = tuple_type;

  std::vector<llvm::Type*> item_types;
  for (auto& tuple_item : tuple.types()) {
    auto& type = std::get<std::unique_ptr<typing::Type>>(tuple_item);
    item_types.push_back(LLVMTypeGenerator::Generate(context_, *type, path_));
  }

  tuple_type->setBody(item_types);
  result_ = tuple_type;
}

void LLVMTypeGenerator::Type(typing::Function& func) {
  // FIXME(acomminos): add function stub here
  llvm::Type* yield_type = LLVMTypeGenerator::Generate(context_, *func.yields(), path_);
  std::vector<llvm::Type*> arg_types(func.arguments().size());
  for (unsigned int i = 0; i < func.arguments().size(); i++) {
    arg_types[i] = LLVMTypeGenerator::Generate(context_, *func.arguments()[i], path_);
  }
  result_ = llvm::FunctionType::get(yield_type, arg_types, false);
}

void LLVMTypeGenerator::Type(typing::DisjointUnion& disjoint) {
  // A disjoint union is represented in IR by a type index, as well as a
  // segment of data large enough to store the largest subtype.
  assert(disjoint.types().size() <= UINT32_MAX);

  llvm::StructType* disjoint_type = llvm::StructType::create(context_, "");
  // Store intermediate types along the current path in case a subtype is
  // recursive.
  path_.generated_types[&disjoint] = disjoint_type;

  llvm::Type* index_type = llvm::Type::getInt32Ty(context_);
  uint64_t max_bytes = 0;
  for (auto& type : disjoint.types()) {
    // FIXME(acomminos): don't use default data layout, share with module
    llvm::DataLayout layout("");
    llvm::Type* subtype = LLVMTypeGenerator::Generate(context_, *type, path_);
    auto bytes = layout.getTypeSizeInBits(subtype) / 8;
    max_bytes = bytes > max_bytes ? bytes : max_bytes;
  }

  llvm::Type* data_type = llvm::ArrayType::get(
      llvm::Type::getInt8Ty(context_), max_bytes);

  disjoint_type->setBody({index_type, data_type});
  result_ = disjoint_type;
}

void LLVMTypeGenerator::Type(typing::Recurrence& recurrence) {
  // TODO(acomminos): recurrence types indicate a reference to a parent type,
  // modeled by a pointer to that intermediary type being generated.
  assert(recurrence.parent_type());
  llvm::Type* parent_type = path_.generated_types[recurrence.parent_type()];
  assert(parent_type);
  result_ = parent_type->getPointerTo();
}

}  // namespace backend
}  // namespace darlang
