#ifndef DARLANG_SRC_LEXER_H_
#define DARLANG_SRC_LEXER_H_

#include <iostream>
#include <stack>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"

namespace darlang {

struct Token {
  // Human-readable type labels indexed by Token::Type.
  static const char* TypeNames[];

  enum Type {
    ID = 0,
    ID_CONSTANT,
    BLOCK_START,
    BLOCK_END,
    BRACE_START,
    BRACE_END,
    BREAK,
    OP_ASSIGNMENT,
    LITERAL_STRING,
    LITERAL_INTEGRAL,
    LITERAL_NUMERIC,
    COMMA,
    COLON,
    WILDCARD,
    END_OF_FILE
  } type;
  std::string value;
};

inline std::ostream& operator<<(std::ostream& os, const Token& tok) {
  return os << "{" << "type: " << Token::TypeNames[tok.type] << ", value: '" << tok.value << "'}";
}

class Lexer {
 public:
  Lexer(Logger& log, std::istream& input, const std::string filename = "unknown")
    : log_(log), input_(input), file_(filename), line_(0), column_(0) {}

  Token Next();

  std::string file() const { return file_; }
  int line() const { return line_; }
  int column() const { return column_; }

 private:
  Token ReadIdentifier();
  Token ReadNumericLiteral();
  Token ReadStringLiteral();

  // Fetches a character from the stream, updating position information.
  char getchar() {
    char c = input_.get();
    if (c == '\n') {
      line_++;
      column_ = 0;
    } else {
      column_++;
    }
    return c;
  }

  void error(const std::string msg) {
    log_.Fatal(msg, line_, column_);
  }

  void expect_next(char c) {
    auto next = input_.get();
    if (next != c) {
     std::stringstream ss;
     ss << "expected character " << c << ", got " << next;
     error(ss.str());
    }
  }

  Logger& log_;
  std::istream& input_;
  const std::string file_;
  int line_;
  int column_;
};

// A buffered proxy for a lexer.
class TokenStream {
 public:
  TokenStream(Lexer& lexer) : lexer_(lexer) {}

  Token Next() {
    Token tok;
    if (buffered_.size() > 0) {
      tok = buffered_.top();
      buffered_.pop();
    } else {
      tok = lexer_.Next();
    }

    return tok;
  }

  // Checks if the next token in the stream is of the given type- if it is,
  // consume it and return true. Otherwise, return false and do nothing.
  bool CheckNext(Token::Type type, Token* out_tok = nullptr) {
    if (PeekType() == type) {
      auto tok = Next();
      if (out_tok) {
        *out_tok = tok;
      }
      return true;
    }
    return false;
  }

  Token::Type PeekType() {
    auto t = Next();
    PutBack(t);
    return t.type;
  }

  // Puts the given token at the front of the token stream.
  void PutBack(Token t) {
    buffered_.push(t);
  }

  std::string file() const { return lexer_.file(); }
  int line() const { return lexer_.line(); }
  int column() const { return lexer_.column(); }

 private:
  Lexer& lexer_;
  std::stack<Token> buffered_;
};

}  // namespace darlang

#endif  // DARLANG_SRC_LEXER_H_
