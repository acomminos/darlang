#ifndef DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
#define DARLANG_SRC_BACKEND_LLVM_BACKEND_H_

#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ast/types.h"

namespace darlang {
namespace backend {

class LLVMModuleTransformer : public ast::Visitor {
 public:
  // Transforms the given AST node (presumed to be a module) into an
  // llvm::Module. Returns nullptr on failure.
  static std::unique_ptr<llvm::Module> Transform(llvm::LLVMContext& context, ast::Node& node);

  void Module(ast::ModuleNode& node) override;
 private:
  LLVMModuleTransformer(llvm::LLVMContext& context) : context_(context) {}

  llvm::LLVMContext& context_;
  std::unique_ptr<llvm::Module> module_;
};

// Transforms top-level function and constant declarations in a module.
class LLVMDeclarationTransformer : public ast::Visitor {
 public:
  LLVMDeclarationTransformer(llvm::LLVMContext& context, llvm::Module* module) : context_(context), module_(module) {}

 private:
  void Declaration(ast::DeclarationNode& node) override;
  void Constant(ast::ConstantNode& node) override;

  llvm::LLVMContext& context_;
  llvm::Module* module_;
};

// Transforms AST nodes representing an expression into a llvm::Value*.
class LLVMValueTransformer : public ast::Visitor {
 public:
  static llvm::Value* Transform(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, ast::Node& node);

  /*
  void IdExpression(ast::IdExpressionNode& node);
  void IntegralLiteral(ast::IntegralLiteralNode& node);
  void StringLiteral(ast::StringLiteralNode& node);
  void BooleanLiteral(ast::BooleanLiteralNode& node);
  void Invocation(ast::InvocationNode& node);
  */
  void Guard(ast::GuardNode& node);

  llvm::Value* value() { return value_; }

 private:
  LLVMValueTransformer(llvm::LLVMContext& context, llvm::IRBuilder<>& builder) : context_(context), builder_(builder), value_(nullptr) {}

  llvm::LLVMContext& context_;
  llvm::IRBuilder<>& builder_;
  llvm::Value* value_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
