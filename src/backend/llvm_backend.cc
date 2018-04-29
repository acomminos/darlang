#include "llvm_backend.h"

namespace darlang {
namespace backend {

/* static */
std::unique_ptr<llvm::Module> LLVMModuleTransformer::Transform(llvm::LLVMContext& context, ast::Node& node) {
  LLVMModuleTransformer transformer(context);
  node.Visit(transformer);
  return std::move(transformer.module_);
}

void LLVMModuleTransformer::Module(ast::ModuleNode& node) {
  module_ = llvm::make_unique<llvm::Module>(node.name, context_);

  LLVMDeclarationTransformer decl_transform(context_, module_.get());
  for (auto& child : node.body) {
    child->Visit(decl_transform);
  }
}

void LLVMDeclarationTransformer::Declaration(ast::DeclarationNode& node) {
  // TODO(acomminos): implement type system
  std::vector<llvm::Type*> arg_types(node.args.size(),
                                     llvm::Type::getDoubleTy(context_));
  auto func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context_),
                                           arg_types, false);

  auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.id, module_);
  auto entry_block = llvm::BasicBlock::Create(context_, "entry", func);

  llvm::IRBuilder<> builder(context_);
  builder.SetInsertPoint(entry_block);

  auto expr = LLVMValueTransformer::Transform(context_, builder, *node.expr);
  builder.CreateRet(expr);
}

void LLVMDeclarationTransformer::Constant(ast::ConstantNode& node) {
}

/* static */
llvm::Value* LLVMValueTransformer::Transform(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, ast::Node& node) {
  LLVMValueTransformer transformer(context, builder);
  node.Visit(transformer);
  return transformer.value();
}

void LLVMValueTransformer::Guard(ast::GuardNode& node) {
  // Get the current function we're within.
  auto parent_func = builder_.GetInsertBlock()->getParent();

  // Block containing branches to various case blocks.
  auto prelude_block = llvm::BasicBlock::Create(context_);
  // Combinator block joining cases.
  auto terminal_block = llvm::BasicBlock::Create(context_);

  parent_func->getBasicBlockList().push_back(prelude_block);

  // Insert a phi node as the first instruction in the terminal block.
  builder_.SetInsertPoint(terminal_block);
  // TODO: implement type system
  auto phi_node = builder_.CreatePHI(llvm::Type::getDoubleTy(context_), node.cases.size());

  for (auto& guard_case : node.cases) {
    auto case_block = llvm::BasicBlock::Create(context_);

    // Add a conditional check to the prelude to jump to this case.
    builder_.SetInsertPoint(prelude_block);
    auto cond_value = LLVMValueTransformer::Transform(context_, builder_, *guard_case.first);
    auto branch = builder_.CreateCondBr(cond_value, case_block, nullptr);

    // Compute the expression value in the case block and branch to the terminator.
    builder_.SetInsertPoint(case_block);
    auto expr_value = LLVMValueTransformer::Transform(context_, builder_, *guard_case.second);
    phi_node->addIncoming(expr_value, case_block);
    builder_.CreateBr(terminal_block);

    parent_func->getBasicBlockList().push_back(case_block);
  }

  parent_func->getBasicBlockList().push_back(terminal_block);

  builder_.SetInsertPoint(terminal_block);

  value_ = phi_node;
}

}  // namespace backend
}  // namespace darlang
