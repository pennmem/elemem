#include "ExperCPS.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  ExperCPS::ExperCPS(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    AddToThread(this);

    // TODO: RDD: use executable-wide seed if available
    seed = 1;
    n_var = 1;
    obsNoise = 0.1;
    exp_bias = 0.25;
    n_init_samples = 5;
    pval_threshold = 0.05;

    verbosity = 1;

    // cnpy::npz_t npz_dict;
    // CKern* k = getSklearnKernel((unsigned int)n_var, npz_dict, kernel, std::string(""), true);
    k = new CMatern32Kern(n_var);
    kern = CCmpndKern(n_var);
    kern.addKern(k);
    whitek = new CWhiteKern(n_var);
    kern.addKern(whitek);
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

    // TODO: RDD: load parameters from config file - ask Ryan/James about how this is handled
    // TODO: RDD: check values with design docs
    n_normalize_events = 2;
    poststim_classif_lockout_ms = 250;  //500;
    // TODO: RDD: replace with CPSSpecs.intertrial_range_ms
    stim_lockout_ms = 250; // 2500;
    normalize_lockout_ms = 300; // 3000; // should be stim_lockout_ms + stim duration, though duration may not be fixed
    // TODO: JPB: move to config file for setting classifier settings during initialization?
    // classification interval duration currently
    classify_ms = 500;  // 1000 // 1350;  // TODO: RDD/JPB: lengthen circular buffer?

    // classifier event counter
    classif_id = 0;
    search_order_idx = 0;
  }

  ExperCPS::~ExperCPS() {
    delete k;
    delete whitek;
    Stop_Handler();
  }

  void ExperCPS::SetStimProfiles_Handler(
        const RC::Data1D<CSStimProfile>& new_stim_loc_profiles) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::SetStimProfiles\n"));
    #endif
    n_searches = new_stim_loc_profiles.size();
    for (size_t i = 0; i < n_searches + 1; i++) { search_order += i; }
    // TODO: RDD: determine adequate number of sham events
    // add extra sham events for stability
    if (n_searches > 5) { search_order += 0; }
    param_bounds.clear();
    // for now give all searches the same kernels and kernel hyperparameters
    vector<CCmpndKern> kernels;
    vector<double> observation_noises;
    vector<double> exploration_biases;
    vector<int> init_samples;
    vector<int> rng_seeds;

    CMatrix bounds(n_var, 2);

    for (int i = 0; i < n_searches; i++) {
      // TODO: LATER RDD: fix this to allow for searching over arbitrary parameters
      //            for now assume that the maximum amplitude is the amplitude in the given profiles
      // TODO: RDD: bounds should be passed separately with min/max configuration; fine for now with just amplitude being tuned
      // amplitude bounds in mA
      bounds.setVal(0.1, 0);
      bounds.setVal(((double)new_stim_loc_profiles[i][0].amplitude)/1000, 1);

      kernels.push_back(kern);
      param_bounds.push_back(bounds);
      observation_noises.push_back(obsNoise * obsNoise);
      exploration_biases.push_back(exp_bias);
      init_samples.push_back(n_init_samples);
      rng_seeds.push_back(seed + n_searches * 100000);
    }
    search = CSearchComparison(n_searches, pval_threshold, kernels, param_bounds, observation_noises,
        exploration_biases, init_samples, rng_seeds, verbosity);
    stim_loc_profiles = new_stim_loc_profiles;
    stim_profiles = RC::Data2D<CSStimProfile>(0, n_searches);
