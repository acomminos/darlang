#ifndef DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
#define DARLANG_SRC_BACKEND_LLVM_BACKEND_H_

#include <memory>
#include <unordered_map>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ast/types.h"
#include "ast/util.h"
#include "typing/function_specializer.h"
#include "util/scoped_map.h"

namespace darlang {
namespace backend {

// Scoped symbol table for arguments, bindings, and functions.
typedef util::ScopedMap<std::string, llvm::Value*> SymbolTable;

class LLVMPrelude;
class LLVMTypeCache;

class LLVMModuleTransformer : public ast::Visitor {
 public:
  // Transforms the given AST node (presumed to be a module) into an
  // llvm::Module. Requires the result of a type synthesis pass.
  //
  // Returns nullptr on failure.
  static std::unique_ptr<llvm::Module> Transform(llvm::LLVMContext& context, typing::SpecializationMap& specs, ast::Node& node);

  bool Module(ast::ModuleNode& node) override;
 private:
  LLVMModuleTransformer(llvm::LLVMContext& context, typing::SpecializationMap& specs)
    : context_(context), specs_(specs) {}

  llvm::LLVMContext& context_;
  std::unique_ptr<llvm::Module> module_;
  typing::SpecializationMap& specs_;
};

// Transforms top-level function and constant declarations in a module.
// Writes traversed function definitions to the provided symbol table.
class LLVMDeclarationTransformer : public ast::Visitor {
 public:
  LLVMDeclarationTransformer(llvm::Module* module, typing::SpecializationMap& specs, SymbolTable& symbols, LLVMTypeCache& cache)
    : module_(module), specs_(specs), symbols_(symbols), cache_(cache) {}

 private:
  bool Declaration(ast::DeclarationNode& node) override;
  bool Constant(ast::ConstantNode& node) override;

  llvm::Module* module_;
  typing::SpecializationMap& specs_;
  SymbolTable& symbols_;
  LLVMTypeCache& cache_;
};

// Generates function bodies.
class LLVMFunctionTransformer : public ast::Visitor {
 public:
  LLVMFunctionTransformer(llvm::LLVMContext& context,
                          typing::SpecializationMap& specs,
                          const SymbolTable& symbols,
                          LLVMTypeCache& cache,
                          LLVMPrelude& prelude)
    : context_(context), specs_(specs), symbols_(symbols), cache_(cache)
    , prelude_(prelude) {}

 private:
  bool Declaration(ast::DeclarationNode& node) override;

  llvm::LLVMContext& context_;
  typing::SpecializationMap& specs_;
  const SymbolTable& symbols_;
  LLVMTypeCache& cache_;
  LLVMPrelude& prelude_;
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
                                typing::TypeableMap& types,
                                const SymbolTable& symbols,
                                LLVMTypeCache& cache,
                                LLVMPrelude& prelude,
                                ast::Node& node);

  bool IdExpression(ast::IdExpressionNode& node) override;
  bool IntegralLiteral(ast::IntegralLiteralNode& node) override;
  bool StringLiteral(ast::StringLiteralNode& node) override;
  bool BooleanLiteral(ast::BooleanLiteralNode& node) override;
  bool Invocation(ast::InvocationNode& node) override;
  bool Guard(ast::GuardNode& node) override;
  bool Bind(ast::BindNode& node) override;
  bool Tuple(ast::TupleNode& node) override;

  llvm::Value* value() { return value_; }

 private:
  LLVMValueTransformer(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
                       typing::TypeableMap& types, const SymbolTable& symbols,
                       LLVMTypeCache& cache, LLVMPrelude& prelude)
    : context_(context)
    , builder_(builder)
    , types_(types)
    , symbols_(symbols)
    , cache_(cache)
    , prelude_(prelude)
    , value_(nullptr) {}

  llvm::LLVMContext& context_;
  llvm::IRBuilder<>& builder_;
  typing::TypeableMap& types_;
  const SymbolTable& symbols_;
  LLVMTypeCache& cache_;
  LLVMPrelude& prelude_;

  llvm::Value* value_;
};

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_BACKEND_H_
