#include <fstream>
#include <iostream>

#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "ast/prettyprinter.h"
#include "backend/llvm_backend.h"

// XXX(acomminos): just for printing IR
#include "llvm/Support/raw_ostream.h"

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cerr << "Usage: darlang [input-file]..." << std::endl;
  }

  darlang::Logger logger(std::cerr);
  for (int i = 1; i < argc; i++) {
    const char* filename = argv[i];

    std::ifstream is(filename);
    darlang::Lexer l(logger, is, filename);
    darlang::TokenStream ts(l);

    darlang::Parser p(logger, ts);
    auto module = p.ParseModule();

    //darlang::ast::PrettyPrinter pp;
    //module->Visit(pp);

    llvm::LLVMContext llvm_context;
    auto llvm_module = darlang::backend::LLVMModuleTransformer::Transform(llvm_context, *module);
    llvm_module->print(llvm::errs(), nullptr);
  }
}
