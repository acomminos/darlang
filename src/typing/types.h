#ifndef DARLANG_SRC_TYPING_TYPES_H_
#define DARLANG_SRC_TYPING_TYPES_H_

namespace darlang {
namespace typing {

class FunctionType {
 private:
  std::vector<Type> args_;
};

class StructType {
 private:
  std::unordered_map<std::string, Type> members_;
};

enum class Primitive {
  Int64,
  Bool,
  String,
  Struct,
  Function,
};

class PrimitiveType {
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPES_H_
