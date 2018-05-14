#ifndef DARLANG_SRC_TYPING_TYPES_H_
#define DARLANG_SRC_TYPING_TYPES_H_

namespace darlang {
namespace typing {

enum class PrimitiveType {
  Int64,
  Float,
  Boolean,
  String,
};

// TODO(acomminos): concrete type instantiations?

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPES_H_
