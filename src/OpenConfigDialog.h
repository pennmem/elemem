#ifndef OPENCONFIGDIALOG_H
#define OPENCONFIGDIALOG_H

#include "RC/RC.h"


namespace CML {
  class MainWindow;

  class OpenConfigDialog {
    public:

    OpenConfigDialog(RC::Ptr<MainWindow>);
    RC::FileRead GetFile();

    protected:

    RC::Ptr<MainWindow> main_window;
  };
}



#endif // OPENCONFIGDIALOG_H

