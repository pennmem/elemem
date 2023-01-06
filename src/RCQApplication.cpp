#include "RC/RC.h"
#include "RCQApplication.h"
#include "Popup.h"
#ifndef NO_HDF5
#include <H5Cpp.h>
#endif // NO_HDF5

using namespace RC;
using namespace CML;

// Note - This robustness model was intentionally broken for Qt6 out in 2021,
// requiring a significant architectural change before any upgrade.
// Calls to QApplication::notify will simply no longer happen in the future.
// Possible strategy:  Move catches to RCqt/Worker with a registered
// exception handler, integrate timers into Worker, and clean up any possible
// exception propagation from raw gui connects.
bool RCQApplication::notify(QObject *receiver, QEvent *e) {
  try {
    return QApplication::notify(receiver, e);
  }
  CatchErrorsReturn();

  return false;
}


