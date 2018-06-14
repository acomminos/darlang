#ifndef DARLANG_SRC_BACKEND_LLVM_INTRINSICS_H_
#define DARLANG_SRC_BACKEND_LLVM_INTRINSICS_H_

#include "llvm/IR/IRBuilder.h"
#include "intrinsics.h"

namespace darlang {
namespace backend {

llvm::Value* GenerateIntrinsic(Intrinsic intrinsic, std::vector<llvm::Value*> args, llvm::IRBuilder<> builder) {
  switch (intrinsic) {
    case Intrinsic::IS:
      // TODO: type this for integers + floats
      assert(args.size() == 2);
      return builder.CreateICmpEQ(args[0], args[1]);
    case Intrinsic::MOD:
      // TODO: type this for integers + floats, signed and unsigned?
      assert(args.size() == 2);
      return builder.CreateSRem(args[0], args[1]);
    case Intrinsic::ADD:
      // TODO: type this for integers + floats, signed and unsigned?
      assert(args.size() == 2);
      return builder.CreateAdd(args[0], args[1]);
    default:
      assert(false);
  }
}

}  // namespace backend
}  // namespace darlang

#endif  // DARLANG_SRC_BACKEND_LLVM_INTRINSICS_H_
