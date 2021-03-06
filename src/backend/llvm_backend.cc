#include "backend/llvm_backend.h"
#include "backend/llvm_intrinsics.h"
#include "backend/llvm_symbol_namer.h"
#include "backend/llvm_typer.h"
#include "backend/llvm_prelude.h"
#include "typing/solver.h"

namespace darlang {
namespace backend {

/* static */
std::unique_ptr<llvm::Module> LLVMModuleTransformer::Transform(
    llvm::LLVMContext& context, typing::SpecializationMap& specs, ast::Node& node) {
  LLVMModuleTransformer transformer(context, specs);
  node.Visit(transformer);
  return std::move(transformer.module_);
}

bool LLVMModuleTransformer::Module(ast::ModuleNode& node) {
  module_ = llvm::make_unique<llvm::Module>(node.name, context_);
  SymbolTable symbols;
  LLVMTypeCache cache;

  llvm::DataLayout layout(module_.get());
  LLVMPrelude prelude(module_.get(), layout);

  // Perform an initial pass to populate function declarations.
  LLVMDeclarationTransformer decl_transform(module_.get(), specs_, symbols, cache);
  for (auto& child : node.body) {
    child->Visit(decl_transform);
  }

  LLVMFunctionTransformer func_transform(context_, specs_, symbols, cache, prelude);
  for (auto& child : node.body) {
    child->Visit(func_transform);
  }
  return false;
}

bool LLVMDeclarationTransformer::Declaration(ast::DeclarationNode& node) {
  auto func_specs = specs_.Get(node.name);

  // Only allow one specialization for monomorphic functions.
  if (!node.exported) {
    assert(func_specs.size() == 1);
  }
  // TODO(acomminos): warn about empty func_specs?

  for (auto& spec : func_specs) {
    std::string symbol_name;
    if (node.exported) {
      symbol_name = LLVMSymbolNamer::Declaration(node.name, *spec.func_typeable->Solve());
    } else {
      symbol_name = node.name;
    }

    auto func_type = static_cast<llvm::FunctionType*>(LLVMTypeGenerator::Generate(module_->getContext(), spec.func_typeable, cache_));
    auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, symbol_name, module_);
    // TODO(acomminos): check for duplicates, assign using symbol name
    symbols_.Assign(symbol_name, func);
  }
  return false;
}

bool LLVMDeclarationTransformer::Constant(ast::ConstantNode& node) {
  // TODO(acomminos)
  assert(false);
  return false;
}

bool LLVMFunctionTransformer::Declaration(ast::DeclarationNode& node) {
  auto func_specs = specs_.Get(node.name);

  // Only allow one specialization for monomorphic functions.
  if (!node.exported) {
    assert(func_specs.size() == 1);
  }

  // TODO(acomminos): warn about empty func_specs?
  for (auto& spec : func_specs) {
    std::string symbol_name;
    if (node.exported) {
      symbol_name = LLVMSymbolNamer::Declaration(node.name, *spec.func_typeable->Solve());
    } else {
      symbol_name = node.name;
    }

    // FIXME(acomminos): casts make me sad.
    auto func = static_cast<llvm::Function*>(symbols_.Lookup(symbol_name));
    assert(func);

    auto entry_block = llvm::BasicBlock::Create(context_, "entry", func);

    int arg_idx = 0;
    SymbolTable arg_symbols(symbols_);
    for (auto it = func->arg_begin(); it != func->arg_end(); it++) {
      arg_symbols.Assign(node.args[arg_idx++], it);
    }

    llvm::IRBuilder<> builder(context_);
    builder.SetInsertPoint(entry_block);

    auto expr = LLVMValueTransformer::Transform(context_, builder, spec.typeables, arg_symbols, cache_, prelude_, *node.expr);
    builder.CreateRet(expr);
  }
  return false;
}

/* static */
llvm::Value* LLVMValueTransformer::Transform(llvm::LLVMContext& context,
                                             llvm::IRBuilder<>& builder,
                                             typing::TypeableMap& types,
                                             const SymbolTable& symbols,
                                             LLVMTypeCache& cache,
                                             LLVMPrelude& prelude,
                                             ast::Node& node) {
  LLVMValueTransformer transformer(context, builder, types, symbols, cache, prelude);
  node.Visit(transformer);
  return transformer.value();
}

bool LLVMValueTransformer::IdExpression(ast::IdExpressionNode& node) {
  value_ = symbols_.Lookup(node.name);
  assert(value_ != nullptr);
  return false;
}

bool LLVMValueTransformer::IntegralLiteral(ast::IntegralLiteralNode& node) {
  // TODO(acomminos): support multiple precision ints
  value_ = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), node.literal);
  return false;
}

bool LLVMValueTransformer::StringLiteral(ast::StringLiteralNode& node) {
  // Represent strings as a pointer to null-terminated characters in static
  // data. LLVM will automatically null-terminate this, which we may want to
  // reconsider later.
  value_ = builder_.CreateGlobalStringPtr(node.literal);
  return false;
}

