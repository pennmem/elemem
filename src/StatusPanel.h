#ifndef STATUSPANEL_H
#define STATUSPANEL_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "GuiParts.h"
#include <QTimer>
#include <QWidget>

namespace CML {
  class StatusPanel : public QWidget, public RCqt::Worker {
    Q_OBJECT

    public:
    StatusPanel();

    RCqt::TaskCaller<const RC::RStr> SetSubject =
      TaskHandler(StatusPanel::SetSubject_Handler);
    RCqt::TaskCaller<const bool> SetStimList =
      TaskHandler(StatusPanel::SetStimList_Handler);
    RCqt::TaskCaller<const RC::RStr> SetEvent =
      TaskHandler(StatusPanel::SetEvent_Handler);
    RCqt::TaskCaller<const uint32_t> SetStimming =
      TaskHandler(StatusPanel::SetStimming_Handler);
    RCqt::TaskCaller<const int64_t> SetSession =
      TaskHandler(StatusPanel::SetSession_Handler);
    RCqt::TaskCaller<const int64_t> SetTrial =
      TaskHandler(StatusPanel::SetTrial_Handler);
    RCqt::TaskCaller<> Clear =
      TaskHandler(StatusPanel::Clear_Handler);

    protected:
    void SetSubject_Handler(const RC::RStr& subj);
    void SetStimList_Handler(const bool& stim_list);
    void SetEvent_Handler(const RC::RStr& event);
    void SetStimming_Handler(const uint32_t& duration_us);
    void SetSession_Handler(const int64_t& session_num);
    void SetTrial_Handler(const int64_t& trial_num);
    void Clear_Handler();

    protected slots:

    void StimmingDone();

    protected:

    RC::Ptr<Indicator> subject;
    RC::Ptr<Indicator> session;
    RC::Ptr<Indicator> trial;
    RC::Ptr<Indicator> state;
    RC::Ptr<Indicator> stim_enabled;
    RC::Ptr<Indicator> stimming;

    QTimer stimming_timer;
    Color stim_on_color{1.0f, 0.0f, 0.0f};
    Color stim_off_color{0.9f, 0.9f, 0.9f};
  };
}

#endif // STATUSPANEL_H

