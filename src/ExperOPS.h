#ifndef EXPEROPS_H
#define EXPEROPS_H

#include "RC/APtr.h"
#include "RC/RStr.h"
#include "RC/Ptr.h"
#include "RC/RND.h"
#include "RCqt/Worker.h"
#include "ConfigFile.h"
#include "CereStim.h"
#include "ExpEvent.h"
#include "OPSSpecs.h"
#include <QTimer>

namespace CML {
  class Handler;
  class StatusPanel;


  class ExperOPS : public RCqt::WorkerThread, public QObject {
    public:

    ExperOPS(RC::Ptr<Handler> hndl);
    ~ExperOPS();

    // Rule of 3.
    ExperOPS(const ExperOPS&) = delete;
    ExperOPS& operator=(const ExperOPS&) = delete;

    RCqt::TaskCaller<const OPSSpecs> SetOPSSpecs =
      TaskHandler(ExperOPS::SetOPSSpecs_Handler);

    RCqt::TaskCaller<const RC::Data1D<StimProfile>> SetStimProfiles =
      TaskHandler(ExperOPS::SetStimProfiles_Handler);

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(ExperOPS::SetStatusPanel_Handler);

    RCqt::TaskCaller<> Start =
      TaskHandler(ExperOPS::Start_Handler);

    RCqt::TaskBlocker<> Stop =
      TaskHandler(ExperOPS::Stop_Handler);

    protected:
    void SetOPSSpecs_Handler(const OPSSpecs& new_ops_specs) {
      ops_specs = new_ops_specs;
    }

    void SetStimProfiles_Handler(
        const RC::Data1D<StimProfile>& new_stim_profiles);


    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
    }

    void DoConfigEvent(RC::Caller<> event);
    void DoStimEvent(RC::Caller<> event);
    void DoShamEvent();

    void Start_Handler();
    void Stop_Handler();
    void InternalStop();

    protected slots:
    void RunEvent();
    protected:
    void TriggerAt(uint64_t target_ms);
    void BeAllocatedTimer();

    RC::Ptr<Handler> hndl;
    RC::Ptr<StatusPanel> status_panel;
    RC::RStr buffer;

    RC::Data1D<StimProfile> stim_profiles;
    RC::Data1D<ExpEvent> exp_events;
    uint64_t event_time;
    f64 exp_start;
    size_t cur_ev;
    bool pre_ev;
    uint64_t next_event_time;

    OPSSpecs ops_specs;

    RC::RND rng;
    RC::APtr<QTimer> timer;
  };
}

#endif // EXPEROPS_H