bool LLVMValueTransformer::BooleanLiteral(ast::BooleanLiteralNode& node) {
  value_ = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context_), node.literal);
  return false;
}

bool LLVMValueTransformer::Invocation(ast::InvocationNode& node) {
  std::vector<llvm::Value*> arg_values;
  for (auto& expr : node.args) {
    auto value = LLVMValueTransformer::Transform(context_, builder_, types_, symbols_, cache_, prelude_, *expr);
    arg_values.push_back(value);
  }

  Intrinsic intr;
  llvm::Value* callee;
  if ((intr = GetIntrinsic(node.callee)) != Intrinsic::UNKNOWN) {
    value_ = GenerateIntrinsic(intr, arg_values, builder_);
  } else if ((callee = symbols_.Lookup(node.callee))) {
    // Use the unobfuscated symbol name, in case this is a monomorphic export or
    // the main function.
    value_ = builder_.CreateCall(callee, arg_values);
  } else {
    // Otherwise, try to generate a call to the appropriate specialization by
    // deriving it from argument types and the callee name.
    //
    // TODO(acomminos): make simpler, perhaps by leveraging typeable linkage?
    std::vector<std::unique_ptr<typing::Type>> arg_types;
    for (auto& arg_node : node.args) {
      auto arg_typeable = types_[arg_node->id];
      arg_types.push_back(arg_typeable->Solve());
    }
    auto symbol_name = LLVMSymbolNamer::Call(node.callee, arg_types);
    auto callee = symbols_.Lookup(symbol_name);
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

  llvm::Type* guard_type = LLVMTypeGenerator::Generate(context_, types_[node.id], cache_);
  assert(guard_type);

  auto phi_node = builder_.CreatePHI(guard_type, 1 + node.cases.size());

  // Must have at least one case in order to terminate the prelude block.
  assert(node.cases.size() > 0);

  for (int i = 0; i < node.cases.size(); i++) {
    auto& guard_case = node.cases[i];
    auto case_block = llvm::BasicBlock::Create(context_, "case");

    // Compute the expression value in the case block and branch to the terminator.
    builder_.SetInsertPoint(case_block);
    auto expr_value = LLVMValueTransformer::Transform(context_, builder_, types_, symbols_, cache_, prelude_, *guard_case.second);
    phi_node->addIncoming(expr_value, case_block);
    builder_.CreateBr(terminal_block);

    parent_func->getBasicBlockList().push_back(case_block);

    // Add a conditional check to the prelude to jump to this case.
    builder_.SetInsertPoint(prelude_block);
    auto cond_value = LLVMValueTransformer::Transform(context_, builder_, types_, symbols_, cache_, prelude_, *guard_case.first);
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
  auto wildcard_value = LLVMValueTransformer::Transform(context_, builder_, types_, symbols_, cache_, prelude_, *node.wildcard_case);
  phi_node->addIncoming(wildcard_value, wildcard_block);
  builder_.CreateBr(terminal_block);
  parent_func->getBasicBlockList().push_back(wildcard_block);

  parent_func->getBasicBlockList().push_back(terminal_block);

  builder_.SetInsertPoint(terminal_block);

  value_ = phi_node;
  return false;
}

bool LLVMValueTransformer::Bind(ast::BindNode& node) {
  auto expr_value = LLVMValueTransformer::Transform(context_, builder_, types_, symbols_, cache_, prelude_, *node.expr);

  SymbolTable scoped_table(symbols_);
  // TODO(acomminos): enforce identifier override?
  scoped_table.Assign(node.identifier, expr_value);

  value_ = LLVMValueTransformer::Transform(context_, builder_, types_, scoped_table, cache_, prelude_, *node.body);
  return false;
}

bool LLVMValueTransformer::Tuple(ast::TupleNode& node) {
  llvm::Type* tuple_type = LLVMTypeGenerator::Generate(context_, types_[node.id], cache_);
  assert(tuple_type);

  llvm::Value* struct_addr;
  if (auto* ptr_type = llvm::dyn_cast<llvm::PointerType>(tuple_type)) {
    // TODO/FIXME(acomminos): make a call to malloc() for tuples yielding a
    //                        pointer type
    struct_addr = prelude_.CreateHeapAlloc(builder_, ptr_type->getElementType());
  } else {
    struct_addr = builder_.CreateAlloca(tuple_type);
  }

  unsigned int tuple_offset = 0;
  for (auto& item : node.items) {
    auto& node = std::get<ast::NodePtr>(item);
    auto item_value = LLVMValueTransformer::Transform(context_, builder_, types_, symbols_, cache_, prelude_, *node);
    // TODO(acomminos): fetch tuple element pointers in aggregate
    llvm::Value* item_addr = builder_.CreateStructGEP(struct_addr, tuple_offset++);
    builder_.CreateStore(item_value, item_addr);
  }

  // Return a pointer iff the type is recursive (passed by pointer).
  if (tuple_type->isPointerTy()) {
    value_ = struct_addr;
  } else {
    value_ = builder_.CreateLoad(struct_addr);
  }

  return false;
}

}  // namespace backend
}  // namespace darlang
