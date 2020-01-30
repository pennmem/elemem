#include "MainWindow.h"
#include "OpenConfigDialog.h"
#include "Popup.h"
#include "RC/RC.h"
#include <QFileDialog>
#include <QString>


using namespace std;
using namespace RC;

namespace CML {
  OpenConfigDialog::OpenConfigDialog(Ptr<MainWindow> main_window)
    : main_window(main_window) {
  }


  // Returns an open read-only file pointer, or NULL if error/cancelled.
  FileRead OpenConfigDialog::GetFile() {
    FileRead fr;
    QString filename;

    filename = QFileDialog::getOpenFileName(main_window.Raw(),
               QObject::tr("Open Config File"),
               main_window->GetLastOpenDir().ToQString());

    if (filename != "") {
      if (fr.Open(filename.toStdString()) == false) {
        ErrorWin("Could not open file.");
      }
    }

    return fr;
  }
}


