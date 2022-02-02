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
#include "TaskClassifierSettings.h"
#include <QTimer>
#include <QThread>
#include "../../BayesGPc/CBayesianSearch.h"

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

    ClassifierCallback ClassifierDecision =
      TaskHandler(ExperCPS::ClassifierDecision_Handler);

    protected:
    void SetCPSSpecs_Handler(const CPSSpecs& new_cps_specs) {
      cps_specs = new_cps_specs;
    }

    void SetStimProfiles_Handler(
        const RC::Data1D<CSStimProfile>& new_stim_profiles);


    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
    }

    void GetNextEvent();
    void UpdateSearch(const CSStimChannel stim_info, const ExpEvent ev, const double biomarker);
    void DoConfigEvent(RC::Caller<> event);
    void DoStimEvent(RC::Caller<> event);
    void DoShamEvent();

    void Start_Handler();
    void Stop_Handler();
    void InternalStop();

    void ClassifierDecision_Handler(const double& result, const TaskClassifierSettings& task_classifier_settings);

    protected slots:
    void RunEvent();
    protected:
    void TriggerAt(uint64_t target_ms);
    void WaitUntil(uint64_t target_ms);
    void BeAllocatedTimer();

    // experiment configuration variables
    uint64_t experiment_duration; // in seconds
    size_t n_normalize_events;
    uint64_t classify_ms;
    uint64_t normalize_lockout_ms;
    uint64_t stim_lockout_ms;
    uint64_t poststim_classif_lockout_ms;

    // TODO: RDD: link to general Elemem seed
    int seed;
    int n_var;
    double obsNoise;
    double exp_bias;
    int n_init_samples;
    CKern* k;
    CCmpndKern kern;
    CWhiteKern* whitek;
    BayesianSearchModel search;

    RC::Ptr<Handler> hndl;
    RC::Ptr<StatusPanel> status_panel;
    RC::RStr buffer;

    // TODO will want to update this to an extenxible container or else ensure list is long enough for all possible events, e.g.,
    // include all potential stim events as well, potentially being recorded as non-stim events if stim wasn't 
    // ordered on basis of biomarkers
    RC::Data1D<CSStimProfile> stim_profiles;
    // set of stimulation profiles used to indicate unique stim locations
    // ordered by testing priority (unknown number of stim events per experiment)
    RC::Data1D<CSStimProfile> stim_loc_profiles;
    RC::Data1D<double> classif_results;
    RC::Data1D<ExpEvent> exp_events;
    RC::Data1D<bool> stim_event_flags;
    RC::Data1D<TaskClassifierSettings> exper_classif_settings;
    RC::Data1D<double> abs_event_times;
    uint64_t event_time;
    f64 exp_start;
    size_t cur_ev;
    uint64_t next_min_event_time;
    uint64_t classif_id;

    CPSSpecs cps_specs;



    RC::RND rng;
    RC::APtr<QTimer> timer;
  };
}

#endif // EXPERCPS_H

