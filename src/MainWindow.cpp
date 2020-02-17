#include "About.h"
#include "ChannelSelector.h"
#include "EEGDisplay.h"
#include "GuiParts.h"
#include "MainWindow.h"
#include "Popup.h"
#include "StimGUIConfig.h"
#include "Utils.h"
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

    UnusedVar(PopupManager::GetManager());  // Initialize singleton.

    last_open_dir = GetDesktop();

    PrepareMenus();
    BuildLayout();
    SetReadyToStart_Handler(false);
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
    RC::UnusedVar(setup_menu);

    Ptr<QMenu> help_menu = menuBar()->addMenu(tr("&Help"));

    SubMenuEntry(help_menu, "&About", "Open the about page",
                 &MainWindow::HelpAboutClicked, QKeySequence::HelpContents);
  }


  RC::Ptr<QGridLayout> MainWindow::BuildStimGrid() {
    int stim_cols = 2;
    int stim_rows = 3;
    stim_config_boxes.Clear();
    RC::Ptr<QGridLayout> stim_grid = new QGridLayout();
    for (int r=0; r<stim_rows; r++) {
      for (int c=0; c<stim_cols; c++) {
        RC::Ptr<StimConfigBox> box = new StimConfigBox(hndl->SetStimSettings,
                                                       hndl->TestStim);
        stim_config_boxes += box;
        stim_grid->addWidget(box, r, c);
      }
    }

    return stim_grid;
  }


  void MainWindow::BuildLayout() {
    QWidget *central = new QWidget(this);

    // waveform_disp = new WaveformDisplay();

    RC::Ptr<QHBoxLayout> control_and_display = new QHBoxLayout();

    RC::Ptr<QVBoxLayout> stim_and_start = new QVBoxLayout();
    stim_and_start->addLayout(BuildStimGrid());

    RC::Ptr<QHBoxLayout> start_stop_buttons = new QHBoxLayout();
    start_button = new Button(hndl->StartExperiment,
                              "Start Experiment");
    start_button->SetColor({0.3f, 0.9f, 0.3f});
    stop_button = new Button(hndl->StopExperiment,
                             "Stop Experiment");
    stop_button->SetColor({1.0f, 0.2f, 0.2f});
    start_stop_buttons->addWidget(start_button);
    start_stop_buttons->addWidget(stop_button);
    stim_and_start->addLayout(start_stop_buttons);

    RC::Ptr<QHBoxLayout> eeg_and_chan = new QHBoxLayout();

    Data1D<uint16_t> channels;
    for (uint16_t c=0; c<128; c++) {
      channels += c;
    }
    hndl->eeg_acq.SetChannels(channels);

    eeg_disp = new EEGDisplay(800, 800);
    eeg_and_chan->addWidget(eeg_disp);
    hndl->eeg_acq.RegisterCallback("EEGDisplay", eeg_disp->UpdateData);

    //RC::Ptr<QVBoxLayout> test_layout = new QVBoxLayout();
    //RC::Ptr<Button> cerebus_test = new Button(hndl->CerebusTest, "Cerebus Test");
    //RC::Ptr<Button> cerestim_test = new Button(hndl->CereStimTest, "CereStim Test");

    //test_layout->addWidget(cerebus_test);
    //test_layout->addWidget(cerestim_test);

    channel_selector = new ChannelSelector(this);
    Data1D<EEGChan> demo_chans;
    for (uint32_t i=32; i<32+16; i++) {
      demo_chans += EEGChan(i);
    }
    channel_selector->SetChannels(demo_chans);
    eeg_and_chan->addWidget(channel_selector, 0);

    status_panel = new StatusPanel();

    RC::Ptr<QVBoxLayout> right_panel = new QVBoxLayout();
    right_panel->addLayout(eeg_and_chan, 1);
    right_panel->addWidget(status_panel, 0);

    control_and_display->addLayout(stim_and_start, 0);
    control_and_display->addLayout(right_panel, 1);

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


  void MainWindow::SetReadyToStart_Handler(const bool& ready) {
    start_button->SetEnabled(ready);
  }


  void MainWindow::closeEvent(QCloseEvent *event) {
    if (ConfirmWin("Are you sure you want to quit?", "Quit Elemem?")) {
      hndl->Shutdown();
      Worker::ExitAllWorkers();
      event->accept();
    }
    else {
      event->ignore();
    }
  }
}

