#ifndef SPUTILS_H
#define SPUTILS_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include <climits>
#include <limits>
#include <sstream>
#include <vector>
#include <type_traits>


namespace SP {
  // Remove all leading and trailing whitespace.
  std::string Trim(const std::string& str);

  // Return comma-separated values with whitespace trimmed.
  std::vector<std::string> SplitCSV(const std::string& str);

  // Remove commas and newlines from string.
  std::string CleanStr(const std::string& str);

  inline std::string AssembleString() { return ""; }  // Base case.

  template<class T, class... Args>
  inline std::string AssembleString(T&& t, Args&&... args) {
    std::stringstream ss;
    ss << t << AssembleString(std::forward<Args>(args)...);
    return ss.str();
  }

  // Produce an error message with no commas or newlines.
  // Each argument is passed to an ostream.
  template<class... Args>
  std::string CleanError(Args&&... args) {
    return CleanStr(AssembleString(std::forward<Args>(args)...));
  }

  uint64_t To_uint64(const std::string& s);
  float To_float(const std::string& s);

  template<class T>
  void CheckedUnsignedCopy(unsigned long long val, T& t) {
    static_assert(std::is_unsigned_v<T>, "Most copy to unsigned.");
    if (val > std::numeric_limits<T>::max()) {
      throw std::runtime_error(std::to_string(val) + " is larger than the "
          "maximum allowed for the numeric type.");
    }
    t = val;
  }

  template<class T>
  void ConvertString(const std::string& str, T& t, bool onetozero=false) {
    uint64_t val = To_uint64(str);
    if (onetozero) {
      if (val == 0) {
        throw std::runtime_error("Value 0 in StimProc config where minimum "
            "allowed is 1.");
      }
      val--;
    }
    CheckedUnsignedCopy(val, t);
  }
}

#endif

