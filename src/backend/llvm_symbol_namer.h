#ifndef DARLANG_SRC_BACKEND_LLVM_SYMBOL_NAMER_H_
#define DARLANG_SRC_BACKEND_LLVM_SYMBOL_NAMER_H_

#include "typing/types.h"
#include <sstream>

namespace darlang::backend {

// Symbolifies concrete darlang types into symbol names, making polymorphic
// instantiations of a function unique. Top-level declarations should use the
// "Declaration" helper to include the callee name in the symbol.
//
// The following grammar is generated:
// <function>   ::= F<name><arg-count><args>
// <tuple>      ::= T<item-count><tuple-item>
// <tuple-item> ::= <item> | e
//
// Upper case characters are control characters used for structure.
//
// Note that we don't distinguish based on return value, which is monomorphic
// within darlang (specializations are only uniqued on argument types).
//
// Example for euclid(int64, int64) -> int64:
//
//   euclid_F2ii
//
// Example for something((int64, int64), int64) -> float:
//
//   something_F2T2iii
//
class LLVMSymbolNamer : public typing::Type::Visitor {
 public:
  static std::string Name(typing::Type& type) {
    LLVMSymbolNamer namer;
    type.Visit(namer);
    return namer.value();
  }

  static std::string Declaration(std::string fname, typing::Type& type) {
    return fname + "_" + Name(type);
  }

  // Given a function name and list of argument types, returns the appropriate
  // implementation function symbol.
  static std::string Call(std::string fname, std::vector<std::unique_ptr<typing::Type>>& args) {
    return fname + "_" + FunctionSignature(args);
  }

  // Convention: use lower case identifiers to distinguish primitives.
  void Type(typing::Primitive& prim) {
    switch (prim.type()) {
      case typing::PrimitiveType::Int64:
        value_ = "i";
        break;
      case typing::PrimitiveType::Float:
        value_ = "f";
        break;
      case typing::PrimitiveType::Boolean:
        value_ = "b";
        break;
      case typing::PrimitiveType::String:
        value_ = "s";
        break;
    }
  }

  void Type(typing::Tuple& tuple) {
    std::stringstream ss;
    ss << "T";
    ss << tuple.types().size();
    for (auto& type : tuple.types()) {
      ss << LLVMSymbolNamer::Name(*std::get<std::unique_ptr<typing::Type>>(type));
    }
    value_ = ss.str();
  }

  void Type(typing::Function& func) {
    value_ = FunctionSignature(func.arguments());
  }

  std::string value() { return value_; }

 private:
  // Factored out to share code for function name generation between
  // instantiated function types and calls.
  static std::string FunctionSignature(const std::vector<std::unique_ptr<typing::Type>>& args) {
    std::stringstream ss;
    ss << "F";
    ss << args.size();
    for (auto& arg : args) {
      ss << LLVMSymbolNamer::Name(*arg);
    }
    return ss.str();
  }

  std::string value_;
};

}  // namespace darlang::backend

#endif  // DARLANG_SRC_BACKEND_LLVM_SYMBOL_NAMER_H_
