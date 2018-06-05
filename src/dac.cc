#include <fstream>
#include <iostream>

#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "backend/llvm_backend.h"
#include "typing/type_transform.h"
#include "typing/module_specializer.h"

// XXX(acomminos): just for printing IR
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

int main(int argc, char* argv[]) {
  llvm::cl::opt<std::string> input_file(llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::init("-"));
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

  darlang::Lexer l(logger, *input, input_file);
  darlang::TokenStream ts(l);

  darlang::Parser p(logger, ts);
  auto module = p.ParseModule();

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
