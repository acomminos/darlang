#ifndef DARLANG_SRC_PARSER_H_
#define DARLANG_SRC_PARSER_H_

#include <sstream>

#include "ast/types.h"
#include "lexer.h"
#include "logger.h"

namespace darlang::parsing {

class Parser {
 public:
   Parser(Logger& log, TokenStream& ts) : log_(log), ts_(ts) {}
   ast::NodePtr ParseModule();

   // Returns the current position within the source file, formatted as a
   // util::Location.
   util::Location location() const {
     return {ts_.file(), ts_.line(), ts_.column()};
   }

 private:
   ast::NodePtr ParseDecl();
   ast::NodePtr ParseConstantDecl();
   ast::NodePtr ParseExpr();
   ast::NodePtr ParseIdent(); // parses an expression prefixed by an id
   ast::NodePtr ParseIdentExpr();
   ast::NodePtr ParseGuard();
   ast::NodePtr ParseInvoke();
   ast::NodePtr ParseBind();
   ast::NodePtr ParseStringLiteral();
   ast::NodePtr ParseIntegralLiteral();
   ast::NodePtr ParseTuple();

   Token expect_next(Token::Type type) {
     auto tok = ts_.Next();
     if (tok.type != type) {
       std::stringstream ss;
       ss << "expected token " << Token::TypeNames[type] << ", got " << Token::TypeNames[tok.type];
       log_.Fatal(ss.str(), {ts_.file(), ts_.line(), ts_.column()});
     }
     return tok;
   }

   Logger& log_;
   TokenStream& ts_;
};

}  // namespace darlang::parsing

#endif  // DARLANG_SRC_PARSER_H_
