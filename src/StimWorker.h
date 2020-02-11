#ifndef STIMWORKER_H
#define STIMWORKER_H

#include "CereStim.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;
  class StatusPanel;

  class StimWorker : public RCqt::WorkerThread {
    public:

    StimWorker(RC::Ptr<Handler> hndl);

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(StimWorker::SetStatusPanel_Handler);

    RCqt::TaskCaller<CSStimProfile> ConfigureStimulation =
      TaskHandler(StimWorker::ConfigureStimulation_Handler);

    RCqt::TaskCaller<> Stimulate =
      TaskHandler(StimWorker::Stimulate_Handler);

    RCqt::TaskBlocker<> CloseCereStim =
      TaskHandler(StimWorker::CloseCereStim_Handler);

    protected:
    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
    }

    void ConfigureStimulation_Handler(CSStimProfile& profile);
    void Stimulate_Handler();

    void CloseCereStim_Handler();

    RC::Ptr<Handler> hndl;
    RC::Ptr<StatusPanel> status_panel;

    CereStim cerestim;
    CSStimProfile cur_profile;

    uint32_t max_duration = 0;
  };
}

#endif // STIMWORKER_H

