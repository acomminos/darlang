#include "backend/llvm_backend.h"
#include "backend/llvm_intrinsics.h"
#include "backend/llvm_typer.h"
#include "typing/solver.h"

namespace darlang {
namespace backend {

/* static */
std::unique_ptr<llvm::Module> LLVMModuleTransformer::Transform(llvm::LLVMContext& context, TypeMap& types, ast::Node& node) {
  LLVMModuleTransformer transformer(context, types);
  node.Visit(transformer);
  return std::move(transformer.module_);
}

bool LLVMModuleTransformer::Module(ast::ModuleNode& node) {
  module_ = llvm::make_unique<llvm::Module>(node.name, context_);

  // Perform an initial pass to populate function declarations.
  LLVMDeclarationTransformer decl_transform(context_, module_.get(), types_);
  for (auto& child : node.body) {
    child->Visit(decl_transform);
  }

  auto& func_table = decl_transform.func_table();

  LLVMFunctionTransformer func_transform(context_, module_.get(), func_table, types_);
  for (auto& child : node.body) {
    child->Visit(func_transform);
  }
  return false;
}

bool LLVMDeclarationTransformer::Declaration(ast::DeclarationNode& node) {
  auto func_type = static_cast<llvm::FunctionType*>(LLVMTypeGenerator::Generate(context_, *types_[node.id]));
  auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.name, module_);
  // TODO(acomminos): check for duplicates
  func_table_[node.name] = func;
  return false;
}

bool LLVMDeclarationTransformer::Constant(ast::ConstantNode& node) {
  // TODO(acomminos)
  return false;
}

bool LLVMFunctionTransformer::Declaration(ast::DeclarationNode& node) {
  auto func = func_table_[node.name];
  auto entry_block = llvm::BasicBlock::Create(context_, "entry", func);

  ArgumentSymbolTable arg_symbols;
  for (auto it = func->arg_begin(); it != func->arg_end(); it++) {
    int arg_idx = arg_symbols.size();
    arg_symbols[node.args[arg_idx]] = it;
  }

  llvm::IRBuilder<> builder(context_);
  builder.SetInsertPoint(entry_block);

  auto expr = LLVMValueTransformer::Transform(context_, builder, *node.expr, func_table_, arg_symbols, types_);
  builder.CreateRet(expr);
  return false;
}

/* static */
llvm::Value* LLVMValueTransformer::Transform(llvm::LLVMContext& context,
                                             llvm::IRBuilder<>& builder,
                                             ast::Node& node,
                                             FunctionTable& funcs,
                                             ArgumentSymbolTable& symbols,
                                             TypeMap& types) {
  LLVMValueTransformer transformer(context, builder, funcs, symbols, types);
  node.Visit(transformer);
  return transformer.value();
}

bool LLVMValueTransformer::IdExpression(ast::IdExpressionNode& node) {
  value_ = symbols_[node.name];
  assert(value_ != nullptr);
  return false;
}

bool LLVMValueTransformer::IntegralLiteral(ast::IntegralLiteralNode& node) {
  // TODO(acomminos): support multiple precision ints
  value_ = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), node.literal);
  return false;
}

bool LLVMValueTransformer::BooleanLiteral(ast::BooleanLiteralNode& node) {
  value_ = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context_), node.literal);
  return false;
}

bool LLVMValueTransformer::Invocation(ast::InvocationNode& node) {
  std::vector<llvm::Value*> arg_values;
  for (auto& expr : node.args) {
    auto value = LLVMValueTransformer::Transform(context_, builder_, *expr, funcs_, symbols_, types_);
    arg_values.push_back(value);
  }

  Intrinsic intr;
  if ((intr = GetIntrinsic(node.callee)) != Intrinsic::UNKNOWN) {
    value_ = GenerateIntrinsic(intr, arg_values, builder_);
  } else {
    auto callee = funcs_[node.callee];
    assert(callee != nullptr);

    value_ = builder_.CreateCall(callee, arg_values);
  }
  return false;
}

bool LLVMValueTransformer::Guard(ast::GuardNode& node) {
  // Get the current function we're within.
  auto parent_func = builder_.GetInsertBlock()->getParent();

  // Current block containing branching to various case blocks.
  auto prelude_block = builder_.GetInsertBlock();
  // Wildcard case for when no pattern matches.
  auto wildcard_block = llvm::BasicBlock::Create(context_, "wildcard");
  // Combinator block joining cases.
  auto terminal_block = llvm::BasicBlock::Create(context_, "terminal");

  // Insert a phi node as the first instruction in the terminal block.
  builder_.SetInsertPoint(terminal_block);

  llvm::Type* guard_type = LLVMTypeGenerator::Generate(context_, *types_[node.id]);
  assert(guard_type);

  auto phi_node = builder_.CreatePHI(guard_type, 1 + node.cases.size());

  // Must have at least one case in order to terminate the prelude block.
  assert(node.cases.size() > 0);

  for (int i = 0; i < node.cases.size(); i++) {
    auto& guard_case = node.cases[i];
    auto case_block = llvm::BasicBlock::Create(context_, "case");

    // Compute the expression value in the case block and branch to the terminator.
    builder_.SetInsertPoint(case_block);
    auto expr_value = LLVMValueTransformer::Transform(context_, builder_, *guard_case.second, funcs_, symbols_, types_);
    phi_node->addIncoming(expr_value, case_block);
    builder_.CreateBr(terminal_block);

    parent_func->getBasicBlockList().push_back(case_block);

    // Add a conditional check to the prelude to jump to this case.
    builder_.SetInsertPoint(prelude_block);
    auto cond_value = LLVMValueTransformer::Transform(context_, builder_, *guard_case.first, funcs_, symbols_, types_);
    if (i < node.cases.size() - 1) {
      // Create and branch to the next possible case if this case's check fails.
      // All blocks' terminators must have a defined control flow.
      auto next_prelude = llvm::BasicBlock::Create(context_, "check");
      builder_.CreateCondBr(cond_value, case_block, next_prelude);
      parent_func->getBasicBlockList().push_back(next_prelude);
      prelude_block = next_prelude;
    } else {
      // When we reach the last case, branch to the wildcard case.
      builder_.CreateCondBr(cond_value, case_block, wildcard_block);
    }
  }

  // Generate the wildcard (else) block.
  builder_.SetInsertPoint(wildcard_block);
  auto wildcard_value = LLVMValueTransformer::Transform(context_, builder_, *node.wildcard_case, funcs_, symbols_, types_);
  phi_node->addIncoming(wildcard_value, wildcard_block);
  builder_.CreateBr(terminal_block);
  parent_func->getBasicBlockList().push_back(wildcard_block);

  parent_func->getBasicBlockList().push_back(terminal_block);

  builder_.SetInsertPoint(terminal_block);

  value_ = phi_node;
  return false;
}

bool LLVMValueTransformer::Bind(ast::BindNode& node) {
  auto expr_value = LLVMValueTransformer::Transform(node.expr);
  auto body_value = LLVMValueTransformer::Transform(node.body);

  // TODO(acomminos): switch to scoped map for llvm values

  return false;
}

}  // namespace backend
}  // namespace darlang
