#include <fstream>
#include <iostream>

#include "lexer.h"

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cerr << "Usage: darlang [input-file]..." << std::endl;
  }

  for (int i = 1; i < argc; i++) {
    const char* filename = argv[i];

    std::ifstream is(filename);
    darlang::Lexer l(is);

    darlang::Token tok;
    do {
      tok = l.Next();
      std::cout << "{ " << "type: " << tok.type << ", value: '" << tok.value << "' }" << std::endl;
    } while (tok.type != darlang::Token::END_OF_FILE);
  }
}
