#include "Utils.h"
#include <QDir>
#include <QStandardPaths>

namespace CML {
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
}

