#ifndef DARLANG_SRC_LOGGER_H_
#define DARLANG_SRC_LOGGER_H_

#include <ostream>

namespace darlang {

// A simple error logger that provides contextual information.
class Logger {
 public:
  Logger(std::ostream& os) : os_(os) {}

  void Fatal(const std::string msg, const std::string file, int line, int column) {
    log("fatality", msg, file, line, column);
    exit(1);
  }

  void Warn(const std::string msg, const std::string file, int line, int column) {
    log("warning", msg, file, line, column);
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

static Logger DefaultLogger(std::cerr);

}  // namespace darlang

#endif  // DARLANG_SRC_LOGGER_H_
