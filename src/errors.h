#ifndef DARLANG_SRC_ERRORS_H_
#define DARLANG_SRC_ERRORS_H_

#include <memory>
#include <sstream>

namespace darlang {

enum class ErrorCode {
  OK = 0,

  UNIMPLEMENTED,      // stub for unimplemented methods

  TOKEN_UNEXPECTED,   // unexpected token

  ID_UNDECLARED,      // reference to undeclared identifier

  TYPE_INCOMPATIBLE,  // conflicting typeable constraints set
  TYPE_INDETERMINATE, // insufficient evidence to infer a typeable's class
};

struct Result {
  static Result Ok() {
    return {ErrorCode::OK, ""};
  }

  static Result Error(ErrorCode code, const std::string message = "") {
    return {code, message};
  }

  operator bool() const {
    return code == ErrorCode::OK;
  }

  operator std::string() const {
    std::stringstream ss;
    // TODO(acomminos): map code to string
    ss << "[" << (int)code << "]" << " " << message;
    // TODO(acomminos): print child
    return ss.str();
  }

  ErrorCode code;
  std::string message;
};

// A helper for computations that may fail.
// XXX(acomminos): should consider using a union instead? Result::Ok() is valid.
template <typename T>
struct Failable {
 // Implicit constructor for a success value.
 Failable(T value) : value(std::move(value)), result(Result::Ok()) {}

 // Implicit constructor for an error.
 Failable(Result result) : result(result) {}

 operator bool() const {
   return result;
 }

 T value;
 Result result;
};

}  // namespace darlang

#endif  // DARLANG_SRC_ERRORS_H_
