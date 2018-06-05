#include "parser.h"
#include "errors.h"

#include <cassert>

namespace darlang {

// A helper class to annotate the parse start and end locations of a node.
// Leverages the lifetime of the annotator to determine when parsing starts and
// ends. Should be allocated at the start of a bottom-level parse method.
class ScopedLocationAnnotator {
 public:
  ScopedLocationAnnotator(const Parser& parser, ast::Node* node = nullptr)
    : parser_(parser), node_(node)
  {
    start_ = parser.location();
  }

  ~ScopedLocationAnnotator() {
    assert(node_);

    node_->start = start_;
    node_->end = parser_.location();
  }

  ast::Node* Set(ast::Node* node) {
    node_ = node;
    return node;
  }

 private:
  const Parser& parser_;
  util::Location start_;
  ast::Node* node_;
};

ast::NodePtr Parser::ParseModule() {
  auto node_module = std::make_unique<ast::ModuleNode>();
  ScopedLocationAnnotator sla(*this, node_module.get());

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
  return std::move(node_module);
}

ast::NodePtr Parser::ParseDecl() {
  ScopedLocationAnnotator sla(*this);

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

  // FIXME(acomminos): for now, until we implement function exports, main is the
  // only non-polymorphic function permitted. set this bit accordingly.
  bool polymorphic = tok_id.value.compare("main") != 0;

  auto node = std::make_unique<ast::DeclarationNode>(tok_id.value, args, std::move(node_expr), polymorphic);
  sla.Set(node.get());

  return std::move(node);
}

ast::NodePtr Parser::ParseConstantDecl() {
  ScopedLocationAnnotator sla(*this);

  auto tok_id = expect_next(Token::ID_CONSTANT);
  expect_next(Token::OP_ASSIGNMENT);
  auto node_expr = ParseExpr();

  auto node = std::make_unique<ast::ConstantNode>(tok_id.value, std::move(node_expr));
  sla.Set(node.get());

  return std::move(node);
}

ast::NodePtr Parser::ParseExpr() {
  switch (ts_.PeekType()) {
    case Token::ID:
      return ParseIdent();
// TODO(acomminos)
//    case Token::ID_CONSTANT:
//      return ParseConstantExpr();
    case Token::BLOCK_START:
      return ParseGuard();
    case Token::BRACE_START:
      return ParseTuple();
    case Token::LITERAL_STRING:
      return ParseStringLiteral();
    case Token::LITERAL_INTEGRAL:
      return ParseIntegralLiteral();
    default:
      log_.Fatal(
        Result::Error(
          ErrorCode::TOKEN_UNEXPECTED,
          "unexpected " + std::string(Token::TypeNames[ts_.PeekType()])
        ),
        location()
      );
      return nullptr;
  }
}

ast::NodePtr Parser::ParseIdent() {
  auto ident = expect_next(Token::ID);
  auto next_type = ts_.PeekType();

  ts_.PutBack(ident);
  // If the identifier is followed by a set of arguments, parse an invocation.
  if (next_type == Token::BRACE_START) {
    return ParseInvoke();
  } else if (next_type == Token::OP_BIND) {
    return ParseBind();
  } else {
    // Otherwise, treat it as an identifier reference.
    return ParseIdentExpr();
  }
}

ast::NodePtr Parser::ParseIdentExpr() {
  ScopedLocationAnnotator sla(*this);

  auto ident = expect_next(Token::ID);
  auto node = std::make_unique<ast::IdExpressionNode>(ident.value);
  sla.Set(node.get());
  return std::move(node);
}

ast::NodePtr Parser::ParseGuard() {
  ScopedLocationAnnotator sla(*this);

  expect_next(Token::BLOCK_START);

  auto guard_node = std::make_unique<ast::GuardNode>();
  sla.Set(guard_node.get());

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
        log_.Fatal("multiple wildcard cases specified", {ts_.file(), ts_.line(), ts_.column()});
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
    log_.Fatal("no wildcard case specified", {ts_.file(), ts_.line(), ts_.column()});
  }

  // Allow optional trailing break.
  ts_.CheckNext(Token::BREAK);

  expect_next(Token::BLOCK_END);

  return std::move(guard_node);
}

ast::NodePtr Parser::ParseInvoke() {
  auto invoke_node = std::make_unique<ast::InvocationNode>();
  ScopedLocationAnnotator sla(*this, invoke_node.get());

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

ast::NodePtr Parser::ParseBind() {
  ScopedLocationAnnotator sla(*this);

  auto ident = expect_next(Token::ID);
  expect_next(Token::OP_BIND);
  auto expr = ParseExpr();
  expect_next(Token::BREAK);
  auto next_expr = ParseExpr();

  auto node = std::make_unique<ast::BindNode>(ident.value, std::move(expr), std::move(next_expr));;
  sla.Set(node.get());

  return std::move(node);
}

ast::NodePtr Parser::ParseStringLiteral() {
  auto node = std::make_unique<ast::StringLiteralNode>();
  ScopedLocationAnnotator sla(*this, node.get());

  auto ls = expect_next(Token::LITERAL_STRING);
  node->literal = ls.value;

  return std::move(node);
}

ast::NodePtr Parser::ParseIntegralLiteral() {
  auto node = std::make_unique<ast::IntegralLiteralNode>();
  ScopedLocationAnnotator sla(*this, node.get());

  auto ln = expect_next(Token::LITERAL_INTEGRAL);
  // XXX(acomminos): ensure within int64 range
  node->literal = std::stoi(ln.value);

  return std::move(node);
}

ast::NodePtr Parser::ParseTuple() {
  ScopedLocationAnnotator sla(*this);

  expect_next(Token::BRACE_START);

  std::vector<std::tuple<std::string, ast::NodePtr>> items;
  while (ts_.PeekType() != Token::BRACE_END) {
    // Handle the first symbol being a '~', denoting an attribute tag.
    if (ts_.CheckNext(Token::TAG)) {
      auto tag = expect_next(Token::ID);
      auto expr = ParseExpr();
      items.push_back({tag.value, std::move(expr)});
    } else {
      items.push_back({"", ParseExpr()});
    }
    ts_.CheckNext(Token::COMMA); // Permit trailing comma.
  }

  expect_next(Token::BRACE_END);

  auto node = std::make_unique<ast::TupleNode>(std::move(items));
  sla.Set(node.get());

  return std::move(node);
}

}  // namespace darlang
