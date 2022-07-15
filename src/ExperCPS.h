#ifndef EXPERCPS_H
#define EXPERCPS_H

#include "RC/APtr.h"
#include "RC/RStr.h"
#include "RC/Ptr.h"
#include "RC/RND.h"
#include "RCqt/Worker.h"
#include "ConfigFile.h"
#include "StimInterface.h"
#include "ExpEvent.h"
#include "CPSSpecs.h"
#include "TaskClassifierSettings.h"
#include "EEGPowers.h"
#include <QThread>
#include "BayesGPc/CBayesianSearch.h"
#include "BayesGPc/CSearchComparison.h"
#include "nlohmann/json.hpp"

#define DEBUG_EXPERCPS

namespace CML {
  // experiment event definitions
  class NormalizeEvent {
    public:
    f64 start_time;  // start of feature filtering duration, ms
  };

  class ClassifyEvent {
    public:
    f64 start_time;  // start of classification duration, ms
    double result;  // closed-loop control result (e.g. classifier predicted probability of recall)
    bool decision;  // whether stim was applied in response to the closed-loop control result
    TaskClassifierSettings settings;
  };

  class StimEvent {
    public:
    f64 start_time;  // start of classification duration, ms
    StimProfile profile;
  };

  class ShamEvent {
    public:
    f64 start_time;  // start of classification duration, ms
    uint64_t duration;  // ms
  };

  class UpdateEvent {
    public:
    f64 start_time;  // start of pre-stim classification duration, ms
    unsigned int model_idx;  // index into fixed stimulation locations/parameter sets/optimization models
    StimProfile stim_params;  // full stim parameters for update
    vector<vector<double>> opt_params;  // just optimized parameter values (for convenience)
    double biomarker;  // biomarker for update
    json state;  // optimization algorithm state after update
    bool loaded;  // whether update was loaded from a previous session
  };

  class Handler;
  class StatusPanel;

  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const EEGPowers>, const TaskClassifierSettings>;
  using ClassifierCallback = RCqt::TaskCaller<const double, const TaskClassifierSettings>;
  using StimulationCallback = RCqt::TaskCaller<const bool, const TaskClassifierSettings, const f64>;

  class ExperCPS : public RCqt::WorkerThread, public QObject {
    public:

    ExperCPS(RC::Ptr<Handler> hndl);
    void _init();

    // Rule of 3.
    ExperCPS(const ExperCPS&) = delete;
    ExperCPS& operator=(const ExperCPS&) = delete;

    RCqt::TaskCaller<const CPSSpecs> SetCPSSpecs =
      TaskHandler(ExperCPS::SetCPSSpecs_Handler);

    RCqt::TaskCaller<const RC::Data1D<StimProfile>, const RC::Data1D<StimProfile>> SetStimProfiles =
      TaskHandler(ExperCPS::SetStimProfiles_Handler);

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(ExperCPS::SetStatusPanel_Handler);


    // I know, I know. But I spent hours trying to pass a Data1D<RC::RStr> into Setup() to no avail...
    // either arrays were empty after I passed them in (they were non-empty a few lines before) or
    // I got a static compiler message when I tried to pass in a non-reference variable
    RCqt::TaskCaller<const RC::Data1D<RC::RStr>> Setup =
      TaskHandler(ExperCPS::Setup_Handler);

    RCqt::TaskCaller<const uint64_t> Start =
      TaskHandler(ExperCPS::Start_Handler);

    RCqt::TaskBlocker<> Stop =
      TaskHandler(ExperCPS::Stop_Handler);

    FeatureCallback HandleNormalization =
      TaskHandler(ExperCPS::HandleNormalization_Handler);

    ClassifierCallback ClassifierDecision =
      TaskHandler(ExperCPS::ClassifierDecision_Handler);

    StimulationCallback StimDecision =
      TaskHandler(ExperCPS::StimDecision_Handler);

    void Setup_Handler(const RC::Data1D<RC::RStr>& prev_sessions);

    protected:
    void SetCPSSpecs_Handler(const CPSSpecs& new_cps_specs) {
      cps_specs = new_cps_specs;
    }

    void SetStimProfiles_Handler(
        const RC::Data1D<StimProfile>& new_min_stim_loc_profiles,
        const RC::Data1D<StimProfile>& new_max_stim_loc_profiles);


    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
    }

    void GetNextEvent(unsigned int model_idx);
    void UpdateSearch(
        const unsigned int model_idx,
        const StimProfile stim_profile,
        const double biomarker);
    void ComputeBestStimProfile();

    void UpdateSearchPanel();
    void NormalizingPanel();
    void ClassifyingPanel();
    void DoConfigEvent(const StimProfile& profile, const unsigned int model_idx);
    bool ValidateStimulationProfile(const StimProfile& profile, const unsigned int profile_idx);
    void DoStimEvent(const StimProfile& profile);
    void DoShamEvent();

