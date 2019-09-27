#include "Handler.h"
#include "MainWindow.h"
#include "Popup.h"
#include "RC/RC.h"

using namespace std;
using namespace RC;


namespace CML {
  Handler::Handler() {
  }

  void Handler::SetMainWindow(Ptr<MainWindow> new_main) {
    main_window = new_main;
  }
}

