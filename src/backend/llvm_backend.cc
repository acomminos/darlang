#include "llvm_backend.h"
#include "llvm_intrinsics.h"

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

  // Perform an initial pass to populate function declarations.
  LLVMDeclarationTransformer decl_transform(context_, module_.get());
  for (auto& child : node.body) {
    child->Visit(decl_transform);
  }

  auto& func_table = decl_transform.func_table();

  LLVMFunctionTransformer func_transform(context_, module_.get(), func_table);
  for (auto& child : node.body) {
    child->Visit(func_transform);
  }
}

void LLVMDeclarationTransformer::Declaration(ast::DeclarationNode& node) {
  // TODO(acomminos): implement type system
  std::vector<llvm::Type*> arg_types(node.args.size(),
                                     llvm::Type::getInt64Ty(context_));
  auto func_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(context_),
                                           arg_types, false);
  auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.id, module_);
  // TODO(acomminos): check for duplicates
  func_table_[node.id] = func;
}

void LLVMDeclarationTransformer::Constant(ast::ConstantNode& node) {
  // TODO(acomminos)
}

void LLVMFunctionTransformer::Declaration(ast::DeclarationNode& node) {
  auto func = func_table_[node.id];
  auto entry_block = llvm::BasicBlock::Create(context_, "entry", func);

  ArgumentSymbolTable arg_symbols;
  for (auto it = func->arg_begin(); it != func->arg_end(); it++) {
    int arg_idx = arg_symbols.size();
    arg_symbols[node.args[arg_idx]] = it;
  }

  llvm::IRBuilder<> builder(context_);
  builder.SetInsertPoint(entry_block);

  auto expr = LLVMValueTransformer::Transform(context_, builder, *node.expr, func_table_, arg_symbols);
  builder.CreateRet(expr);
}

/* static */
llvm::Value* LLVMValueTransformer::Transform(llvm::LLVMContext& context,
                                             llvm::IRBuilder<>& builder,
                                             ast::Node& node,
                                             FunctionTable& funcs,
                                             ArgumentSymbolTable& symbols) {
  LLVMValueTransformer transformer(context, builder, funcs, symbols);
  node.Visit(transformer);
  return transformer.value();
}

void LLVMValueTransformer::IdExpression(ast::IdExpressionNode& node) {
  value_ = symbols_[node.id];
  assert(value_ != nullptr);
}

void LLVMValueTransformer::IntegralLiteral(ast::IntegralLiteralNode& node) {
  // TODO(acomminos): support multiple precision ints
  value_ = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), node.literal);
}

void LLVMValueTransformer::BooleanLiteral(ast::BooleanLiteralNode& node) {
  value_ = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context_), node.literal);
}

void LLVMValueTransformer::Invocation(ast::InvocationNode& node) {
  std::vector<llvm::Value*> arg_values;
  for (auto& expr : node.args) {
    auto value = LLVMValueTransformer::Transform(context_, builder_, *expr, funcs_, symbols_);
    arg_values.push_back(value);
  }

  Intrinsic intr;
  if ((intr = GetIntrinsic(node.callee)) != I_UNKNOWN) {
    value_ = GenerateIntrinsic(intr, arg_values, builder_);
  } else {
    auto callee = funcs_[node.callee];
    assert(callee != nullptr);

    value_ = builder_.CreateCall(callee, arg_values);
  }
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
  auto phi_node = builder_.CreatePHI(llvm::Type::getInt64Ty(context_), node.cases.size());

  for (auto& guard_case : node.cases) {
    auto case_block = llvm::BasicBlock::Create(context_);

    // Add a conditional check to the prelude to jump to this case.
    builder_.SetInsertPoint(prelude_block);
    auto cond_value = LLVMValueTransformer::Transform(context_, builder_, *guard_case.first, funcs_, symbols_);
    auto branch = builder_.CreateCondBr(cond_value, case_block, nullptr);

    // Compute the expression value in the case block and branch to the terminator.
    builder_.SetInsertPoint(case_block);
    auto expr_value = LLVMValueTransformer::Transform(context_, builder_, *guard_case.second, funcs_, symbols_);
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
