#include "About.h"
#include "MainWindow.h"
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QMenuBar>

using namespace RC;
using namespace std;


namespace CML {
  MainWindow::MainWindow(RC::Ptr<Handler> hndl)
    : hndl (hndl) {

    PrepareMenus();
    BuildLayout();
  }


  template<class T>
  void MainWindow::SubMenuEntry(Ptr<QMenu> menu_entry,
      const RStr &title, const RStr &tip, T qt_slot,
      const QKeySequence &shortcut) {
    QAction *act = new QAction(tr(title.c_str()), this);
    act->setShortcut(shortcut);
    act->setStatusTip(tr(tip.c_str()));
    connect(act, &QAction::triggered, this, qt_slot);
    menu_entry->addAction(act);
  }


  void MainWindow::PrepareMenus() {
    Ptr<QMenu> file_menu = menuBar()->addMenu(tr("&File"));

    //SubMenuEntry(file_menu, "&Open", "Open a configuration file",
    //             &MainWindow::FileOpenClicked, QKeySequence::Open);
    //SubMenuEntry(file_menu, "&Quit", "Exit the application",
    //             &MainWindow::close, QKeySequence::Quit);

    Ptr<QMenu> setup_menu = menuBar()->addMenu(tr("&Setup"));

    Ptr<QMenu> help_menu = menuBar()->addMenu(tr("&Help"));

    SubMenuEntry(help_menu, "&About", "Open the about page",
                 &MainWindow::HelpAboutClicked, QKeySequence::HelpContents);
  }


  void MainWindow::BuildLayout() {
    QWidget *central = new QWidget(this);

    // waveform_disp = new WaveformDisplay();

    QHBoxLayout *control_and_display = new QHBoxLayout();
    central->setLayout(control_and_display);

    setCentralWidget(central);
  }


  void MainWindow::HelpAboutClicked() {
    AboutWin();
  }
}

