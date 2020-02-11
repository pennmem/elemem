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

    RCqt::TaskCaller<const bool> SetStimList =
      TaskHandler(SetStimList_Handler);
    RCqt::TaskCaller<const RC::RStr> SetEvent =
      TaskHandler(SetEvent_Handler);
    RCqt::TaskCaller<const uint32_t> SetStimming =
      TaskHandler(SetStimming_Handler);
    RCqt::TaskCaller<const int64_t> SetSession =
      TaskHandler(SetSession_Handler);
    RCqt::TaskCaller<const int64_t> SetTrial =
      TaskHandler(SetTrial_Handler);
    RCqt::TaskCaller<> Clear =
      TaskHandler(Clear_Handler);

    protected:
    void SetStimList_Handler(const bool& stim_list);
    void SetEvent_Handler(const RC::RStr& event);
    void SetStimming_Handler(const uint32_t& duration_us);
    void SetSession_Handler(const int64_t& session);
    void SetTrial_Handler(const int64_t& trial);
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
    Color stim_on_color{1, 0, 0};
    Color stim_off_color{0.3, 0.3, 0.3};
  }
}

#endif // STATUSPANEL_H

