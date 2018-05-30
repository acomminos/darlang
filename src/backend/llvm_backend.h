#ifndef DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
#define DARLANG_SRC_BACKEND_LLVM_BACKEND_H_

#include <memory>
#include <unordered_map>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ast/types.h"
#include "ast/util.h"
#include "typing/types.h"
#include "util/scoped_map.h"

namespace darlang {
namespace backend {

// Scoped symbol table for arguments, bindings, and functions.
typedef util::ScopedMap<std::string, llvm::Value*> SymbolTable;
// Result of the type analysis phase.
typedef std::unordered_map<ast::NodeID, std::unique_ptr<typing::Type>> TypeMap;

class LLVMModuleTransformer : public ast::Visitor {
 public:
  // Transforms the given AST node (presumed to be a module) into an
  // llvm::Module. Requires the result of a type synthesis pass.
  //
  // Returns nullptr on failure.
  static std::unique_ptr<llvm::Module> Transform(llvm::LLVMContext& context, TypeMap& types, ast::Node& node);

  bool Module(ast::ModuleNode& node) override;
 private:
  LLVMModuleTransformer(llvm::LLVMContext& context, TypeMap& types)
    : context_(context), types_(types) {}

  llvm::LLVMContext& context_;
  std::unique_ptr<llvm::Module> module_;
  TypeMap& types_;
};

// Transforms top-level function and constant declarations in a module.
// Writes traversed function definitions to the provided symbol table.
class LLVMDeclarationTransformer : public ast::Visitor {
 public:
  LLVMDeclarationTransformer(llvm::Module* module, TypeMap& types, SymbolTable& symbols)
    : module_(module), types_(types), symbols_(symbols) {}

 private:
  bool Declaration(ast::DeclarationNode& node) override;
  bool Constant(ast::ConstantNode& node) override;

  llvm::Module* module_;
  TypeMap& types_;
  SymbolTable& symbols_;
};

// Generates function bodies.
class LLVMFunctionTransformer : public ast::Visitor {
 public:
  LLVMFunctionTransformer(llvm::LLVMContext& context,
                          TypeMap& types,
                          const SymbolTable& symbols)
    : context_(context), types_(types), symbols_(symbols) {}

 private:
  bool Declaration(ast::DeclarationNode& node) override;

  llvm::LLVMContext& context_;
  TypeMap& types_;
  const SymbolTable& symbols_;
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
                                TypeMap& types,
                                const SymbolTable& symbols,
                                ast::Node& node);

  bool IdExpression(ast::IdExpressionNode& node) override;
  bool IntegralLiteral(ast::IntegralLiteralNode& node) override;
  bool BooleanLiteral(ast::BooleanLiteralNode& node) override;
  bool Invocation(ast::InvocationNode& node) override;
  bool Guard(ast::GuardNode& node) override;
  bool Bind(ast::BindNode& node) override;
  bool Tuple(ast::TupleNode& node) override;

  llvm::Value* value() { return value_; }

 private:
  LLVMValueTransformer(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
                       TypeMap& types, const SymbolTable& symbols)
    : context_(context)
    , builder_(builder)
    , types_(types)
    , symbols_(symbols)
    , value_(nullptr) {}

  llvm::LLVMContext& context_;
  llvm::IRBuilder<>& builder_;
  TypeMap& types_;
  const SymbolTable& symbols_;

  llvm::Value* value_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
