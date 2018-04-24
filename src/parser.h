#ifndef DARLANG_SRC_PARSER_H_
#define DARLANG_SRC_PARSER_H_

#include "ast.h"
#include "lexer.h"

namespace darlang {

class Parser {
 public:
   Parser(TokenStream& ts) : ts_(ts) {}
   ast::NodePtr ParseModule();

 private:
   ast::NodePtr ParseDecl();
   ast::NodePtr ParseConstantDecl();
   ast::NodePtr ParseExpr();
   ast::NodePtr ParseGuard();
   ast::NodePtr ParseInvoke();
   ast::NodePtr ParseStringLiteral();
   ast::NodePtr ParseIntegralLiteral();

   Token expect_next(Token::Type type) {
     auto tok = ts_.Next();
     if (tok.type != type) {
       std::cerr << "Parser expected token " << type << ", got " << tok.type << std::endl;
       // TODO(acomminos): log error
     }
     return tok;
   }

   // TODO: error accumulator list

   TokenStream& ts_;
};

}  // namespace darlang

#endif  // DARLANG_SRC_PARSER_H_
