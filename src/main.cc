#include <fstream>
#include <iostream>

#include "lexer.h"
#include "parser.h"

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cerr << "Usage: darlang [input-file]..." << std::endl;
  }

  for (int i = 1; i < argc; i++) {
    const char* filename = argv[i];

    std::ifstream is(filename);
    darlang::Lexer l(is);
    darlang::TokenStream ts(l);

    darlang::Parser p(ts);
    auto module = p.ParseModule();

    /*
    darlang::Token tok;
    do {
      tok = l.Next();
      std::cout << "{ " << "type: " << tok.type << ", value: '" << tok.value << "' }" << std::endl;
    } while (tok.type != darlang::Token::END_OF_FILE);
    */
  }
}
