#include "About.h"
#include "ChannelSelector.h"
#include "EEGDisplay.h"
#include "GuiParts.h"
#include "MainWindow.h"
#include "Popup.h"
#include "LocGUIConfig.h"
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
#include <QStackedLayout>
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


  RC::Ptr<QWidget> MainWindow::BuildStimPanelFR() {
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

    RC::Ptr<QWidget> stim_panel = new QWidget();
    stim_panel->setLayout(stim_grid);
    return stim_panel;
  }


  RC::Ptr<QWidget> MainWindow::BuildStimPanelCPS() {
    int stim_cols = 2;
    int stim_rows = 10;
    min_max_stim_config_boxes.Clear();
    RC::Ptr<QGridLayout> stim_grid = new QGridLayout();
    for (int r=0; r<stim_rows; r++) {
      for (int c=0; c<stim_cols; c++) {
        RC::Ptr<MinMaxStimConfigBox> box = new MinMaxStimConfigBox(hndl->SetStimSettings,
                                                                   hndl->TestStim);
        min_max_stim_config_boxes += box;
        stim_grid->addWidget(box, r, c);
      }
    }

    RC::Ptr<QWidget> stim_panel = new QWidget();
    stim_panel->setLayout(stim_grid);
    return stim_panel;
  }


  RC::Ptr<QWidget> MainWindow::BuildStimPanelLoc() {
    RC::Ptr<QWidget> stimloc_panel = new QWidget();

    RC::Ptr<QVBoxLayout> stimloc_layout = new QVBoxLayout();

    loc_config_chans = new LocConfigBox(hndl->TestSelLocChan,
        hndl->SetLocChansApproved, "Channels");
    loc_config_amp = new LocConfigBox(hndl->TestSelLocAmp,
        hndl->SetLocAmpApproved, "Amplitudes");
    loc_config_freq = new LocConfigBox(hndl->TestSelLocFreq,
        hndl->SetLocFreqApproved, "Frequencies");
    loc_config_dur = new LocConfigBox(hndl->TestSelLocDur,
        hndl->SetLocDurApproved, "Durations");

    stimloc_layout->addWidget(loc_config_chans);
    stimloc_layout->addWidget(loc_config_amp);
    stimloc_layout->addWidget(loc_config_freq);
    stimloc_layout->addWidget(loc_config_dur);

    RC::Ptr<QHBoxLayout> button_layout = new QHBoxLayout();
    RC::Ptr<Button> test_stim = new Button(hndl->TestLocStim,
        "Test Stimulation");
    button_layout->addWidget(test_stim);
    button_layout->addWidget(new QWidget());
    stimloc_layout->addLayout(button_layout);

    stimloc_layout->addWidget(new QWidget());
    stimloc_panel->setLayout(stimloc_layout);

    return stimloc_panel;
  }



  void MainWindow::BuildLayout() {
    QWidget *central = new QWidget(this);

    RC::Ptr<QHBoxLayout> control_and_display = new QHBoxLayout();

    RC::Ptr<QVBoxLayout> stim_and_start = new QVBoxLayout();

    stim_panels = new QStackedLayout();


    RC::Ptr<QScrollArea> FR_panel_scroll = new QScrollArea();
    FR_panel_scroll->setWidget(BuildStimPanelFR());
    FR_panel_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    FR_panel_scroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    stim_panels->addWidget(FR_panel_scroll);

    RC::Ptr<QScrollArea> Loc_panel_scroll = new QScrollArea();
    Loc_panel_scroll->setWidget(BuildStimPanelLoc());
    Loc_panel_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    stim_panels->addWidget(Loc_panel_scroll);

    RC::Ptr<QScrollArea> CPS_panel_scroll = new QScrollArea();
    CPS_panel_scroll->setWidget(BuildStimPanelCPS());
    CPS_panel_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    CPS_panel_scroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    stim_panels->addWidget(CPS_panel_scroll);

    stim_and_start->addLayout(stim_panels);

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

    eeg_disp = new EEGDisplay(800, 800);
    eeg_and_chan->addWidget(eeg_disp);

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


  void MainWindow::RegisterEEGDisplay_Handler() {
    hndl->eeg_acq.RegisterCallback("EEGDisplay", eeg_disp->UpdateData);
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

  void MainWindow::SwitchToStimPanelFR_Handler() {
    stim_panels->setCurrentIndex(0);
  }

  void MainWindow::SwitchToStimPanelLoc_Handler() {
    stim_panels->setCurrentIndex(1);
  }

  void MainWindow::SwitchToStimPanelCPS_Handler() {
    stim_panels->setCurrentIndex(2);
  }


  void MainWindow::closeEvent(QCloseEvent *event) {
    if (ConfirmWin("Are you sure you want to quit?", "Quit Elemem?")) {
      hndl->Shutdown();
      hndl->eeg_acq.RemoveCallback("EEGDisplay");
      Worker::ExitAllWorkers();
      event->accept();
    }
    else {
      event->ignore();
    }
  }
}

