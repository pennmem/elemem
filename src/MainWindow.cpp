#include "About.h"
#include "GuiParts.h"
#include "MainWindow.h"
#include <QAction>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
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


  RC::Ptr<QVBoxLayout> MainWindow::BuildStimConfig() {
    RC::Ptr<QVBoxLayout> stim_conf = new QVBoxLayout();

    // Store this label so it can be set at config load.
    stim_conf->addWidget(new QLabel("LA8-LA9"));

    RC::Ptr<QHBoxLayout> anode_cath = new QHBoxLayout();
    anode_cath->addWidget(new QLabel("Anode"));
    anode_cath->addWidget(new QLabel("[89]"));
    anode_cath->addWidget(new QLabel("Cathode"));
    anode_cath->addWidget(new QLabel("[90]"));
    stim_conf->addLayout(anode_cath);

    // Store these, give individual callbacks
    RC::Ptr<LabeledF64> amp = new LabeledF64(hndl->TestLabel, "Amplitude");
    stim_conf->addWidget(amp);
    RC::Ptr<LabeledF64> freq = new LabeledF64(hndl->TestLabel, "Frequency");
    stim_conf->addWidget(freq);
    RC::Ptr<LabeledF64> dur = new LabeledF64(hndl->TestLabel, "Duration");
    stim_conf->addWidget(dur);

    return stim_conf;
  }


  RC::Ptr<QScrollArea> MainWindow::BuildStimGrid() {
    int stim_cols = 2;
    int stim_rows = 3;
    RC::Ptr<QGridLayout> stim_grid = new QGridLayout();
    for (int r=0; r<stim_rows; r++) {
      for (int c=0; c<stim_cols; c++) {
        // TODO Adjust for hooks for stim config
        stim_grid->addLayout(BuildStimConfig(), r, c);
      }
    }

    RC::Ptr<QScrollArea> border = new QScrollArea();
    border->setFrameStyle(QFrame::Panel);
    RC::Ptr<QWidget> container = new QWidget();
    container->setLayout(stim_grid);
    border->setWidget(container);
    return border;
  }


  void MainWindow::BuildLayout() {
    QWidget *central = new QWidget(this);

    // waveform_disp = new WaveformDisplay();

    RC::Ptr<QHBoxLayout> control_and_display = new QHBoxLayout();

    control_and_display->addWidget(BuildStimGrid());

    RC::Ptr<QVBoxLayout> test_layout = new QVBoxLayout();
    RC::Ptr<Button> cerebus_test = new Button(hndl->CerebusTest, "Cerebus Test");
    RC::Ptr<Button> cerestim_test = new Button(hndl->CereStimTest, "CereStim Test");

    test_layout->addWidget(cerebus_test);
    test_layout->addWidget(cerestim_test);

    control_and_display->addLayout(test_layout);

    central->setLayout(control_and_display);

    setCentralWidget(central);
  }


  void MainWindow::HelpAboutClicked() {
    AboutWin();
  }
}

