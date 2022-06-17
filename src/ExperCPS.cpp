#include "ExperCPS.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"
#include "BayesGPc/CMatrix.h"

using namespace RC;

namespace CML {
  ExperCPS::ExperCPS(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    AddToThread(this);

    seed = 1;
    n_var = 1;
    obsNoise = 0.1;
    exp_bias = 0.25;
    n_init_samples = 5;
    pval_threshold = 0.05;

    #ifdef DEBUG_EXPERCPS
    verbosity = 1;
    #else
    verbosity = 0;
    #endif

    CMatern32Kern k(n_var);
    kern = CCmpndKern(n_var);
    kern.addKern(&k);
    CWhiteKern whitek(n_var);
    kern.addKern(&whitek);
    CMatrix b(1, 2);
    b(0, 0) = 0.1;
    b(0, 1) = 2.0;
    kern.setBoundsByName("matern32_0:lengthScale", b);
    b(0, 0) = 0.25;
    b(0, 1) = 4.0;
    kern.setBoundsByName("matern32_0:variance", b);
    b(0, 0) = 0.01;
    b(0, 1) = 4.0;
    kern.setBoundsByName("white_1:variance", b);

    // TODO: RDD: load parameters from config file
    // TODO: RDD: check values with design docs
    n_normalize_events = 2;  // 25
    // TODO: RDD: ask James about current artifact rejection procedure
    poststim_classif_lockout_ms = 50;
    normalize_lockout_ms = 300; // 3000; // should be cps_specs[0] (min ISI) + stim duration, though duration may not be fixed
    // TODO: JPB: move to config file for setting classifier settings during initialization?
    // for now: classification interval duration, may be deprecated
    classify_ms = 500;  // 1000 // 1350;  // TODO: RDD/JPB: lengthen circular buffer?

    // classifier event counter
    classif_id = 0;
    search_order_idx = 0;
    _init();
  }

  void ExperCPS::_init() {
    // TODO: RDD: check this over, make sure everything necessary it being reset for multiple experiments in same session
    // clear all variables (useful for resetting for multiple experiments in same session)
    abs_EEG_collection_times.Clear();
    classif_results.Clear();
    exper_classif_settings.Clear();
    min_stim_loc_profiles.Clear();
    max_stim_loc_profiles.Clear();
    stim_event_flags.Clear();
    sham_results.clear();
    search_order.Clear();
    model_idxs.Clear();
    best_stim_profile.Clear();
    stim_profiles.Clear();
    exp_events.Clear();
    abs_stim_event_times.Clear();
    // TODO: RDD: clear out normalization events? does the classifier automatically clear out norm events?
    beat_sham = false;
  }

  // TODO: RDD: make into a generic Configure() function
  void ExperCPS::SetStimProfiles_Handler(
        const RC::Data1D<StimProfile>& new_min_stim_loc_profiles,
        const RC::Data1D<StimProfile>& new_max_stim_loc_profiles) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::SetStimProfiles\n"));
    #endif
    if (new_min_stim_loc_profiles.size() != new_max_stim_loc_profiles.size()) {
      Throw_RC_Error("ExperCPS::SetStimProfiles: Min and max stimulation profiles must have the same length.");
    }
    if (new_max_stim_loc_profiles.size() < 1) {
      Throw_RC_Error("ExperCPS::SetStimProfiles: Must provide at least one stimulation profile.");
    }
    _init();
    n_searches = new_max_stim_loc_profiles.size();
    search_order += 0;
    // validation
    // TODO: RDD: generalize for searching over different stim parameters
    // TODO: RDD: add parameters to be tuned to config file? would be most explicit, safest, then test max>min in case some parameter ranges are reduced to a single value during clinical safety testing
    //       else just test whether max_param > min_param, make list of tuned parameters, log
    // TODO: RDD: extend grid search in BayesGPc to allow for tuning over non-contiguous parameter search spaces
    for (size_t i = 0; i < n_searches; i++) {
      if (new_min_stim_loc_profiles[i].size() != 1 ||
          new_max_stim_loc_profiles[i].size() != 1) {
        Throw_RC_Error("ExperCPS::SetStimProfiles: Only single-channel stimulation profiles supported.");
      }
      if (new_min_stim_loc_profiles[i][0].electrode_pos != new_max_stim_loc_profiles[i][0].electrode_pos ||
          new_min_stim_loc_profiles[i][0].electrode_neg != new_max_stim_loc_profiles[i][0].electrode_neg ||
          new_min_stim_loc_profiles[i][0].frequency != new_max_stim_loc_profiles[i][0].frequency ||
          new_min_stim_loc_profiles[i][0].duration != new_max_stim_loc_profiles[i][0].duration ||
          new_min_stim_loc_profiles[i][0].area != new_max_stim_loc_profiles[i][0].area ||
          new_min_stim_loc_profiles[i][0].burst_frac != new_max_stim_loc_profiles[i][0].burst_frac ||
          new_min_stim_loc_profiles[i][0].burst_slow_freq != new_max_stim_loc_profiles[i][0].burst_slow_freq) {
        Throw_RC_Error("ExperCPS::SetStimProfiles: Min and max stimulation profiles do not match for fixed parameters.");
      }
      search_order += i + 1;
    }
    // TODO: RDD: determine adequate number of sham events
    // add extra sham events for stability
    // if (n_searches > 5) { search_order += 0; }

