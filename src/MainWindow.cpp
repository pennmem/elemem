#include "About.h"
#include "EEGDisplay.h"
#include "GuiParts.h"
#include "MainWindow.h"
#include "Popup.h"
#include "StimGUIConfig.h"
#include <QAction>
#include <QCloseEvent>
#include <QDir>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QStandardPaths>

using namespace RC;
using namespace std;


namespace CML {
  MainWindow::MainWindow(RC::Ptr<Handler> hndl)
    : hndl (hndl),
      open_config_dialog (new OpenConfigDialog(this)) {

    auto dirlist = QStandardPaths::standardLocations(
          QStandardPaths::DesktopLocation);
    if (dirlist.size() < 1) {
      last_open_dir = RC::File::CurrentDir();
    }
    else {
      last_open_dir = dirlist[0];
    }

    elemem_dir = File::FullPath(RStr{QDir::homePath()}, "ElememData");
    File::MakeDir(elemem_dir);

    PrepareMenus();
    BuildLayout();
  }


  MainWindow::~MainWindow() {
    hndl->eeg_acq.RemoveCallback("EEGDisplay");
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

    SubMenuEntry(file_menu, "&Open Config", "Open a configuration file",
                 &MainWindow::FileOpenClicked, QKeySequence::Open);
    SubMenuEntry(file_menu, "&Quit", "Exit the application",
                 &MainWindow::close, QKeySequence::Quit);

    Ptr<QMenu> setup_menu = menuBar()->addMenu(tr("&Setup"));

    Ptr<QMenu> help_menu = menuBar()->addMenu(tr("&Help"));

    SubMenuEntry(help_menu, "&About", "Open the about page",
                 &MainWindow::HelpAboutClicked, QKeySequence::HelpContents);
  }


  // TODO delete
  RC::Ptr<QGroupBox> MainWindow::BuildStimConfig() {
    // Store this label so it can be set at config load.
    RC::Ptr<QGroupBox> stim_conf_box = new QGroupBox("LA8-LQ9");

    RC::Ptr<QVBoxLayout> stim_conf = new QVBoxLayout();

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

    RC::Ptr<Button> test_stim = new Button(hndl->TestStim, "Test Stim");
    stim_conf->addWidget(test_stim);

    stim_conf_box->setLayout(stim_conf);
    return stim_conf_box;
  }


  RC::Ptr<QGridLayout> MainWindow::BuildStimGrid() {
    int stim_cols = 2;
    int stim_rows = 3;
    RC::Ptr<QGridLayout> stim_grid = new QGridLayout();
    for (int r=0; r<stim_rows; r++) {
      for (int c=0; c<stim_cols; c++) {
        // TODO Adjust for hooks for stim config
//        stim_grid->addWidget(BuildStimConfig(), r, c);
        RC::Ptr<StimConfigBox> box = new StimConfigBox(hndl->SetStimSettings,
                                                       hndl->TestStim);
        stim_grid->addWidget(box, r, c);
      }
    }

    return stim_grid;
  }


  void MainWindow::BuildLayout() {
    QWidget *central = new QWidget(this);

    // waveform_disp = new WaveformDisplay();

    RC::Ptr<QHBoxLayout> control_and_display = new QHBoxLayout();

    control_and_display->addLayout(BuildStimGrid());

    RC::Ptr<QVBoxLayout> test_layout = new QVBoxLayout();
    RC::Ptr<Button> cerebus_test = new Button(hndl->CerebusTest, "Cerebus Test");
    RC::Ptr<Button> cerestim_test = new Button(hndl->CereStimTest, "CereStim Test");

    eeg_disp = new EEGDisplay(600, 600);
    control_and_display->addWidget(eeg_disp);
    for (uint32_t i=32; i<32+8; i++) {
      EEGChan chan(i);
      eeg_disp->SetChannel(chan);
    }
    eeg_disp->ReDraw();
//    Data1D<uint16_t> channels{4, 5, 0, 1, 2};
//    for (uint16_t c=32; c<32+8; c++) {
//      channels += c;
//    }
    Data1D<uint16_t> channels;
    for (uint16_t c=0; c<128; c++) {
      channels += c;
    }
    hndl->eeg_acq.SetChannels(channels);
    hndl->eeg_acq.RegisterCallback("EEGDisplay", eeg_disp->UpdateData);

    test_layout->addWidget(cerebus_test);
    test_layout->addWidget(cerestim_test);

    control_and_display->addLayout(test_layout);

    central->setLayout(control_and_display);

    setCentralWidget(central);
  }


  void MainWindow::FileOpenClicked() {
    RC::FileRead fr = open_config_dialog->GetFile();
    SetLastOpenDir(fr.GetFilename());

    if (fr.IsOpen()) {
      hndl->OpenConfig(fr);
    }
  }


  void MainWindow::HelpAboutClicked() {
    AboutWin();
  }


  void MainWindow::SetLastOpenDir(const RStr& filename) {
    last_open_dir = RC::File::Dirname(filename);
  }


  void MainWindow::closeEvent(QCloseEvent *event) {
    if (ConfirmWin("Are you sure you want to quit?", "Quit Elemem?")) {
      Worker::ExitAllWorkers();
      event->accept();
    }
    else {
      event->ignore();
    }
  }
}

