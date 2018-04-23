#ifndef DARLANG_SRC_PARSER_H_
#define DARLANG_SRC_PARSER_H_

#include "ast.h"

namespace darlang {

class Parser {
 public:
   std::unique_ptr<ast::Node> ParseModule();

 private:
   std::unique_ptr<ast::Node> ParseExpr();

};

}  // namespace darlang

#endif  // DARLANG_SRC_PARSER_H_
