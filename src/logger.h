#ifndef DARLANG_SRC_LOGGER_H_
#define DARLANG_SRC_LOGGER_H_

#include <iostream>

#include "util/location.h"

namespace darlang {

// A simple error logger that provides contextual information.
class Logger {
 public:
  Logger(std::ostream& os) : os_(os) {}

  // An unrecoverable error. Further transformation cannot continue.
  void Fatal(const std::string msg, const util::Location loc) {
    log("fatality", msg, loc.file, loc.line, loc.column);
    exit(1);
  }

  // An error that blocks compilation, but may be gracefully handled.
  void Error(const std::string msg, const util::Location loc) {
    log("error", msg, loc.file, loc.line, loc.column);
  }

  // Intended for warnings that do not prevent valid compilation.
  void Warn(const std::string msg, const util::Location loc) {
    log("warning", msg, loc.file, loc.line, loc.column);
  }

 private:
  void log(const std::string level, const std::string msg, const std::string file, int line, int column) {
    os_ << level
        << " at " << file << ":" << line << ":" << column
        << " | " << msg
        << std::endl;
  }

  std::ostream& os_;
};

static Logger ErrorLog(std::cerr);

}  // namespace darlang

#endif  // DARLANG_SRC_LOGGER_H_