//    add initial stim profiles for all searches here
  }

  void ExperCPS::GetNextEvent(const unsigned int model_idx) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::GetNextEvent\n"));
    #endif
    CMatrix* stim_pars = search.get_next_sample(model_idx);

    // TODO: all: LATER currently assuming that stim parameters not being searched over are set separately in
    //            stim_loc_profiles, should add these to Settings.cpp
    if (stim_loc_profiles[0].size() != 1) {
      Throw_RC_Error("ExperCPS::GetNextEvent: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }
    CSStimChannel stim_chan((stim_loc_profiles[0][0]));

    // set stim parameter values for next search point
    // TODO: RDD: LATER may want to scale down into more desirable interval, e.g., [0, 1]

    // convert to allowable discrete stim settings
    // stim_pars amplitude in mA
    // CSStimChannel.amplitude in uA but allowed stim values in increments of 100 uA
    stim_chan.amplitude = ((uint16_t)(stim_pars->getVal(0)*10 + 0.5)) * 100;

    ExpEvent ev;
    ev.active_ms = stim_chan.duration/1000;

    // Add ISI
    ev.event_ms = ev.active_ms +
      rng.GetRange(cps_specs.intertrial_range_ms[0],
          cps_specs.intertrial_range_ms[1]);

    CSStimProfile stim_profile;
    stim_profile += stim_chan;
    stim_profiles[model_idx] += stim_profile;
    delete stim_pars;
    exp_events += ev;
  }

  void ExperCPS::UpdateSearch(const unsigned int model_idx, const CSStimProfile stim_info, const ExpEvent ev, const double biomarker) {
    if (stim_info.size() != 1) {
      Throw_RC_Error("ExperCPS::UpdateSearch: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::UpdateSearch\n"));
    #endif

    // ExpEvent ev currently unused; API allows for future experiments in which stim duration is tuned
    CMatrix stim_pars(1, 1);
    // map stim parameters for search model
    // TODO: RDD: remove magic numbers for conversion factors, 
    //            easy to forget to change in both UpdateSearch() and GetNextEvent()
    stim_pars(0) = ((double)(stim_info[0].amplitude))/1000;

    CMatrix biomarker_mat(biomarker);
    search.add_sample(model_idx, stim_pars, biomarker_mat);
  }


  void ExperCPS::DoConfigEvent(const CSStimProfile& profile) {
    JSONFile config_event = MakeResp("CONFIG");
    config_event.Set(JSONifyCSStimProfile(profile), "data");
    status_panel->SetEvent("PRESET");
    UpdateSearchPanel(profile);
    hndl->stim_worker.ConfigureStimulation(profile);
  }


  void ExperCPS::UpdateSearchPanel(const CSStimProfile& profile) {
    JSONFile update_event = MakeResp("UPDATE SEARCH");
    update_event.Set(JSONifyCSStimProfile(profile), "data");
    hndl->event_log.Log(update_event.Line());
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


  void ExperCPS::DoStimEvent(const CSStimProfile& profile) {
    JSONFile stim_event = MakeResp("STIM");
    stim_event.Set(JSONifyCSStimProfile(profile).json, "data");
    stim_event.Set(search_order[search_order_idx] - 1, "data", "location_idx");
    hndl->event_log.Log(stim_event.Line());
    status_panel->SetEvent("STIM");
  }


  JSONFile ExperCPS::JSONifyCSStimProfile(const CSStimProfile& profile) {
    JSONFile stim_event;
    stim_event.Set(nlohmann::json::array(), "stim_profile");
    for (int i = 0; i < profile.size(); i++) {
      stim_event.json["stim_profile"].push_back(nlohmann::json({}));
      CSStimChannel chan = profile[i];
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
    JSONFile sham_event = MakeResp("SHAM");
    sham_event.Set(cps_specs.sham_duration_ms, "data", "duration");
    hndl->event_log.Log(sham_event.Line());
    status_panel->SetEvent("SHAM");
  }


  void ExperCPS::Start_Handler() {
    cur_ev = 0;
    event_time = 0;
    next_min_event_time = 0;

    // Total run time (ms), fixes experiment length for CPS.
    experiment_duration = cps_specs.experiment_duration_secs * 1000;

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::Start_Handler\n"));
    #endif

    // Confirm window for run time.
    if (!ConfirmWin(RC::RStr("Total run time will be ") + experiment_duration / (60 * 1000) + " min. "
          + (experiment_duration * 1000) % 60 + " sec.", "Session Duration")) {
      hndl->StopExperiment();
      return;
    }

    exp_start = RC::Time::Get();

    JSONFile startlog = MakeResp("START");
    hndl->event_log.Log(startlog.Line());

    // initial time through searches/stim locations simply go through in order; location 0 comes first
    model_idxs += 0;
    model_idxs += 1;
    GetNextEvent(0);
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::Start_Handler start ProcessClassifierEvent\n"));
    #endif
    TriggerAt(0, ClassificationType::NORMALIZE);
  }


  void ExperCPS::Stop_Handler() {
    if (timer.IsSet()) {
      timer->stop();
    }

    JSONFile stoplog = MakeResp("EXIT");
    hndl->event_log.Log(stoplog.Line());
  }


  // TODO: RDD: are any of these internal stop functions guaranteed to run if other workers fail?
  void ExperCPS::InternalStop() {
    ComputeBestStimProfile();

    // log best stim profile
    
    // TODO RDD/RC: safely make separate data logs in the session directory
    //              e.g. for convenience of analysis

    // JSONFile best_stim_log;
    // best_stim_log.Load(File::FullPath(hndl->session_dir,
    //       "experiment_config.json"));
    JSONFile best_stim_json;
    best_stim_json.Set(JSONifyCSStimProfile(best_stim_profile), "data", "best_stim_profile");
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
      data_log.json["analysis_data"]["exp_events"].push_back(nlohmann::json({}));
      data_log.Set(exp_events[i].active_ms, "analysis_data", "exp_events", i, "active_ms");
      data_log.Set(exp_events[i].event_ms, "analysis_data", "exp_events", i, "event_ms");
    }
    for (int i = 0; i < n_searches; i++) {
      data_log.json["analysis_data"]["stim_profiles"].push_back(nlohmann::json::array());
      for (int j = 0; j < stim_profiles[i].size(); j++) {
        data_log.json["analysis_data"]["stim_profiles"][i].push_back(JSONifyCSStimProfile(stim_profiles[i][j]).json);
      }
    }

    for (int i = 0; i < stim_loc_profiles.size(); i++) {
      data_log.Set(JSONifyCSStimProfile(stim_loc_profiles[i]), "analysis_data", "stim_loc_profiles", i);
    }

    // data_log.Set(exper_classif_settings, "analysis_data", "exper_classif_settings");
    // data_log.Set(abs_event_times, "analysis_data", "abs_event_times");
    // data_log.Save(File::FullPath(hndl->session_dir,
    //       "experiment_data.json"));
    hndl->event_log.Log(data_log.Line());

    Stop_Handler();
    hndl->StopExperiment();
  }


  uint64_t ExperCPS::WaitUntil(uint64_t target_ms) {
    // delays until target_ms milliseconds from the start of the experiment
    // breaks delay every 50 ms to allow for stopping the experiment
    f64 cur_time_sec = RC::Time::Get();
    uint64_t current_time_ms = uint64_t(1000*(cur_time_sec - exp_start)+0.5);

    event_time = target_ms;

    uint64_t delay;
    if (target_ms <= current_time_ms) {
      delay = 0;
    } else {
      delay = target_ms - current_time_ms;
    }

    // delay in 50 ms increments to ensure thread sensitivity to shutdowns
    // TODO: RDD: test delay accuracy - ask Ryan, 
    //            generally use system time to compare at beginning and end of event in question, 
    //            running on separate counter running on processor crystal clock
    //            need to confirm on every machine, task laptop, etc.
    //            James thinks this is overkill for precision of 10-15 ms
    //            https://stackoverflow.com/questions/12937963/get-local-time-in-nanoseconds
    uint64_t min_delay = 50;
    uint64_t delay_next;
    while (delay > 0) {
      delay_next = (delay < min_delay ? delay : min_delay);
      delay -= delay_next;
      QThread::msleep(delay_next);
    }
    // return time after delay (which could differ from target)
    return uint64_t(1000*(RC::Time::Get() - exp_start)+0.5);
  }


  // handles the timing of setting off the next event; handles pre-event general logging
  void ExperCPS::TriggerAt(const uint64_t& next_min_event_time, const ClassificationType& next_classif_state) {
    // run next classification result (which conditionally calls for stimulation events)
    if (next_min_event_time > experiment_duration) {
      // TODO: RDD/RC: preferred method for ending experiments?
      InternalStop();
    }
    abs_EEG_collection_times += WaitUntil(next_min_event_time);
    // TODO: RDD/JPB: use id = 0 for now, how much performance improvement would classifier queueing add?
    // TODO: RDD: remove classify_ms
    hndl->task_classifier_manager->ProcessClassifierEvent(
        next_classif_state, classify_ms, classif_id);
  }


  void ExperCPS::HandleNormalization_Handler(RC::APtr<const EEGPowers>& data, const TaskClassifierSettings& task_classifier_settings) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler\n"));
    #endif

    // TODO: RDD: confirm that last events in all saved event arrays can be disambiguated 
    // (i.e., if any array is longer than another because the experiment stopped in some particular place, 
    // ensure the odd element out can be identified)
    ClassificationType classif_state = task_classifier_settings.cl_type;
    exper_classif_settings += task_classifier_settings;

    ClassificationType next_classif_state;
    if (classif_state == ClassificationType::NORMALIZE) {
      // record times for normalization events separately here
      f64 cur_time_sec = RC::Time::Get();
      uint64_t cur_time_ms = uint64_t(1000*(cur_time_sec - exp_start)+0.5);

      NormalizingPanel();
      next_min_event_time = cur_time_ms + normalize_lockout_ms;
      if (abs_EEG_collection_times.size() < n_normalize_events) {
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler normalization event\n"));
        #endif
        // TODO log event info
        // TODO consider adding some jitter here and to timeouts in general
        next_classif_state = ClassificationType::NORMALIZE;
      }
      else if (abs_EEG_collection_times.size() == n_normalize_events) {
        // TODO log event info
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler sham event\n"));
        #endif
        next_classif_state = ClassificationType::SHAM;
      }
      else {
        Throw_RC_Error("Normalization event requested after n_normalize_events normalization events completed.\n");
      }
      TriggerAt(next_min_event_time, next_classif_state);
    }
  }


  void ExperCPS::ClassifierDecision_Handler(const double& result,
        const TaskClassifierSettings& task_classifier_settings) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler\n"));
    #endif
    // record classifier outcomes
    classif_results += result;
    classif_id++;
  }


  void ExperCPS::StimDecision_Handler(const bool& stim_event, const TaskClassifierSettings& classif_settings, const f64& stim_time_from_start_sec) {
    // TODO: RDD: add docstrings for all functions
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler\n"));
    // #endif

    uint64_t cur_time_ms = uint64_t(1000*(stim_time_from_start_sec - exp_start)+0.5);

    // TODO: RDD: ensure that last events in all saved event arrays can be disambiguated 
    // (i.e., if any array is longer than another because the experiment stopped in some particular place, 
    // ensure the odd element out can be identified)
    // TODO: RDD/JPB: ensure that stim events are set off as close as possible to this callback being called
    //                might be simplest to let this function control the stim?
    //                does CereStim allow us to receive a callback when a stim event ends?
    abs_stim_event_times += cur_time_ms;
    // TODO: RDD: times to record: EEG collection onset/offset for each classification event,
    //                             stim onset/offset for each stim/sham event
    //                             also, return callback return times both for this handler, 
    //                             ClassifierDecision_Handler, and for the StimManager handler

    double result = classif_results[classif_results.size() - 1];
    TaskClassifierSettings task_classifier_settings = exper_classif_settings[exper_classif_settings.size() - 1];
    ClassificationType classif_state = task_classifier_settings.cl_type;

    // pre-stim/post-stim events
    unsigned int model_idx = model_idxs[model_idxs.size() - 1] - 1;
    unsigned int profile_idx = stim_profiles[model_idx].size() - 1;
    CSStimProfile stim_params = stim_profiles[model_idx][profile_idx];
    ExpEvent cur_event = exp_events[cur_ev];

    // TODO define minimum numbers of events for each stim location/sham for experiment

    // store stim event results
    stim_event_flags += stim_event;

    ClassificationType next_classif_state;
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler just before states\n"));
    // #endif
    if (classif_state == ClassificationType::STIM ||
        classif_state == ClassificationType::SHAM) {  // received pre-stim/pre-sham classification event
      if (stim_event) { // stim event occurred or would have occurred if the event were not a sham
        #ifdef DEBUG_EXPERCPS
        if (classif_state == ClassificationType::STIM)
          { RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: stim decision\n")); }
        else { RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: sham decision\n")); }
        #endif
        // TODO handle cases where say stim duration not equal between given and used params
        //      use assert as a placeholder for initial testing
        //      these cases are not expected to occur; this is a precaution
        // assert(stim_params == stim_profiles[model_idx][profile_idx]);
        // if (stim_params != stim_profiles[model_idx][profile_idx]) {
        //   // TODO: RDD: add warning message here
        //   warning
        //   stim_profiles[model_idx][profile_idx] = stim_params;
        // }
        uint64_t stim_offset_ms = abs_stim_event_times[abs_stim_event_times.size() - 1] + stim_params[0].duration / 1000;
        next_min_event_time = stim_offset_ms + poststim_classif_lockout_ms;
        next_classif_state = ClassificationType::NOSTIM;
        if (classif_state == ClassificationType::STIM) {
          prev_sham = false;
          DoStimEvent(stim_params);
        }
        else {
          prev_sham = true;
          DoShamEvent();
        }
      }
      else {  // good memory state detected and stim event would not have occurred
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: good memory state\n"));
        #endif
        // keep classifying until a poor memory state is detected
        next_min_event_time = cur_time_ms;
        next_classif_state = classif_state;
        ClassifyingPanel();
      }
    }
    else if (classif_state == ClassificationType::NOSTIM) {  // received post-stim/post-sham classification event
      uint64_t stim_offset_ms = abs_stim_event_times[abs_stim_event_times.size() - 2] + stim_params[0].duration / 1000;
      next_min_event_time = stim_offset_ms + stim_lockout_ms;
      status_panel->SetEvent("UPDATING");

      double biomarker = result - classif_results[classif_results.size() - 2];
      // TODO add struct for logging everything related to a full pre-post event (i.e., pre-stim classifier event time, stim event time, post-stim classifier time, classifier results, biomarker, stim parameters, ideally Bayesian search hps? definitely should store those separately...)
      //      otherwise could just parse after the fact, this would prevent misparsing...
      // #ifdef DEBUG_EXPERCPS
      // RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: nostim decision\n"));
      // #endif

      if (prev_sham) {
          sham_results.push_back(biomarker);
      }
      else {
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: nostim decision with prev stim\n\n"));
        #endif
        UpdateSearch(model_idx, stim_params, cur_event, biomarker);
        GetNextEvent(model_idx);
        cur_ev++;
      }
      
      // Randomize experiment events. Ensure searches and/or shams are not repeated consecutively.
      search_order_idx++;
      if (search_order_idx == n_searches) {
        size_t prev_search = search_order[n_searches - 1];
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
      if (search_order[search_order_idx]) { next_classif_state = ClassificationType::STIM; }
      else {
        next_classif_state = ClassificationType::SHAM;
        // TODO: RDD: set sham duration/event info? how are sham events controlled in StimulationManager?
      }
      // TODO: RDD: log model_idxs separately in experimental data log; already technically included in stored stim param profiles
      model_idxs += search_order[search_order_idx];

      // run pre-event (stim configuration) as soon as stim parameters are available (after any randomization)
      if (!prev_sham) {
        if (cur_ev < exp_events.size()) {
          unsigned int next_idx = search_order[search_order_idx];
          CSStimProfile next_stim_profile = stim_profiles[next_idx][stim_profiles[next_idx].size() - 1];
          DoConfigEvent(next_stim_profile);
        }
        else {
          Throw_RC_Error("Event requested before being allocated by the search process.");
        }
      }
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

    // TODO
    // QObject::killTimer: Timers cannot be stopped from another thread
    // QObject::~QObject: Timers cannot be stopped from another thread

    // TODO Error: stimulation requested before parameters set

  }


  void ExperCPS::ComputeBestStimProfile() {
    // TODO: RDD: extend to tuning with arbitrary stim parameters
    // TODO: RDD: save/log full ComparisonStruct results in addition to beat_sham
    ComparisonStruct sol = search.get_best_solution();
    CMatrix best_sol_mat = *(sol.xs[sol.idx_best]);
    // assume single-site stim for now (last index indicates one stim site out of many active in a profile)
    CSStimChannel chan = stim_profiles[sol.idx_best][0][0];
    chan.amplitude = best_sol_mat.getVal(0);
    best_stim_profile += chan;

    // compare best stim paramet set with sham events
    TestStruct sham_test = search.compare_GP_to_sample(sol, sham_results);
    beat_sham = sham_test.pval > pval_threshold;
  }
}

