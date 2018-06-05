#ifndef DARLANG_SRC_TYPING_MODULE_SPECIALIZER_H_
#define DARLANG_SRC_TYPING_MODULE_SPECIALIZER_H_

#include "ast/types.h"
#include "typing/function_specializer.h"

namespace darlang::typing {

// A module specializer specializes all polymorphic implementations of a
// function in a module by performing a depth-first derivation from declared
// exports, as well as from the main function in a program module.
class ModuleSpecializer : public ast::Visitor {
 public:
  ModuleSpecializer(Logger& log, bool is_program);

  std::unordered_map<std::string, SpecializationList>& Specialize(ast::Node& node);

  bool Module(ast::ModuleNode& node) override;

 private:
  std::unordered_map<std::string, SpecializationList> specs_;
  Logger& log_;
  // If true, specializes from the "main" function as well.
  bool is_program_;
};

}  // namespace darlang::typing

#endif  // DARLANG_SRC_TYPING_MODULE_SPECIALIZER_H_
