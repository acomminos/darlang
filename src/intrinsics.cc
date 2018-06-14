#include "intrinsics.h"

#include <unordered_map>

namespace darlang {

const static std::unordered_map<std::string, Intrinsic> INTRINSIC_NAMES = {
  {"is",  Intrinsic::IS},
  {"mod", Intrinsic::MOD},
  {"add", Intrinsic::ADD},
};

Intrinsic GetIntrinsic(const std::string id) {
  auto it = INTRINSIC_NAMES.find(id);
  if (it != INTRINSIC_NAMES.end()) {
    return std::get<Intrinsic>(*it);
  }
  return Intrinsic::UNKNOWN;
}

}  // namespace darlang
