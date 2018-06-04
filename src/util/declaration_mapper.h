#ifndef DARLANG_SRC_UTIL_DECLARATION_MAPPER_H_
#define DARLANG_SRC_UTIL_DECLARATION_MAPPER_H_

#include "ast/types.h"

namespace darlang {
namespace util {

// A mapping from function names in a module to their appropriate AST node.
using DeclarationMap = std::unordered_map<std::string, ast::Node*>;

// Converts declarations within a module to a map from function names to nodes.
class DeclarationMapper : public ast::Visitor {
 public:
  static DeclarationMap Map(ast::Node& module_node) {
    DeclarationMapper mapper;
    module_node.Visit(&mapper);
    return mapper.map();
  }

  bool Module(ModuleNode& node) {
    for (auto& child : node.body) {
      child->Visit(*this);
    }
    return false;
  }

  bool Declaration(DeclarationNode& node) {
    assert(!map_[node.name]);
    map_[node.name] = &node;
  }

  DeclarationMap map() const { return map_; }

 private:
  std::unordered_map<std::string, ast::Node*> map_;
};

}  // namespace util
}  // namespace darlang

#endif  // DARLANG_SRC_UTIL_DECLARATION_MAPPER_H_
