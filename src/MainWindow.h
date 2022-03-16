#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "RC/Ptr.h"
#include "RCqt/Worker.h"
#include "Handler.h"
#include "OpenConfigDialog.h"
#include "StatusPanel.h"
#include <QMainWindow>

class QGroupBox;
class QGridLayout;
class QStackedLayout;

namespace CML {
  class EEGDisplay;
  class ChannelSelector;
  class LocConfigBox;
  class StimConfigBox;

  class MainWindow : public QMainWindow, public RCqt::Worker {
    Q_OBJECT

    public:

    MainWindow(RC::Ptr<Handler> hndl);

    // Rule of 3;
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    RC::RStr GetLastOpenDir() const { return last_open_dir; }

    RC::Ptr<Handler> hndl;

    StimConfigBox& GetStimConfigBox(size_t i) {
      return *stim_config_boxes[i];
    }
    size_t StimConfigCount() { return stim_config_boxes.size(); }

    LocConfigBox& GetLocConfigChan() {
      return *loc_config_chans;
    }
    LocConfigBox& GetLocConfigAmp() {
      return *loc_config_amp;
    }
    LocConfigBox& GetLocConfigFreq() {
      return *loc_config_freq;
    }
    LocConfigBox& GetLocConfigDur() {
      return *loc_config_dur;
    }

    RC::Ptr<StatusPanel> GetStatusPanel() const {
      return status_panel;
    }

    RC::Ptr<EEGDisplay> GetEEGDisplay() const {
      return eeg_disp;
    }

    RC::Ptr<ChannelSelector> GetChannelSelector() const {
      return channel_selector;
    }

    RCqt::TaskCaller<> RegisterEEGDisplay =
      TaskHandler(MainWindow::RegisterEEGDisplay_Handler);

    RCqt::TaskCaller<const bool> SetReadyToStart =
      TaskHandler(MainWindow::SetReadyToStart_Handler);

    RCqt::TaskCaller<> SwitchToStimPanelFR =
      TaskHandler(MainWindow::SwitchToStimPanelFR_Handler);
    RCqt::TaskCaller<> SwitchToStimPanelLoc =
      TaskHandler(MainWindow::SwitchToStimPanelLoc_Handler);

    public slots:

    void FileOpenClicked();
    void HelpAboutClicked();

    protected:

    void PrepareMenus();
    RC::Ptr<QGroupBox> BuildStimConfig();
    RC::Ptr<QWidget> BuildStimPanelFR();
    RC::Ptr<QWidget> BuildStimPanelLoc();
    void BuildLayout();

    void closeEvent(QCloseEvent *event);
    template<class T>
    void SubMenuEntry(RC::Ptr<QMenu> menu_entry, const RC::RStr &title,
      const RC::RStr &tip, T qt_slot,
      const QKeySequence &shortcut = QKeySequence::UnknownKey);
    void SetLastOpenDir(const RC::RStr& filename);

    void RegisterEEGDisplay_Handler();

    void SetReadyToStart_Handler(const bool& ready);

    void SwitchToStimPanelFR_Handler();
    void SwitchToStimPanelLoc_Handler();

    RC::Ptr<EEGDisplay> eeg_disp;
    RC::Ptr<StatusPanel> status_panel;
    RC::Ptr<Button> start_button;
    RC::Ptr<Button> stop_button;
    RC::Ptr<ChannelSelector> channel_selector;
    RC::Ptr<QStackedLayout> stim_panels;

    RC::APtr<OpenConfigDialog> open_config_dialog;
    RC::Data1D<RC::Ptr<StimConfigBox>> stim_config_boxes;
    RC::Ptr<LocConfigBox> loc_config_chans;
    RC::Ptr<LocConfigBox> loc_config_amp;
    RC::Ptr<LocConfigBox> loc_config_freq;
    RC::Ptr<LocConfigBox> loc_config_dur;

    RC::RStr last_open_dir;
  };
}

#endif // MAINWINDOW_H

