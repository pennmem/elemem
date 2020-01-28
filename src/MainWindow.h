#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "RC/Ptr.h"
#include "RCqt/Worker.h"
#include "Handler.h"
#include "OpenConfigDialog.h"
#include <QMainWindow>

class QGroupBox;
class QGridLayout;
class EEGDisplay;

namespace CML {
  class MainWindow : public QMainWindow, public RCqt::Worker {
    Q_OBJECT

    public:

    MainWindow(RC::Ptr<Handler> hndl);
    ~MainWindow();

    // Rule of 3;
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    RC::RStr GetLastOpenDir() const { return last_open_dir; }

    RC::Ptr<Handler> hndl;

    public slots:

    void FileOpenClicked();
    void HelpAboutClicked();

    protected:

    void PrepareMenus();
    RC::Ptr<QGroupBox> BuildStimConfig();
    RC::Ptr<QGridLayout> BuildStimGrid();
    void BuildLayout();

    void closeEvent(QCloseEvent *event);
    template<class T>
    void SubMenuEntry(RC::Ptr<QMenu> menu_entry, const RC::RStr &title,
      const RC::RStr &tip, T qt_slot,
      const QKeySequence &shortcut = QKeySequence::UnknownKey);
    void SetLastOpenDir(const RC::RStr& filename);

    RC::Ptr<EEGDisplay> eeg_disp;

    RC::APtr<OpenConfigDialog> open_config_dialog;

    RC::RStr last_open_dir;
    RC::RStr elemem_dir;
  };
}

#endif // MAINWINDOW_H

