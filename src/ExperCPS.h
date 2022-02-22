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
#include "CPSSpecs.h"
#include "TaskClassifierSettings.h"
#include <QTimer>
#include <QThread>
#include "../../BayesGPc/CBayesianSearch.h"

namespace CML {
  class Handler;
  class StatusPanel;

  using ClassifierCallback = RCqt::TaskCaller<const double, const TaskClassifierSettings>;
  using StimulationCallback = RCqt::TaskCaller<const bool, const TaskClassifierSettings, const f64>;

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

    StimulationCallback StimDecision =
      TaskHandler(ExperCPS::StimDecision_Handler);

    protected:
    void SetCPSSpecs_Handler(const CPSSpecs& new_cps_specs) {
      cps_specs = new_cps_specs;
    }

    void SetStimProfiles_Handler(
        const RC::Data1D<CSStimProfile>& new_stim_loc_profiles);


    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
    }

    void GetNextEvent();
    void UpdateSearch(const CSStimProfile stim_info, const ExpEvent ev, const double biomarker);
    void ComputeBestStimProfile();

    void UpdateSearchPanel(const CSStimProfile& profile);
    void ClassifyingPanel();
    void DoConfigEvent(const CSStimProfile& profile);
    void DoStimEvent(const CSStimProfile& profile);
    void DoShamEvent();
    JSONFile JSONifyCSStimProfile(const CSStimProfile& profile);

    void Start_Handler();
    void Stop_Handler();
    void InternalStop();

    void ClassifierDecision_Handler(const double& result, const TaskClassifierSettings& task_classifier_settings);
    void StimDecision_Handler(const bool& stim_event, const TaskClassifierSettings& task_classifier_settings, const f64& stim_time_from_start_sec);
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
    CMatrix bounds;
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

    // set of stimulation profiles used to indicate unique stim locations
    // ordered by testing priority (unknown number of stim events per experiment)
    RC::Data1D<CSStimProfile> stim_loc_profiles;
    CPSSpecs cps_specs;
    CSStimProfile best_stim_profile;

    // logging
    RC::Data1D<CSStimProfile> stim_profiles;
    RC::Data1D<double> classif_results;
    RC::Data1D<ExpEvent> exp_events;
    RC::Data1D<bool> stim_event_flags;
    RC::Data1D<TaskClassifierSettings> exper_classif_settings;
    RC::Data1D<double> abs_event_times;

    // temp
    uint64_t event_time;
    f64 exp_start;
    size_t cur_ev;
    bool prev_sham;
    uint64_t next_min_event_time;
    uint64_t classif_id;

    RC::RND rng;
    RC::APtr<QTimer> timer;
  };
}

#endif // EXPERCPS_H

