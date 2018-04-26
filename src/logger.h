#ifndef DARLANG_SRC_LOGGER_H_
#define DARLANG_SRC_LOGGER_H_

#include <ostream>

namespace darlang {

// A simple error logger that provides contextual information.
class Logger {
 public:
  Logger(std::ostream& os) : os_(os) {}

  void Fatal(const std::string msg, int line, int column) {
    log("fatality", msg, line, column);
    exit(1);
  }

  void Warn(const std::string msg, int line, int column) {
    log("warning", msg, line, column);
  }

 private:
  void log(const std::string level, const std::string msg, int line, int column) {
    os_ << level
        << " at " << line << ":" << column
        << " | " << msg
        << std::endl;
  }

  std::ostream& os_;
};

}  // namespace darlang

#endif  // DARLANG_SRC_LOGGER_H_
