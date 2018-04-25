#ifndef DARLANG_SRC_LEXER_H_
#define DARLANG_SRC_LEXER_H_

#include <iostream>
#include <stack>
#include <string>
#include <vector>

namespace darlang {

struct Token {
  enum Type {
    ID,
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

static const char* TOKEN_NAMES[] = {
  "identifier",
  "constant identifier",
  "block start",
  "block end",
  "brace start",
  "brace end",
  "break",
  "assignment",
  "string literal",
  "integral literal",
  "numeric literal",
  "comma",
  "colon",
  "wildcard",
  "eof",
};

inline std::ostream& operator<<(std::ostream& os, const Token& tok) {
  return os << "{" << "type: " << TOKEN_NAMES[tok.type] << ", value: '" << tok.value << "'}";
}

class Lexer {
 public:
  struct Error {
    std::string message;
    int line;
    int column;
  };

  Lexer(std::istream& input) : input_(input) {}

  Token Next();

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
    errors_.push_back({msg, line_, column_});
    std::cerr << msg << std::endl;
  }

  void expect_next(char c) {
    if (input_.get() != c) {
      error("TODO"); // TODO(acomminos)
    }
  }

  int line_;
  int column_;
  std::istream& input_;
  std::vector<Error> errors_;
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

 private:
  Lexer& lexer_;
  std::stack<Token> buffered_;
};

}  // namespace darlang

#endif  // DARLANG_SRC_LEXER_H_
