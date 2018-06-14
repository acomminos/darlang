#ifndef DARLANG_SRC_INTRINSICS_H_
#define DARLANG_SRC_INTRINSICS_H_

#include <string>

namespace darlang {

// A simple enumeration of all possible intrinsics.
enum class Intrinsic {
  IS,
  MOD,
  ADD,

  UNKNOWN,
};

// Attempts to map a string to an intrinsic.
// Returns Intrinsic::UNKNOWN if the mapping was unsuccessful.
Intrinsic GetIntrinsic(const std::string id);

}  // namespace darlang

#endif  // DARLANG_SRC_INTRINSICS_H_
