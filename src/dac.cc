#include <fstream>
#include <iostream>

#include "logger.h"
#include "parsing/lexer.h"
#include "parsing/parser.h"
#include "backend/llvm_backend.h"
#include "typing/type_transform.h"
#include "typing/module_specializer.h"
#include "ast/prettyprinter.h"

// XXX(acomminos): just for printing IR
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

int main(int argc, char* argv[]) {
  llvm::cl::opt<std::string> input_file(llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::init("-"));
  llvm::cl::opt<bool> print_ast("print-ast", llvm::cl::desc("pretty prints the AST instead of doing anything useful"), llvm::cl::init(false));

  llvm::cl::ParseCommandLineOptions(argc, argv);

  darlang::Logger logger(std::cerr);

  // Read from stdin if '-' is specified as input.
  std::istream* input;
  if (input_file.compare("-") == 0) {
    input = &std::cin;
  } else {
    input = new std::ifstream(input_file);
  }

  if (!*input) {
    std::cerr << "failed to open " << input_file << std::endl;
    return 1;
  }

  darlang::parsing::Lexer l(logger, *input, input_file);
  darlang::parsing::TokenStream ts(l);

  darlang::parsing::Parser p(logger, ts);
  auto module = p.ParseModule();

  if (print_ast) {
    darlang::ast::PrettyPrinter pp;
    module->Visit(pp);
    return 0;
  }

  darlang::typing::ModuleSpecializer specializer(logger, true);
  auto types = specializer.Specialize(*module);

  llvm::LLVMContext llvm_context;
  auto llvm_module = darlang::backend::LLVMModuleTransformer::Transform(llvm_context, types, *module);
  llvm_module->print(llvm::errs(), nullptr);

  /*
  // XXX(acomminos): hackish object code generation
  auto target_triple = llvm::sys::getDefaultTargetTriple();
  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    std::cerr << error << std::endl;
    return 1;
  }
  auto target_machine = target->createTargetMachine(target_triple, "generic", "", llvm::Optional<llvm::Reloc::Model>());
  llvm_module->setDataLayout(target_machine->createDataLayout());
  llvm_module->setTargetTriple(target_triple);

  std::string obj_filename = std::string(filename) + ".o";
  */
  delete input;
}
