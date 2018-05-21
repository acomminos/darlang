#ifndef DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
#define DARLANG_SRC_BACKEND_LLVM_BACKEND_H_

#include <memory>
#include <unordered_map>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ast/types.h"
#include "typing/typeable.h"

namespace darlang {
namespace backend {

// Unscoped symbol table for function argument values.
typedef std::unordered_map<std::string, llvm::Value*> ArgumentSymbolTable;
// Global function table produced by the declaration transform.
typedef std::unordered_map<std::string, llvm::Function*> FunctionTable;
// Result of the type analysis phase.
typedef std::unordered_map<ast::NodeID, std::unique_ptr<typing::Typeable>> TypeablePass;

class LLVMModuleTransformer : public ast::Visitor {
 public:
  // Transforms the given AST node (presumed to be a module) into an
  // llvm::Module. Requires the result of a type synthesis pass.
  //
  // Returns nullptr on failure.
  static std::unique_ptr<llvm::Module> Transform(llvm::LLVMContext& context, const TypeablePass& typeables, ast::Node& node);

  bool Module(ast::ModuleNode& node) override;
 private:
  LLVMModuleTransformer(llvm::LLVMContext& context, const TypeablePass& typeables)
    : context_(context), typeables_(typeables) {}

  llvm::LLVMContext& context_;
  std::unique_ptr<llvm::Module> module_;
  const TypeablePass& typeables_;
};

// Transforms top-level function and constant declarations in a module.
class LLVMDeclarationTransformer : public ast::Visitor {
 public:
  LLVMDeclarationTransformer(llvm::LLVMContext& context, llvm::Module* module, const TypeablePass& typeables)
    : context_(context), module_(module), typeables_(typeables) {}

  FunctionTable& func_table() { return func_table_; }

 private:
  bool Declaration(ast::DeclarationNode& node) override;
  bool Constant(ast::ConstantNode& node) override;

  llvm::LLVMContext& context_;
  llvm::Module* module_;
  FunctionTable func_table_;
  const TypeablePass& typeables_;
};

// Generates function bodies.
class LLVMFunctionTransformer : public ast::Visitor {
 public:
  LLVMFunctionTransformer(llvm::LLVMContext& context,
                          llvm::Module* module,
                          FunctionTable& func_table,
                          const TypeablePass& typeables)
    : context_(context), module_(module), func_table_(func_table), typeables_(typeables) {}

 private:
  bool Declaration(ast::DeclarationNode& node) override;

  llvm::LLVMContext& context_;
  llvm::Module* module_;
  FunctionTable& func_table_;
  const TypeablePass& typeables_;
};

// Transforms AST nodes representing an expression into a llvm::Value* within
// the block targeted by the provided IRBuilder.
//
// Each visitor method is expected to produce instructions in one basic block,
// and leave the IRBuilder tracking a single open-ended basic block through
// which all control flow must route through.
class LLVMValueTransformer : public ast::Visitor {
 public:
  static llvm::Value* Transform(llvm::LLVMContext& context,
                                llvm::IRBuilder<>& builder,
                                ast::Node& node,
                                FunctionTable& funcs,
                                ArgumentSymbolTable& symbols,
                                const TypeablePass& typeables);

  bool IdExpression(ast::IdExpressionNode& node) override;
  bool IntegralLiteral(ast::IntegralLiteralNode& node) override;
  bool BooleanLiteral(ast::BooleanLiteralNode& node);
  bool Invocation(ast::InvocationNode& node) override;
  bool Guard(ast::GuardNode& node) override;

  llvm::Value* value() { return value_; }

 private:
  LLVMValueTransformer(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
                       FunctionTable& funcs, ArgumentSymbolTable& symbols,
                       const TypeablePass& typeables)
    : context_(context)
    , builder_(builder)
    , funcs_(funcs)
    , symbols_(symbols)
    , typeables_(typeables)
    , value_(nullptr) {}

  llvm::LLVMContext& context_;
  llvm::IRBuilder<>& builder_;
  FunctionTable& funcs_;
  ArgumentSymbolTable& symbols_;
  const TypeablePass& typeables_;
  llvm::Value* value_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
