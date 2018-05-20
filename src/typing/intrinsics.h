#ifndef DARLANG_SRC_TYPING_INTRINSICS_H_
#define DARLANG_SRC_TYPING_INTRINSICS_H_

#include "../intrinsics.h"

namespace darlang {
namespace typing {

// Generates a partially constrained typeable from the given intrinsic ID.
// Note that intrinsics are dissimilar to global typeables, which eventually
// converge to a single unified type per identifier. Given an intrinsic type,
// multiple disjoint specializations may be typed (backend permitting).
//
// An example of this would be implementations of `is` for floats and ints.
//
// XXX: Essentially, this is a stopgap until we have true function polymorphism.
std::unique_ptr<Typeable> CreateIntrinsicTypeable(Intrinsic intrinsic);

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_INTRINSICS_H_
