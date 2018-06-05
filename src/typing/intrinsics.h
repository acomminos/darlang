#ifndef DARLANG_SRC_TYPING_INTRINSICS_H_
#define DARLANG_SRC_TYPING_INTRINSICS_H_

#include "typing/typeable.h"
#include "../intrinsics.h"

namespace darlang {
namespace typing {

class Specializer;

// Loads specializations of a compiler intrinsic/builtin into the provided
// specializer.
void LoadIntrinsic(Intrinsic intrinsic, Specializer& spec);

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_INTRINSICS_H_