    param_bounds.clear();
    // for now give all searches the same kernels and kernel hyperparameters
    vector<CCmpndKern> kernels;
    vector<double> observation_noises;
    vector<double> exploration_biases;
    vector<int> init_samples;
    vector<int> rng_seeds;

    CMatrix bounds(n_var, 2);

    // search grids
    vector<vector<CMatrix>> all_grid_vals;

    // set search bounds
    for (int i = 0; i < n_searches; i++) {
      // TODO: LATER RDD: fix this to allow for searching over arbitrary parameters

      // amplitude bounds in mA
      bounds.setVal(((double)new_min_stim_loc_profiles[i][0].amplitude)/1000, 0, 0);
      bounds.setVal(((double)new_max_stim_loc_profiles[i][0].amplitude)/1000, 0, 1);

      kernels.push_back(kern);
      param_bounds.push_back(bounds);
      observation_noises.push_back(obsNoise * obsNoise);
      exploration_biases.push_back(exp_bias);
      init_samples.push_back(n_init_samples);
      rng_seeds.push_back(seed + n_searches * 100000);

      double amplitude_resolution = 0.1;  // mA
      int n_grid = (bounds.getVal(1) - bounds.getVal(0))/amplitude_resolution + 1;
      // grid represented as vector of CMatrices over grid values for each dimension
      vector<CMatrix> grid_vals;
      for (int i = 0; i < n_var; i++) {
          CMatrix grid1D = linspace(bounds.getVal(0),
                                    bounds.getVal(1),
                                    n_grid);
          grid_vals.push_back(grid1D);
      }
      all_grid_vals.push_back(grid_vals);

      uint64_t duration = new_max_stim_loc_profiles[i][0].duration / 1000;
      // TODO: RDD: consider this scheme of conservatively setting the sham duration vs. just setting it in config as done now
      // max_stim_duration_ms = duration > max_stim_duration_ms ? duration : max_stim_duration_ms;
    }

    search = CSearchComparison(n_searches, pval_threshold, kernels, param_bounds, observation_noises,
        exploration_biases, init_samples, rng_seeds, verbosity, all_grid_vals);
    min_stim_loc_profiles = new_min_stim_loc_profiles;
    max_stim_loc_profiles = new_max_stim_loc_profiles;

    // add initial stim profiles
    stim_profiles = RC::Data1D<RC::Data1D<StimProfile>>(n_searches);
    exp_events = RC::Data1D<RC::Data1D<ExpEvent>>(n_searches + 1);  // index zero corresponds to sham events
    for (int i = 0; i < n_searches; i++) { GetNextEvent(i); }

