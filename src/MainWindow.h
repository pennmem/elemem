#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "RC/Ptr.h"
#include "RCqt/Worker.h"
#include "Handler.h"
#include <QMainWindow>

class QVBoxLayout;
class QScrollArea;

namespace CML {
  class MainWindow : public QMainWindow, public RCqt::Worker {
    Q_OBJECT

    public:

    MainWindow(RC::Ptr<Handler> hndl);

    RC::Ptr<Handler> hndl;
    
    protected:

    void PrepareMenus();
    RC::Ptr<QVBoxLayout> BuildStimConfig();
    RC::Ptr<QScrollArea> BuildStimGrid();
    void BuildLayout();

    template<class T>
    void SubMenuEntry(RC::Ptr<QMenu> menu_entry, const RC::RStr &title,
      const RC::RStr &tip, T qt_slot,
      const QKeySequence &shortcut = QKeySequence::UnknownKey);

    public slots:

    //void FileOpenClicked();

    void HelpAboutClicked();
  };
}

#endif // MAINWINDOW_H

