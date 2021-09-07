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
  catch (RC::ErrorMsgFatal& err) {
    RStr errormsg = RStr("Fatal Error:  ")+err.what();
    ErrorWin(errormsg);
    exit(-1);
  }
  catch (RC::ErrorMsgNote& err) {
    RStr errormsg = RStr(err.GetError());
    ErrorWin(errormsg);
    return true;
  }
  catch (RC::ErrorMsg& err) {
    RStr errormsg = RStr("Error:  ")+err.GetError();
    RStr logmsg = RStr("Error:  ")+err.what();
    ErrorWin(errormsg, "Error", logmsg);
    return true;
  }
#ifndef NO_HDF5
  catch (H5::Exception& ex) {
    RStr errormsg = RStr("HDF5 Error:  ")+ex.getCDetailMsg();
    ErrorWin(errormsg);
    return true;
  }
#endif // NO_HDF5
  catch (std::exception &ex) {
    RStr errormsg = RStr("Unhandled exception: ") + ex.what();
    ErrorWin(errormsg);
    return true;
  }
  catch (...) {
    RStr errormsg = "Unknown exception type";
    ErrorWin(errormsg);
    return true;
  }

  return false;
}


