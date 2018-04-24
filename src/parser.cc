#include "parser.h"

namespace darlang {

// Returns if the given token type is valid lookahead for an expression value.
static inline bool is_expr_lookahead(Token::Type type) {
  return type == Token::ID ||
         type == Token::ID_CONSTANT ||
         type == Token::BRACE_START ||
         type == Token::BLOCK_START ||
         type == Token::LITERAL_STRING ||
         type == Token::LITERAL_INTEGRAL ||
         type == Token::LITERAL_NUMERIC;
}

ast::NodePtr Parser::ParseModule() {
  auto node_module = std::make_unique<ast::ModuleNode>();
  while (ts_.PeekType() != Token::END_OF_FILE) {
    switch (ts_.PeekType()) {
      case Token::ID:
        node_module->body.push_back(ParseDecl());
        break;
      case Token::ID_CONSTANT:
        node_module->body.push_back(ParseConstantDecl());
        break;
      default:
        // TODO(acomminos): throw error
        return node_module;
    }
  }
  return node_module;
}

ast::NodePtr Parser::ParseDecl() {
  auto tok_id = expect_next(Token::ID);

  std::vector<std::string> args;
  while (ts_.PeekType() == Token::ID) {
    args.push_back(ts_.Next().value);
  }

  expect_next(Token::OP_ASSIGNMENT);

  auto node_expr = ParseExpr();

  return std::make_unique<ast::DeclarationNode>(tok_id.value, args, std::move(node_expr));
}

ast::NodePtr Parser::ParseConstantDecl() {
  auto tok_id = expect_next(Token::ID_CONSTANT);
  expect_next(Token::OP_ASSIGNMENT);
  auto node_expr = ParseExpr();
  return std::make_unique<ast::ConstantNode>(tok_id.value, std::move(node_expr));
}

ast::NodePtr Parser::ParseExpr() {
  switch (ts_.PeekType()) {
    case Token::ID:
      return ParseInvoke();
//    case Token::ID_CONSTANT:
//      return ParseConstantRef();
    case Token::BLOCK_START:
      return ParseGuard();
    case Token::BRACE_START:
    {
      expect_next(Token::BRACE_START);
      auto subexpr = ParseExpr();
      expect_next(Token::BRACE_END);
      return subexpr;
    }
    case Token::LITERAL_STRING:
      return ParseStringLiteral();
    case Token::LITERAL_INTEGRAL:
      return ParseIntegralLiteral();
  }
  return nullptr;
}

ast::NodePtr Parser::ParseGuard() {
  expect_next(Token::BLOCK_START);

  auto guard_node = std::make_unique<ast::GuardNode>();

  while (ts_.PeekType() != Token::BLOCK_END) {
    auto cond_expr = ParseExpr();
    expect_next(Token::COLON);
    auto value_expr = ParseExpr();
    expect_next(Token::BREAK);
  }

  expect_next(Token::BLOCK_END);

  return std::move(guard_node);
}

ast::NodePtr Parser::ParseInvoke() {
  auto invoke_node = std::make_unique<ast::InvocationNode>();

  auto func_tok = expect_next(Token::ID);
  invoke_node->callee = func_tok.value;

  while (is_expr_lookahead(ts_.PeekType())) {
    invoke_node->args.push_back(ParseExpr());
  }

  return std::move(invoke_node);
}

ast::NodePtr Parser::ParseStringLiteral() {
  auto node = std::make_unique<ast::StringLiteralNode>();

  auto ls = expect_next(Token::LITERAL_STRING);
  node->literal = ls.value;

  return std::move(node);
}

ast::NodePtr Parser::ParseIntegralLiteral() {
  auto node = std::make_unique<ast::IntegralLiteralNode>();

  auto ln = expect_next(Token::LITERAL_INTEGRAL);
  // XXX(acomminos): ensure within int64 range
  node->literal = std::stoi(ln.value);

  return std::move(node);
}

}  // namespace darlang
