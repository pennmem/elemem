#include "Utils.h"
#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>

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

  // True if the amount of storage exists at the specified path.
  // Throws an exception if the path does not exist or cannot be written to.
  bool CheckStorage(RC::RStr path, size_t amount) {
    QStorageInfo storage(path.ToQString());
    int64_t avail = storage.bytesAvailable();
    if ( storage.isReadOnly() || (avail < 0) ||
         (! (storage.isValid() && storage.isReady())) ) {
      Throw_RC_Type(File, (RC::RStr("Cannot write to ") + path).c_str());
    }
    return size_t(avail) >= amount;
  }
}

