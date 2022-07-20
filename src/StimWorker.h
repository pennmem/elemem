#ifndef STIMWORKER_H
#define STIMWORKER_H

#include "CereStim.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  enum class StimulatorType { CereStim, Simulator };
  class Handler;
  class StatusPanel;

  class StimWorker : public RCqt::WorkerThread {
    public:

    StimWorker(RC::Ptr<Handler> hndl);

    RCqt::TaskCaller<> Open =
      TaskHandler(StimWorker::Open_Handler);

    RCqt::TaskCaller<RC::APtr<StimInterface>> SetStimInterface =
      TaskHandler(StimWorker::SetStimInterface_Handler);

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(StimWorker::SetStatusPanel_Handler);

    RCqt::TaskCaller<const StimProfile> ConfigureStimulation =
      TaskHandler(StimWorker::ConfigureStimulation_Handler);

    RCqt::TaskCaller<> Stimulate =
      TaskHandler(StimWorker::Stimulate_Handler);

    RCqt::TaskBlocker<> CloseStim =
      TaskHandler(StimWorker::CloseStim_Handler);

    StimulatorType GetStimulatorType() const;

    protected:
    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
    }

    void Open_Handler();
    void SetStimInterface_Handler(RC::APtr<StimInterface>& new_interface);
    void ConfigureStimulation_Handler(const StimProfile& profile);
    void Stimulate_Handler();

    void CloseStim_Handler();

    RC::Ptr<Handler> hndl;
    RC::Ptr<StatusPanel> status_panel;

    RC::APtr<StimInterface> stim_interface;
    StimProfile cur_profile;

    uint32_t max_duration = 0;
    const f64 stim_lockout_sec = 1.0;
    f64 prev_stim_time_sec;
  };
}

#endif // STIMWORKER_H

