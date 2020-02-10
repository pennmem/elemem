#ifndef STATUSPANEL_H
#define STATUSPANEL_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "GuiParts.h"
#include <QLayout>

namespace CML {
  class StatusPanel : public QLayout, public RCqt::Worker {
    Q_OBJECT

    public:
    StatusPanel();

    RCqt::TaskCaller<const bool> SetStimList =
      TaskHandler(SetStimList_Handler);
    RCqt::TaskCaller<const RC::RStr> SetEvent =
      TaskHandler(SetEvent_Handler);
    RCqt::TaskCaller<const bool> SetStimming =
      TaskHandler(SetStimming_Handler);

    protected:
    void SetStimList_Handler(const bool& stim_list);
    void SetEvent_Handler(const RC::RStr& event);
    void SetStimming_Handler(const bool& stimming);

//    RC::Ptr<Indicator> subject;
//    RC::Ptr<Indicator> session;
//    RC::Ptr<Indicator> trial;
//    RC::Ptr<Indicator> state;
//    RC::Ptr<Indicator> stim_enabled;
//    RC::Ptr<Indicator> stimming;
  }
}

#endif // STATUSPANEL_H

