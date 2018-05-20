#ifndef DARLANG_SRC_INTRINSICS_H_
#define DARLANG_SRC_INTRINSICS_H_

namespace darlang {

// A simple enumeration of all possible intrinsics.
enum class Intrinsic {
  IS,
  MOD,

  UNKNOWN,
};

// Attempts to map a string to an intrinsic.
// Returns Intrinsic::UNKNOWN if the mapping was unsuccessful.
inline Intrinsic GetIntrinsic(const std::string id) {
  // TODO(acomminos): hashing is probably faster here.
  if (id.compare("is") == 0) {
    return Intrinsic::IS;
  }

  if (id.compare("mod") == 0) {
    return Intrinsic::MOD;
  }

  return Intrinsic::UNKNOWN;
}

}  // namespace darlang

#endif  // DARLANG_SRC_INTRINSICS_H_
