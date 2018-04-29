#ifndef DARLANG_SRC_BACKEND_LLVM_INTRINSICS_H_
#define DARLANG_SRC_BACKEND_LLVM_INTRINSICS_H_

#include "llvm/IR/IRBuilder.h"

namespace darlang {
namespace backend {

enum Intrinsic {
  I_IS,
  I_MOD,
  I_UNKNOWN
};

Intrinsic GetIntrinsic(const std::string id) {
  if (id.compare("is") == 0) {
    return I_IS;
  }

  if (id.compare("mod") == 0) {
    return I_MOD;
  }

  return I_UNKNOWN;
}

llvm::Value* GenerateIntrinsic(Intrinsic intrinsic, std::vector<llvm::Value*> args, llvm::IRBuilder<> builder) {
  switch (intrinsic) {
    case I_IS:
      // TODO: type this for integers + floats
      assert(args.size() == 2);
      return builder.CreateICmpEQ(args[0], args[1]);
    case I_MOD:
      // TODO: type this for integers + floats, signed and unsigned?
      assert(args.size() == 2);
      return builder.CreateSRem(args[0], args[1]);
    default:
      assert(false);
  }
}

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_INTRINSICS_H_
