#include "Utils.h"
#include <QDir>
#include <QStandardPaths>

namespace CML {
  RC::RStr GetHomeDir() {
      return QDir::homePath();
  }

  RC::RStr GetDesktop() {
    auto dirlist = QStandardPaths::standardLocations(
          QStandardPaths::DesktopLocation);
    RC::RStr desktop;
    if (dirlist.size() < 1) {
      // Fallback to user directory if desktop not found.
      desktop = QDir::homePath();
    }
    else {
      desktop = dirlist[0];
    }
    return desktop;
  }

  // This is integer division that returns the ceiling
  int CeilDiv(int dividend, int divisor) {
    return dividend / divisor + (dividend % divisor != 0); 
  }

  // This is integer division that returns the ceiling
  size_t CeilDiv(size_t dividend, size_t divisor) {
    return dividend / divisor + (dividend % divisor != 0); 
  }
}

