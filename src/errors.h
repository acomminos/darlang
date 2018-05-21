#ifndef DARLANG_SRC_ERRORS_H_
#define DARLANG_SRC_ERRORS_H_

#include <memory>
#include <sstream>

namespace darlang {

enum class ErrorCode {
  OK = 0,

  UNIMPLEMENTED,      // stub for unimplemented methods

  ID_UNDECLARED,      // reference to undeclared identifier

  TYPE_INCOMPATIBLE,  // conflicting typeable constraints set
  TYPE_INDETERMINATE, // insufficient evidence to infer a typeable's class
};

struct Result {
  static Result Ok() {
    return {ErrorCode::OK, "", nullptr};
  }

  static Result Error(ErrorCode code, const std::string message = "", std::unique_ptr<Result> child = nullptr) {
    return {code, message, std::move(child)};
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
  std::unique_ptr<Result> child;
};

}  // namespace darlang

#endif  // DARLANG_SRC_ERRORS_H_
