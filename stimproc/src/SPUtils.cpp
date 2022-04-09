#include "SPUtils.h"
#include <cmath>
#include <utility>

namespace SP {
  // Remove all leading and trailing whitespace
  std::string Trim(const std::string& str) {
    const std::string trim_chars = " \t\r\n";
    size_t start, end;
    std::string retval;

    if (str.length() > 0) {
      end = str.find_last_not_of(trim_chars);
      if (end != str.npos) {
        start = str.find_first_not_of(trim_chars);
        retval = str.substr(start, end-start+1);
      }
    }

    return retval;
  }


  // Return comma-separated values with whitespace trimmed.
  std::vector<std::string> SplitCSV(const std::string& str) {
    std::vector<std::string> retval;
    size_t start, end;

    start = 0;

    while (start <= str.length()) {
      end = str.find_first_of(',', start);
      if (end == str.npos) {
        retval.push_back(Trim(str.substr(start, str.npos)));
        break;
      }
      else {
        retval.push_back(Trim(str.substr(start, end-start)));
      }
      start = end+1;
    }

    return retval;
  }


  // Remove commas and newlines from string.
  std::string CleanStr(const std::string& str) {
    std::string retval = str;
    for (char& c: retval) {
      if (c == ',' || c == '\n' || c == '\r') {
        c = ' ';
      }
    }
    return retval;
  }


  // Strict conversion.
  uint64_t To_uint64(const std::string& s) {
    static_assert(
        std::numeric_limits<unsigned long long>::max() <=
        std::numeric_limits<uint64_t>::max(), "Result could overflow.");
    std::size_t last = 0;
    uint64_t val = std::stoull(s, &last);
    // If not completely converted, or negative.
    if (last != s.size() || s.find('-') != s.npos) {
      throw std::runtime_error(CleanError("Invalid uint64_t \"", s, '"'));
    }

    return val;
  }

  // Strict conversion.
  float To_float(const std::string& s) {
    std::size_t last = 0;
    float val = std::stof(s, &last);
    // If not completely converted, nan, or inf.
    if (last != s.size() || (!std::isfinite(val)) ) {
      throw std::runtime_error(CleanError("Invalid float \"", s, '"'));
    }

    return val;
  }
}

