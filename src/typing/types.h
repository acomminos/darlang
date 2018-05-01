#ifndef DARLANG_SRC_TYPING_TYPES_H_
#define DARLANG_SRC_TYPING_TYPES_H_

namespace darlang {
namespace typing {

enum class Type {
  Int64,
  Bool,
  String,
  Struct,
  Function,
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPES_H_
