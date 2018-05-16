#ifndef DARLANG_SRC_ERRORS_H_
#define DARLANG_SRC_ERRORS_H_

namespace darlang {

enum class ErrorCode {
  OK = 0,

  // type inference
  TYPE_INCOMPATIBLE,
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

  ErrorCode code;
  std::string message;
  std::unique_ptr<Result> child;
};

}  // namespace darlang

#endif  // DARLANG_SRC_ERRORS_H_
