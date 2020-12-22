#include "RC/RC.h"
#include "RCQApplication.h"
#include "Popup.h"
#include <H5Cpp.h>

using namespace RC;
using namespace CML;

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
    RStr errormsg = RStr("Error:  ")+err.what();
    ErrorWin(errormsg);
    return true;
  }
  catch (H5::Exception& ex) {
    RStr errormsg = RStr("HDF5 Error:  ")+ex.getCDetailMsg();
    ErrorWin(errormsg);
    return true;
  }
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


