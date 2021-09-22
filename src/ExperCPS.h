#ifndef EXPERCPS_H
#define EXPERCPS_H

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


  class ExperCPS : public RCqt::WorkerThread, public QObject {
    public:

    ExperCPS(RC::Ptr<Handler> hndl);
    ~ExperCPS();

    // Rule of 3.
    ExperCPS(const ExperCPS&) = delete;
    ExperCPS& operator=(const ExperCPS&) = delete;

    RCqt::TaskCaller<const CPSSpecs> SetCPSSpecs =
      TaskHandler(ExperCPS::SetCPSSpecs_Handler);

    RCqt::TaskCaller<const RC::Data1D<CSStimProfile>> SetStimProfiles =
      TaskHandler(ExperCPS::SetStimProfiles_Handler);

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(ExperCPS::SetStatusPanel_Handler);

    RCqt::TaskCaller<> Start =
      TaskHandler(ExperCPS::Start_Handler);

    RCqt::TaskBlocker<> Stop =
      TaskHandler(ExperCPS::Stop_Handler);

    protected:
    void SetCPSSpecs_Handler(const CPSSpecs& new_cps_specs) {
      cps_specs = new_cps_specs;
    }

    void SetStimProfiles_Handler(
        const RC::Data1D<CSStimProfile>& new_stim_profiles);


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

    // TODO will want to update this to a stack or else ensure list is long enough for all possible events, e.g.,
    // include all potential stim events as well, potentially being recorded as non-stim events if stim wasn't 
    // ordered on basis of biomarkers
    RC::Data1D<CSStimProfile> stim_profiles;
    RC::Data1D<ExpEvent> exp_events;
    uint64_t event_time;
    f64 exp_start;
    size_t cur_ev;
    bool pre_ev;
    uint64_t next_event_time;

    CPSSpecs cps_specs;

    RC::RND rng;
    RC::APtr<QTimer> timer;
  };
}

#endif // EXPERCPS_H

