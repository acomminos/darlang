#ifndef DARLANG_SRC_LEXER_H_
#define DARLANG_SRC_LEXER_H_

#include <iostream>
#include <string>
#include <vector>

namespace darlang {

struct Token {
  enum Type {
    ID,
    BLOCK_START,
    BLOCK_END,
    BREAK,
    OP_ASSIGNMENT,
    OP_MODULO,
    LITERAL_STRING,
    LITERAL_NUMERIC,
    END_OF_FILE
  } type;
  std::string value;
};

class Lexer {
 public:
  struct Error {
    std::string message;
    int line;
    int column;
  };

  Lexer(std::istream& input) : input_(input) {}

  Token Next();

 private:
  Token ReadIdentifier();
  Token ReadNumericLiteral();
  Token ReadStringLiteral();

  void error(const std::string msg) {
    errors_.push_back(msg);
    std::cerr << msg << std::endl;
  }

  void expect_next(char c) {
    if (input_.get() != c) {
      error("TODO"); // TODO(acomminos)
    }
  }

  std::istream& input_;
  std::vector<std::string> errors_;
};

}  // namespace darlang

#endif  // DARLANG_SRC_LEXER_H_
