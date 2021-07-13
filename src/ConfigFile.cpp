#include "ConfigFile.h"

namespace CML {
  // Removes # and // comments, left/right whitespace, and blank lines.
  void RemoveComments(RC::Data1D<RC::RStr>& lines) {
    for (size_t i=0; i<lines.size(); i++) {
      bool in_quote = false;
      bool escaped = false;
      bool start_comment = false;
      auto& line = lines[i];
      for (size_t c=0; c<line.size(); c++) {
        if (in_quote) {
          if (escaped) {
            escaped = false;
          }
          else if (line[c] == '"') {
            in_quote = false;
          }
          else if (line[c] == '\\') {
            escaped = true;
          }
        }
        else {
          if (start_comment) {
            if (line[c] == '/') {
              line = line.substr(0, c-1);
              break;
            }
            start_comment = false;
          }
          else {
            if (line[c] == '/') {
              start_comment = true;
            }
            else if (line[c] == '#') {
              line = line.substr(0, c);
              break;
            }
            else if (line[c] == '"') {
              in_quote = true;
            }
          }
        }
      }

      line.Trim();
      if (line.empty()) {
        lines.Remove(i);
        i--;
      }
    }
  }

  void RemoveBOM(RC::RStr& line) {
    if (line.size() >= 3 &&
        line[0] == '\xef' && line[1] == '\xbb' && line[2] == '\xbf') {
      line = line.substr(3);
    }
  }
}

