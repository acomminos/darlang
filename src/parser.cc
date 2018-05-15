#include "parser.h"

namespace darlang {

ast::NodePtr Parser::ParseModule() {
  auto node_module = std::make_unique<ast::ModuleNode>();
  node_module->start = location();
  while (ts_.PeekType() != Token::END_OF_FILE) {
    std::unique_ptr<ast::Node> child;
    switch (ts_.PeekType()) {
      case Token::ID:
        child = ParseDecl();
        break;
      case Token::ID_CONSTANT:
        child = ParseConstantDecl();
        break;
      default:
        // TODO(acomminos): throw error
        return node_module;
    }
    child->parent = node_module.get();
    node_module->body.push_back(std::move(child));
  }
  node_module->end = location();
  return std::move(node_module);
}

ast::NodePtr Parser::ParseDecl() {
  auto tok_id = expect_next(Token::ID);

  expect_next(Token::BRACE_START);

  std::vector<std::string> args;
  while (ts_.PeekType() != Token::BRACE_END) {
    args.push_back(expect_next(Token::ID).value);
    if (!ts_.CheckNext(Token::COMMA)) {
      break;
    }
  }

  expect_next(Token::BRACE_END);

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
      return ParseIdentExpr();
// TODO(acomminos)
//    case Token::ID_CONSTANT:
//      return ParseConstantExpr();
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
    default:
      // TODO: throw error
      break;
  }
  return nullptr;
}

ast::NodePtr Parser::ParseIdentExpr() {
  auto ident = expect_next(Token::ID);
  switch (ts_.PeekType()) {
    case Token::BRACE_START:
      ts_.PutBack(ident);
      return ParseInvoke();
    default:
      return std::make_unique<ast::IdExpressionNode>(ident.value);
  }
}

ast::NodePtr Parser::ParseGuard() {
  expect_next(Token::BLOCK_START);

  auto guard_node = std::make_unique<ast::GuardNode>();

  while (ts_.PeekType() != Token::BLOCK_END) {
    bool wildcard = false;
    ast::NodePtr cond_expr = nullptr;
    if (ts_.CheckNext(Token::WILDCARD)) {
      wildcard = true;
    } else {
      cond_expr = ParseExpr();
      cond_expr->parent = guard_node.get();
    }

    expect_next(Token::COLON);

    auto value_expr = ParseExpr();
    value_expr->parent = guard_node.get();

    if (wildcard) {
      if (guard_node->wildcard_case) {
        log_.Fatal("multiple wildcard cases specified", ts_.file(), ts_.line(), ts_.column());
      }
      guard_node->wildcard_case = std::move(value_expr);
    } else {
      guard_node->cases.push_back(
          {std::move(cond_expr), std::move(value_expr)}
      );
    }

    if (!ts_.CheckNext(Token::BREAK)) {
      break;
    }
  }

  if (!guard_node->wildcard_case) {
    log_.Fatal("no wildcard case specified", ts_.file(), ts_.line(), ts_.column());
  }

  // Allow optional trailing break.
  ts_.CheckNext(Token::BREAK);

  expect_next(Token::BLOCK_END);

  return std::move(guard_node);
}

ast::NodePtr Parser::ParseInvoke() {
  auto invoke_node = std::make_unique<ast::InvocationNode>();

  auto func_tok = expect_next(Token::ID);
  invoke_node->callee = func_tok.value;

  expect_next(Token::BRACE_START);

  while (ts_.PeekType() != Token::BRACE_END) {
    auto expr = ParseExpr();
    expr->parent = invoke_node.get();
    invoke_node->args.push_back(std::move(expr));

    if (!ts_.CheckNext(Token::COMMA)) {
      break;
    }
  }

  expect_next(Token::BRACE_END);

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
