#include "Handler.h"
#include "MainWindow.h"
#include "Popup.h"
#include "RC/APtr.h"
#include "RC/Data1D.h"
#include "RC/RStr.h"
#include "RC/Errors.h"
#include "RCQApplication.h"
#include "QtStyle.h"

#include <iostream>
#include <stdlib.h>
#include <QtGui>

int main (int argc, char *argv[]) {
  int retval = -1;

  RC::Segfault::SetHandler();

  try {
    RCQApplication app(argc, argv);
    app.setStyleSheet(Resource::QtStyle_css.c_str());

    RC::APtr<CML::Handler> hndl(new CML::Handler());

    CML::MainWindow main_window(hndl);

    try { // Preserve this scope for DirectCallingScope.
      RCqt::Worker::DirectCallingScope direct;

#ifdef MACOS
      ErrorMsg::TaskPid(current_task());
#elif defined(unix)
      ErrorMsg::TaskPid(getpid());
#elif defined(WIN32)
      ErrorMsg::TaskPid(_getpid());
#else
      ErrorMsg::TaskPid(0);
#endif

      hndl->SetMainWindow(&main_window);

      hndl->LoadSysConfig();  // Must come before RegisterEEGDisplay.
      hndl->Initialize();
    }
    // We need to separately catch all the exceptions from the
    // DirectCallingScope block, as RCQApplication::notify is not used.
    catch (RC::ErrorMsgFatal& err) {
      RC::RStr errormsg = RC::RStr("Fatal Error:  ")+err.what();
      CML::ErrorWin(errormsg);
      exit(-1);
    }
    catch (RC::ErrorMsgNote& err) {
      RC::RStr errormsg = RC::RStr(err.GetError());
      CML::ErrorWin(errormsg);
    }
    catch (RC::ErrorMsg& err) {
      RC::RStr errormsg = RC::RStr("Error:  ")+err.GetError();
      RC::RStr logmsg = RC::RStr("Error:  ")+err.what();
      CML::ErrorWin(errormsg, "Error", logmsg);
    }
  #ifndef NO_HDF5
    catch (H5::Exception& ex) {
      RC::RStr errormsg = RC::RStr("HDF5 Error:  ")+ex.getCDetailMsg();
      CML::ErrorWin(errormsg);
    }
  #endif // NO_HDF5
    catch (std::exception &ex) {
      RC::RStr errormsg = RC::RStr("Unhandled exception: ") + ex.what();
      CML::ErrorWin(errormsg);
    }
    catch (...) {
      RC::RStr errormsg = "Unknown exception type";
      CML::ErrorWin(errormsg);
    }

    main_window.RegisterEEGDisplay();
    main_window.show();

    retval = app.exec();
  }
  catch (RC::ErrorMsgFatal& err) {
    std::cerr << "Fatal Error:  " << err.what() << std::endl;
    retval = -1;
    exit(-1);
  }
  catch (RC::ErrorMsg& err) {
    std::cerr << "Error:  " << err.what() << std::endl;
    retval = -2;
  }
  catch (std::exception &ex) {
    std::cerr << "Unhandled exception: " << ex.what() << std::endl;
    retval = -3;
  }

  return retval;
}