    void LogNormalize(NormalizeEvent ev);
    void LogClassify(ClassifyEvent ev);
    void LogSham(ShamEvent ev);
    void LogUpdate(UpdateEvent ev);
    JSONFile JSONifyStimProfile(const StimProfile& profile);
    // TODO: RDD: move somewhere Elemem-general, use in StimWorker?
    JSONFile StimChannel2JSON(StimChannel chan);
    StimChannel JSON2StimChannel(JSONFile j);

    void Start_Handler(const uint64_t& duration_s);
    void Stop_Handler();
    void InternalStop();

    // experimental event handlers
    void HandleNormalization_Handler(
        RC::APtr<const EEGPowers>& data,
        const TaskClassifierSettings& task_classifier_settings);
    void ClassifierDecision_Handler(
        const double& result,
        const TaskClassifierSettings& task_classifier_settings);
    void StimDecision_Handler(
        const bool& stim_event,
        const TaskClassifierSettings& task_classifier_settings,
        const f64& stim_time_from_start_sec);

    protected slots:
    void RunEvent();
    protected:
    uint64_t TimeSinceExpStartMs();
    f64 ToAbsoluteTime(uint64_t time_ms);
    uint64_t WaitUntil(uint64_t target_time_ms);
    void TriggerAt(
        const uint64_t& next_min_event_time,
        const ClassificationType& next_classif_state);
    void BeAllocatedTimer();

    // core logic for handling incoming stim event and triggering next stim event
    // ProcessEvent() avoids race condition between ClassifierDecision() and StimDecision() vs. just putting logic in StimDecision()
    void ProcessEvent();
    template <typename T>
    void ShuffleNoConsecutive(RC::Data1D<T>& d);

    // state variables for triggering ProcessEvent()
    bool classif_decision_arrived = false;
    bool stim_decision_arrived = false;

    // experiment configuration variables
    uint64_t experiment_duration; // in seconds
    // number of events for normalizing EEG features
    size_t n_normalize_events;
    // classification interval duration
    uint64_t classify_ms;
    // lockout period between stim offset and post-stim classification interval onset
    uint64_t poststim_classif_lockout_ms;

    int seed;
    int n_var;
    vector<CMatrix> param_bounds;
    double obsNoise;
    double exp_bias;
    int n_init_samples;
    // number of distinct search locations (or more precisely, number of continuous distinct search spaces,
    // i.e. a single location could have different isolated blocks of stim parameters allowed for search, e.g.
    // search is allowed from 1-10 Hz and from 50-100 Hz for a single stim location. Both ranges are searched
    // separately)
    int n_searches;
    int verbosity;
    CCmpndKern kern;
    CSearchComparison search;

    RC::Ptr<Handler> hndl;
    RC::Ptr<StatusPanel> status_panel;
    RC::RStr buffer;

    // set of stimulation profiles used to indicate unique stim locations
    // ordered by testing priority (unknown number of stim events per experiment)
    RC::Data1D<StimProfile> min_stim_loc_profiles;
    RC::Data1D<StimProfile> max_stim_loc_profiles;
    CPSSpecs cps_specs;
    StimProfile best_stim_profile;
    bool beat_sham;

    // logging
    RC::Data1D<RC::Data1D<StimProfile>> stim_profiles;
    RC::Data1D<double> classif_results;
    vector<double> sham_results;

    // ExpEvent structs effectively only store ISIs
    RC::Data1D<RC::Data1D<ExpEvent>> exp_events;
    // indices into exp_events for latest events added for each stim profile in stim_profiles
    RC::Data1D<size_t> exp_event_idxs;
    RC::Data1D<bool> stim_event_flags;
    RC::Data1D<TaskClassifierSettings> exper_classif_settings;
    // absolute (relative to start of the experiment) times of EEG collection for each event in ms
    RC::Data1D<uint64_t> eeg_times;
    RC::Data1D<uint64_t> stim_times;
    // array of distinct stim profile indices in order of event selection; index zero indicates sham
    RC::Data1D<unsigned int> model_idxs;

    // temp
    uint64_t event_time;
    f64 exp_start;
//    size_t cur_ev;
    bool prev_sham;
    // holds shuffled indices of stim profiles (and sham events) to stimulate with
    RC::Data1D<size_t> search_order;
    // index into search_order to obtain index of stim profile to stimulate with next
    size_t search_order_idx;
    uint64_t next_min_event_time;
    uint64_t classif_id;

    RC::RND rng;

    // debug only
    #ifdef DEBUG_EXPERCPS
    uint64_t clf_start_time;  // time at which classification is requested
    uint64_t clf_handler_time;
    #endif
  };
}

#endif // EXPERCPS_H

