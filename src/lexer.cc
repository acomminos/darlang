#include "lexer.h"

#include <sstream>

namespace darlang {

static bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z');
}

static bool is_numeric(char c) {
  return c >= '0' && c <= '9';
}

Token Lexer::Next() {
  char c = input_.peek();

  // Skip whitespace.
  if (c == ' ' || c == '\t' || c == '\n') {
    input_.get();
    return Next();
  }

  if (c == EOF) {
    return {Token::END_OF_FILE};
  }

  if (is_alpha(c) || c == '_') {
    return ReadIdentifier();
  }

  if (is_numeric(c)) {
    return ReadNumericLiteral();
  }

  if (c == '"') {
    return ReadStringLiteral();
  }

  input_.get();
  switch (c) {
    case ':':
      return {Token::OP_ASSIGNMENT};
    case '%':
      return {Token::OP_MODULO};
    case ';':
      return {Token::BREAK};
    case '{':
      return {Token::BLOCK_START};
    case '}':
      return {Token::BLOCK_END};
  }

  // Log unknown character, skip.
  // TODO(acomminos): factor this out
  std::stringstream buf;
  buf << "unknown character: " << c;
  error(buf.str());
  return Next();
}

Token Lexer::ReadIdentifier() {
  char c = input_.peek();
  std::string value;
  while (is_alpha(c) || is_numeric(c) || c == '_') {
    value += input_.get();
    c = input_.peek();
  }
  return {Token::ID, value};
}

Token Lexer::ReadNumericLiteral() {
  char c = input_.peek();
  bool has_dot = false;
  std::string value;
  while (is_numeric(c) || c == '.') {
    if (c == '.') {
      if (has_dot) {
        error("unexpected second decimal");
        // TODO(acomminos): log error
      } else {
        has_dot = true;
      }
    }
    value += c;
    input_.get();
    c = input_.peek();
  }
  return {Token::LITERAL_NUMERIC, value};
}

Token Lexer::ReadStringLiteral() {
  expect_next('"');

  char c = input_.peek();
  std::string literal;
  bool escape = false;
  while (escape || c != '"') {
    if (c == '\\' && !escape) {
      escape = true;
    } else {
      literal += c;
      escape = false;
    }
    input_.get();
    c = input_.peek();
  }

  expect_next('"');

  return {Token::LITERAL_STRING, literal};
}

}  // namespace darlang
