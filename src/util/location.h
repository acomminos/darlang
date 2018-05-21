#ifndef DARLANG_SRC_UTIL_LOCATION_H_
#define DARLANG_SRC_UTIL_LOCATION_H_

namespace darlang {
namespace util {

// A description of a location in a source file.
struct Location {
  std::string file;
  int line;
  int column;
};

}  // namespace util
}  // namespace darlang

#endif  // DARLANG_SRC_UTIL_LOCATION_H_