    // initial event times
    // initial time through searches/stim locations simply go through in order; location 0 comes first
    model_idxs += 0;
    // TODO: RDD: after refactoring shuffle, shuffle here at start
    ExpEvent ev;
    ev.active_ms = cps_specs.sham_duration_ms;
    ev.event_ms = ev.active_ms;
    exp_events[0] += ev;
  }

  void ExperCPS::GetNextEvent(const unsigned int model_idx) {
    RC_DEBOUT(RC::RStr("ExperCPS::GetNextEvent\n"));
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::GetNextEvent\n"));
    #endif
    // TODO: RDD: refactor so sham events are stored alongside all stim events (i.e. in same variables)?
    RC::APtr<CMatrix> stim_pars = search.get_next_sample(model_idx);

    // TODO: all: LATER currently assuming that stim parameters not being searched over are set separately in
    //            stim_loc_profiles, should add these to Settings.cpp
    if (max_stim_loc_profiles[model_idx].size() != 1) {
      Throw_RC_Error("ExperCPS::GetNextEvent: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }
    StimChannel stim_chan((max_stim_loc_profiles[model_idx][0]));

    // set stim parameter values for next search point
    // TODO: RDD: LATER may want to scale down into more desirable interval, e.g., [0, 1]

    // convert to allowable distinct stim settings
    // stim_pars amplitude in mA
    // StimChannel.amplitude in uA but allowed stim values in increments of 100 uA
    stim_chan.amplitude = ((uint16_t)(stim_pars->getVal(0)*10 + 0.5)) * 100;

    ExpEvent ev;
    ev.active_ms = stim_chan.duration/1000;

    // Add ISI
    // TODO: RDD: assert that CPSSpecs.intertrial_range_ms lower bound is at least stim duration + poststim_classif_lockout
    ev.event_ms = ev.active_ms +
        rng.GetRange(cps_specs.intertrial_range_ms[0],
          cps_specs.intertrial_range_ms[1]);

    StimProfile stim_profile;
    stim_profile += stim_chan;
    stim_profiles[model_idx] += stim_profile;
    // index zero for sham
    exp_events[model_idx + 1] += ev;
  }

  void ExperCPS::UpdateSearch(const unsigned int model_idx, const StimProfile stim_info, const ExpEvent ev, const double biomarker) {
    // TODO: RDD: ev parameter not used in UpdateSearch, probably would never be used, unless it were logged in UpdateSearch()
    UpdateSearchPanel(stim_info);
    if (stim_info.size() != 1) {
      Throw_RC_Error("ExperCPS::UpdateSearch: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::UpdateSearch\n"));
    #endif

    CMatrix stim_pars(1, 1);
    // map stim parameters for search model
    // TODO: RDD: remove magic numbers for conversion factors, 
    //            easy to forget to change in both UpdateSearch() and GetNextEvent()
    stim_pars(0) = ((double)(stim_info[0].amplitude))/1000;

    CMatrix biomarker_mat(biomarker);
    search.add_sample(model_idx, stim_pars, biomarker_mat);

    // log
    JSONFile update_event = MakeResp("UPDATE SEARCH");
    update_event.Set(JSONifyStimProfile(stim_info), "data", "stim_profile");
    vector<vector<double>> x_vec = to_vector(stim_pars);
    update_event.Set(model_idx, "data", "profile_index");
    update_event.Set(x_vec, "data", "x");
    update_event.Set(biomarker, "data", "y");
    hndl->event_log.Log(update_event.Line());
  }


  void ExperCPS::DoConfigEvent(const StimProfile& profile) {
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS: enter DoConfigEvent \n\n"));
    // #endif
    JSONFile config_event = MakeResp("CONFIG");
    config_event.Set(JSONifyStimProfile(profile), "data");
    status_panel->SetEvent("PRESET");
    hndl->stim_worker.ConfigureStimulation(profile);
  }


  void ExperCPS::UpdateSearchPanel(const StimProfile& profile) {
    status_panel->SetEvent("UPDATE SEARCH");
  }


  void ExperCPS::NormalizingPanel() {
    JSONFile classif_event = MakeResp("NORMALIZING");
    hndl->event_log.Log(classif_event.Line());
    status_panel->SetEvent("NORMALIZING");
  }


  void ExperCPS::ClassifyingPanel() {
    JSONFile classif_event = MakeResp("CLASSIFYING");
    hndl->event_log.Log(classif_event.Line());
    status_panel->SetEvent("CLASSIFYING");
  }


  void ExperCPS::DoStimEvent(const StimProfile& profile) {
    JSONFile stim_event = MakeResp("STIM");
    stim_event.Set(JSONifyStimProfile(profile).json, "data");
    stim_event.Set(search_order[search_order_idx] - 1, "data", "location_idx");
    hndl->event_log.Log(stim_event.Line());
    status_panel->SetEvent("STIM");
  }


  JSONFile ExperCPS::JSONifyStimProfile(const StimProfile& profile) {
    JSONFile stim_event;
    stim_event.Set(nlohmann::json::array(), "stim_profile");
    for (int i = 0; i < profile.size(); i++) {
      stim_event.json["stim_profile"].push_back(nlohmann::json({}));
      StimChannel chan = profile[i];
      stim_event.Set(chan.electrode_pos, "stim_profile", i, "electrode_pos");
      stim_event.Set(chan.electrode_neg, "stim_profile", i, "electrode_neg");
      stim_event.Set(chan.amplitude, "stim_profile", i, "amplitude");
      stim_event.Set(chan.frequency, "stim_profile", i, "frequency");
      stim_event.Set(chan.duration, "stim_profile", i, "duration");
      stim_event.Set(chan.area, "stim_profile", i, "area");
      stim_event.Set(chan.burst_frac, "stim_profile", i, "burst_frac");
      stim_event.Set(chan.burst_slow_freq, "stim_profile", i, "burst_slow_freq");
    }
    return stim_event;
  }


  void ExperCPS::DoShamEvent() {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::DoShamEvent\n"));
    #endif
    JSONFile sham_event = MakeResp("SHAM");
    // TODO: RDD: should sham_duration_ms not always be the same as stim duration?
    sham_event.Set(cps_specs.sham_duration_ms, "data", "duration");
    hndl->event_log.Log(sham_event.Line());
    status_panel->SetEvent("SHAM");
  }


  void ExperCPS::Start_Handler() {
    RC_DEBOUT(RC::RStr("ExperCPS::Start_Handler\n"));
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::Start_Handler\n"));
    #endif

//    cur_ev = 0;
    event_time = 0;
    next_min_event_time = 0;

    // Total run time (ms), fixes experiment length for CPS.
    experiment_duration = cps_specs.experiment_duration_secs * 1000;

    // Confirm window for run time.
    if (!ConfirmWin(RC::RStr("Total run time will be ") + experiment_duration / (60 * 1000) + " min. "
          + (experiment_duration * 1000) % 60 + " sec.", "Session Duration")) {
      hndl->StopExperiment();
      return;
    }

    exp_start = RC::Time::Get();

    JSONFile startlog = MakeResp("START");
    hndl->event_log.Log(startlog.Line());

    TriggerAt(0, ClassificationType::NORMALIZE);
  }


  // TODO: RDD/JPB: would starting and restarting an experiment mean rewatching part of the video?
  void ExperCPS::Stop_Handler() {}


  // TODO: RDD: are any of these internal stop functions guaranteed to run if other workers fail?
  void ExperCPS::InternalStop() {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::InternalStop\n"));
    #endif
    ComputeBestStimProfile();

    // log best stim profile

    // TODO: RDD: logging needs to occur throughout the experiment, not just at the ends
    
    // TODO RDD/RC: safely make separate data logs in the session directory
    //              e.g. for convenience of analysis

    // JSONFile best_stim_log;
    // best_stim_log.Load(File::FullPath(hndl->session_dir,
    //       "experiment_config.json"));
    JSONFile best_stim_json;
    best_stim_json.Set(JSONifyStimProfile(best_stim_profile), "data", "best_stim_profile");
    // best_stim_log.Set("", "experiment", "stim_channels");
    // best_stim_log.Set(best_stim_json, "experiment", "stim_channels", 0);
    // best_stim_log.Save(File::FullPath(hndl->session_dir,
    //       "best_stim_parameters.json"));
    if (beat_sham) {
      best_stim_json.Set("true", "data", "beat sham");
    }
    else {
      best_stim_json.Set("false", "data", "beat sham");
    }
    hndl->event_log.Log(best_stim_json.Line());

    JSONFile data_log;
    // save all other data for analysis
    data_log.Set(nlohmann::json::array(), "analysis_data", "classif_results");
    data_log.Set(nlohmann::json::array(), "analysis_data", "stim_profiles");
    data_log.Set(nlohmann::json::array(), "analysis_data", "exp_events");
    for (int i = 0; i < classif_results.size(); i++) {
      data_log.json["analysis_data"]["classif_results"].push_back(classif_results[i]);
    }
//    for (int i = 0; i < exp_events.size(); i++) {
//      data_log.json["analysis_data"]["exp_events"].push_back(nlohmann::json({}));
//      data_log.Set(exp_events[i].active_ms, "analysis_data", "exp_events", i, "active_ms");
//      data_log.Set(exp_events[i].event_ms, "analysis_data", "exp_events", i, "event_ms");
//    }
    // TODO: RDD: need to log stim profile indices
    // TODO: RDD: check whether I'm redundantly logging stim profiles here as well as during the experiment itself
    // TODO: RDD: remove analysis_data logging; just want to log data as it comes in, not at the end
    for (int i = 0; i < n_searches; i++) {
      data_log.json["analysis_data"]["stim_profiles"].push_back(nlohmann::json::array());
      for (int j = 0; j < stim_profiles[i].size(); j++) {
        data_log.json["analysis_data"]["stim_profiles"][i].push_back(JSONifyStimProfile(stim_profiles[i][j]).json);
      }
    }

    // TODO: RDD: move these log statements and all others to be as early as possible? then can't have a single JSON line
    data_log.Set(nlohmann::json::array(), "analysis_data", "min_stim_loc_profiles");
    data_log.Set(nlohmann::json::array(), "analysis_data", "max_stim_loc_profiles");
    for (int i = 0; i < min_stim_loc_profiles.size(); i++) {
      data_log.json["analysis_data"]["min_stim_loc_profiles"].push_back(JSONifyStimProfile(min_stim_loc_profiles[i]).json);
      data_log.json["analysis_data"]["max_stim_loc_profiles"].push_back(JSONifyStimProfile(max_stim_loc_profiles[i]).json);
    }

    // data_log.Set(exper_classif_settings, "analysis_data", "exper_classif_settings");
    // data_log.Set(abs_event_times, "analysis_data", "abs_event_times");
    // data_log.Save(File::FullPath(hndl->session_dir,
    //       "experiment_data.json"));
    hndl->event_log.Log(data_log.Line());

    Stop_Handler();
    JSONFile stoplog = MakeResp("EXIT");
    hndl->event_log.Log(stoplog.Line());
    hndl->StopExperiment();

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::InternalStop exit\n"));
    #endif
  }


  uint64_t ExperCPS::WaitUntil(uint64_t target_ms) {
    // delays until target_ms milliseconds from the start of the experiment
    // breaks delay every 50 ms to allow for stopping the experiment
    f64 cur_time_sec = RC::Time::Get();
    uint64_t current_time_ms = uint64_t(1000*(cur_time_sec - exp_start)+0.5);

    event_time = target_ms;
    uint64_t delay = (target_ms <= current_time_ms ? 0 : target_ms - current_time_ms);  // ms

    // delay in max_delay ms increments to ensure thread sensitivity to shutdowns
    // TODO: RDD: test delay accuracy - ask Ryan, 
    //            generally use system time to compare at beginning and end of event in question, 
    //            running on separate counter running on processor crystal clock
    //            need to confirm on every machine, task laptop, etc.
    //            James thinks this is overkill for precision of 10-15 ms
    //            https://stackoverflow.com/questions/12937963/get-local-time-in-nanoseconds
    uint64_t max_delay = 50;  // ms
    // amount to delay next to ensure delay amount is at most max_delay ms
    uint64_t delay_next;  // ms
    // total time delayed so far in ms
    uint64_t delay_total = uint64_t(1000*(RC::Time::Get() - cur_time_sec)+0.5);
    while (delay_total < delay) {
      // TODO: RDD: some delays are 1 ms off, e.g. last for 999 ms instead of 1000 ms
      //            likely due to round-up of cur_time_sec
      // delay either max_delay or the current amount of delay remaining if less than max_delay
      delay_next = (delay - delay_total < max_delay ? delay - delay_total : max_delay);

      if (ShouldAbort()) { return uint64_t(1000*(RC::Time::Get() - exp_start)+0.5); }
      QThread::msleep(delay_next);
      delay_total = uint64_t(1000*(RC::Time::Get() - cur_time_sec)+0.5);
    }

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::WaitUntil() Intended delay: ") + to_string(delay)
              + RC::RStr("\nActual delay: ")
              + to_string(uint64_t(1000*(RC::Time::Get() - cur_time_sec)+0.5))
              + RC::RStr("\n"));
    #endif

    // return time after delay (which could differ from target due to e.g. operating system delays)
    return uint64_t(1000*(RC::Time::Get() - exp_start)+0.5);
  }


  // handles the timing of setting off the next event; handles pre-event general logging
  void ExperCPS::TriggerAt(const uint64_t& next_min_event_time, const ClassificationType& next_classif_state) {
    // run next classification result (which conditionally calls for stimulation events)
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS::TriggerAt\n"));
    // #endif
    if (next_min_event_time > experiment_duration) { InternalStop(); }
    if (ShouldAbort()) { return; }

    abs_EEG_collection_times += WaitUntil(next_min_event_time);
    hndl->task_classifier_manager->ProcessClassifierEvent(
        next_classif_state, classify_ms, classif_id);
  }


  void ExperCPS::HandleNormalization_Handler(
      RC::APtr<const EEGPowers>& data,
      const TaskClassifierSettings& task_classifier_settings) {
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler\n"));
    // #endif

    // TODO: RDD: confirm that last events in all saved event arrays can be disambiguated 
    // (i.e., if any array is longer than another because the experiment stopped in some particular place, 
    // ensure the odd element out can be identified)
    // experiment could abort at any point
    ClassificationType classif_state = task_classifier_settings.cl_type;
    exper_classif_settings += task_classifier_settings;

    ClassificationType next_classif_state;

    if (classif_state != ClassificationType::NORMALIZE) { return; }

    // record times for normalization events separately here
    f64 cur_time_sec = RC::Time::Get();
    uint64_t cur_time_ms = uint64_t(1000*(cur_time_sec - exp_start)+0.5);

    NormalizingPanel();
    next_min_event_time = cur_time_ms + normalize_lockout_ms;
    if (abs_EEG_collection_times.size() < n_normalize_events) {
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler request normalization event\n"));
      #endif
      // TODO log event info
      // TODO consider adding some jitter here and to timeouts in general
      next_classif_state = ClassificationType::NORMALIZE;
    }
    else if (abs_EEG_collection_times.size() == n_normalize_events) {
      // TODO log event info
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler request sham event\n"));
      #endif
      next_classif_state = ClassificationType::SHAM;
    }
    else {
      Throw_RC_Error("Normalization event requested after n_normalize_events normalization events completed.\n");
    }
    TriggerAt(next_min_event_time, next_classif_state);
  }


  void ExperCPS::ClassifierDecision_Handler(const double& result,
        const TaskClassifierSettings& task_classifier_settings) {
    //#ifdef DEBUG_EXPERCPS
    //RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler\n"));
    //#endif
    classif_decision_arrived = true;
    // record classifier outcomes
    classif_results += result;
    classif_id++;
    if (stim_decision_arrived) { ProcessEvent(); }
  }

  void ExperCPS::StimDecision_Handler(const bool& stim_event, const TaskClassifierSettings& classif_settings, const f64& stim_time_from_start_sec) {
//    #ifdef DEBUG_EXPERCPS
//    RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler\n"));
//    #endif

    stim_decision_arrived = true;
    uint64_t cur_time_ms = uint64_t(1000*(stim_time_from_start_sec - exp_start)+0.5);

    // TODO: RDD: ensure that last events in all saved event arrays can be disambiguated 
    // (i.e., if any array is longer than another because the experiment stopped in some particular place, 
    // ensure the odd element out can be identified)
    abs_stim_event_times += cur_time_ms;

    // store stim event results
    stim_event_flags += stim_event;

    if (classif_decision_arrived) { ProcessEvent(); }
  }

  void ExperCPS::ProcessEvent() {
    // reset state variables
    classif_decision_arrived = false;
    stim_decision_arrived = false;

    const TaskClassifierSettings classif_settings = exper_classif_settings[exper_classif_settings.size() - 1];
    uint64_t cur_time_ms = abs_stim_event_times[abs_stim_event_times.size() - 1];
    const bool stim_event = stim_event_flags[stim_event_flags.size() - 1];

    // TODO: RDD: times to record: EEG collection onset/offset for each classification event,
    //                             stim onset/offset for each stim/sham event
    //                             also, return callback return times both for this handler,
    //                             ClassifierDecision_Handler, and for the StimManager handler
    double result = classif_results[classif_results.size() - 1];
    TaskClassifierSettings task_classifier_settings = exper_classif_settings[exper_classif_settings.size() - 1];
    ClassificationType classif_state = task_classifier_settings.cl_type;

    unsigned int model_idx = model_idxs[model_idxs.size() - 1];
    ExpEvent cur_event = exp_events[model_idx][exp_events[model_idx].size() - 1];
    // duration of either next stim event (for pre-event) or previous stim event (for post-event)
    uint64_t event_duration_ms = cur_event.active_ms;

    StimProfile stim_params;

    // TODO: RDD: define minimum numbers of events for each stim location/sham for experiment

    ClassificationType next_classif_state;
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler just before states\n"));
    // #endif
    if (classif_state == ClassificationType::STIM ||
        classif_state == ClassificationType::SHAM) {  // received pre-stim/pre-sham classification event
      if (stim_event) { // stim event occurred or would have occurred if the event were not a sham
        #ifdef DEBUG_EXPERCPS
        if (classif_state == ClassificationType::STIM)
          { RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: stim event received\n")); }
        else { RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: sham event received\n")); }
        #endif

        uint64_t stim_offset_ms = abs_stim_event_times[abs_stim_event_times.size() - 1] + event_duration_ms;
        //#ifdef DEBUG_EXPERCPS
        //RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: just after stim_offset_ms assignment\n"));
        //#endif
        next_min_event_time = stim_offset_ms + poststim_classif_lockout_ms;
        next_classif_state = ClassificationType::NOSTIM;
        if (classif_state == ClassificationType::STIM) {
          prev_sham = false;
          DoStimEvent(stim_params);
        }
        else {  // SHAM event
          prev_sham = true;
          DoShamEvent();
        }
      }
      else {  // stim event did not occur (good memory state detected)
        // #ifdef DEBUG_EXPERCPS
        // RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: good memory state\n"));
        // #endif
        // keep classifying until bad memory state detected
        next_min_event_time = cur_time_ms;
        next_classif_state = classif_state;
        ClassifyingPanel();
      }
    }
    else if (classif_state == ClassificationType::NOSTIM) {  // received post-stim/post-sham classification event
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: nostim event enter\n"));
      #endif

      // get next event
      uint64_t ISI = cur_event.event_ms - cur_event.active_ms;
      // TODO: RDD: ensure event logging and task is robust to abort signals for each event type
      // TODO: RDD: consider all event definitions for logging; need to log events as they occur, not just at the end
      // TODO: RDD: ensure absolute event times are recorded in logging
      // TODO: RDD: confirm that events are separated by event duration + ISI
      next_min_event_time = abs_stim_event_times[abs_stim_event_times.size() - 2] + event_duration_ms + ISI;
      status_panel->SetEvent("UPDATING");

      double biomarker = result - classif_results[classif_results.size() - 2];
      // TODO add struct for logging everything related to a full pre-post event (i.e., pre-stim classifier event time, stim event time, post-stim classifier time, classifier results, biomarker, stim parameters, ideally Bayesian search hps? definitely should store those separately...)
      //      otherwise could just parse after the fact, this would prevent misparsing...
      // #ifdef DEBUG_EXPERCPS
      // RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: nostim decision\n"));
      // #endif

      if (prev_sham) {
          sham_results.push_back(biomarker);
          ExpEvent ev;
          ev.active_ms = cps_specs.sham_duration_ms;
          // Add ISI
          ev.event_ms = ev.active_ms +
              rng.GetRange(cps_specs.intertrial_range_ms[0],
                cps_specs.intertrial_range_ms[1]);
          exp_events[0] += ev;
      }
      else {
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: nostim decision with prev stim\n\n"));
        #endif
        // get previous stim event
        unsigned int profile_idx = stim_profiles[model_idx - 1].size() - 1;
        StimProfile stim_params = stim_profiles[model_idx - 1][profile_idx];

        // TODO: RDD: would it make sense to compare sham events to acccount for autocorrelation in performance? GP estimates will be biased toward later samples which will cluster around final estimate whereas shams will occur throughout experiment...
        UpdateSearch(model_idx - 1, stim_params, cur_event, biomarker);
        // TODO: RDD: add logging here once pre-post event has been added to search
        GetNextEvent(model_idx - 1);
      }
      
      // Randomize experiment events. Ensure searches and/or shams are not repeated consecutively.
      // TODO: RDD: refactor into separate function
      // TODO: RDD: confirm shuffling constraints (stochasticity, non-consecutiveness) in automated tests
      search_order_idx++;
      if (search_order_idx == search_order.size()) {
        // shuffle order of selecting distinct stim profiles for next events, reset search_order_idx
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: shuffling stim locations\n\n"));
        #endif
        size_t prev_search = search_order[search_order.size() - 1];
        bool two_in_row = true;
        while (search_order[0] == prev_search || two_in_row) {
          search_order.Shuffle();
          two_in_row = false;
          for (int p = 0; p < search_order.size() - 1; p++) {
            if (search_order[p] == search_order[p + 1]) { two_in_row = true; }
          }
        }
        search_order_idx = 0;
      }
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: search_order[search_order_idx] ") + to_string(search_order[search_order_idx]) + RC::RStr("\n"));
      #endif

      if (search_order[search_order_idx]) {
        next_classif_state = ClassificationType::STIM;
        // run pre-event (stim configuration) once stim parameters are available (after any randomization)
        // if needed, could reduce latency after sham events by configuring next stim set earlier 
        // immediately after sham events
        unsigned int next_idx = search_order[search_order_idx] - 1;
        StimProfile next_stim_profile = stim_profiles[next_idx][stim_profiles[next_idx].size() - 1];
        DoConfigEvent(next_stim_profile);
      }
      else { next_classif_state = ClassificationType::SHAM; }
      // log which stim location (or whether sham) was last requested
      // TODO: RDD: log model_idxs separately in experimental data log; already technically included in stored stim param profiles
      model_idxs += search_order[search_order_idx];

      status_panel->SetEvent("CLASSIFYING");
    }
    else if (classif_state == ClassificationType::NORMALIZE) {
      Throw_RC_Error("Normalization event requested after n_normalize_events normalization events completed.\n");
    }
    else {
      Throw_RC_Error("Invalid classification type received.\n");
    }

    TriggerAt(next_min_event_time, next_classif_state);
    // TODO: RDD: add logging after each event
    //  JSONFile data;
    // data.Set(result, "result");
    // data.Set(stim, "decision");

    // const RC::RStr type = [&] {
    //     switch (task_classifier_settings.cl_type) {
    //       case ClassificationType::STIM: return "STIM_DECISON";
    //       case ClassificationType::SHAM: return "SHAM_DECISON";
    //       case ClassificationType::NOSTIM: return "NOSTIM_DECISION";
    //       case ClassificationType::NORMALIZE: return "NORMALIZE_NOSTIM_DECISION";
    //       default: Throw_RC_Error("Invalid classification type received.");
    //     }
    // }();

    // auto resp = MakeResp(type, task_classifier_settings.classif_id, data);
    // hndl->event_log.Log(resp.Line());
    // RC_DEBOUT(resp);
  }


  void ExperCPS::ComputeBestStimProfile() {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::ComputeBestStimProfile\n"));
    #endif
    // TODO LATER: RDD: extend to tuning with arbitrary stim parameters
    // TODO: RDD: save/log full ComparisonStruct results in addition to beat_sham
    ComparisonStruct sol = search.get_best_solution();

    if (sol.xs[sol.idx_best] == nullptr) { Throw_RC_Error("Null pointer in ExperCPS::ComputeBestStimProfile() sol.\n"); }
    CMatrix best_sol_mat = *(sol.xs[sol.idx_best]);
    // assume single-site stim for now (last index indicates one stim site out of many active in a profile)
    StimChannel chan = stim_profiles[sol.idx_best][0][0];
    // convert from mA to uA
    chan.amplitude = (uint16_t)(best_sol_mat.getVal(0) * 1000);
    best_stim_profile += chan;

    // compare best stim parameter set with sham events
    TestStruct sham_test = search.compare_GP_to_sample(sol, sham_results);
    beat_sham = sham_test.pval < pval_threshold;
  }
}

