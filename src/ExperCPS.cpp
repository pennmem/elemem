#include "ExperCPS.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"

using namespace RC;

#define DEBUG_EXPERCPS

// TODO change name from CPS to something less easily mistaken for OPS

namespace CML {
  ExperCPS::ExperCPS(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    AddToThread(this);

    seed = 1;
    n_var = 1;
    obsNoise = 0.1;
    exp_bias = 0.25;
    n_init_samples = 100;
    CMatrix bounds(n_var, 2);
    // amplitude bounds in mA
    bounds(0, 0) = 0.0;
    bounds(0, 1) = 0.0;

    int verbosity = 0;

    // cnpy::npz_t npz_dict;
    // CKern* k = getSklearnKernel((unsigned int)n_var, npz_dict, kernel, std::string(""), true);
    k = new CMatern32Kern(n_var);
    kern = CCmpndKern(n_var);
    kern.addKern(k);
    whitek = new CWhiteKern(n_var);
    kern.addKern(whitek);

    // TODO: RDD: load parameters from config file - ask Ryan/James about how this is handled
    // TODO: RDD: check values with design docs
    n_normalize_events = 25;
    poststim_classif_lockout_ms = 500;
    // TODO: RDD: replace with CPSSpecs.intertrial_range_ms
    stim_lockout_ms = 2500;
    normalize_lockout_ms = 3000; // should be stim_lockout_ms + stim duration, though duration may not be fixed
    // TODO: JPB: move to config file for setting classifier settings during initialization?
    // classification interval duration currently
    classify_ms = 1350;

    // classifier event counter
    classif_id = 0;

    BayesianSearchModel search(kern, &bounds, obsNoise * obsNoise, exp_bias, n_init_samples, seed, verbosity);
  }

  ExperCPS::~ExperCPS() {
    delete k;
    delete whitek;
    Stop_Handler();
  }

  void ExperCPS::SetStimProfiles_Handler(
        const RC::Data1D<CSStimProfile>& new_stim_loc_profiles) {
    if (new_stim_loc_profiles.size() != 1) {
      Throw_RC_Error("ExperCPS::SetStimProfiles: Need to update for multiple stim sites...");
    }
    for (int i = 0; i < new_stim_loc_profiles.size(); i++) {
      if (new_stim_loc_profiles[i].size() != 1) {
        Throw_RC_Error("ExperCPS::SetStimProfiles: Only stimulation events with stimulation profiles of length 1 are supported currently.");
      }
      // TODO: LATER RDD: fix this to allow for searching over arbitrary parameters
      //            for now assume that the maximum amplitude is the amplitude in the given profiles
      // TODO: RDD: bounds should be passed separately with min/max configuration
      search.bounds->setVal(0.1, 0);
      search.bounds->setVal(new_stim_loc_profiles[i][0].amplitude, 1);
    }
    stim_loc_profiles = new_stim_loc_profiles;
  }

  void ExperCPS::GetNextEvent() {
    CMatrix* stim_pars = search.get_next_sample();

    // TODO: RDD: update to select between multiple stim sites
    // TODO: all: LATER currently assuming that stim parameters not being searched over are set separately in
    //            stim_loc_profiles, should ensure these are set in Settings.cpp
    if (stim_loc_profiles[0].size() != 1) {
      Throw_RC_Error("ExperCPS::GetNextEvent: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }
    CSStimChannel stim_chan((stim_loc_profiles[0][0]));

    // set stim parameter values for next search point
    // TODO: RDD: LATER may want to scale down into more desirable interval, e.g., [0, 1]

    // convert to allowable discrete stim settings
    // stim_pars amplitude in mA
    // CSStimChannel.amplitude in uA but allowed stim values in increments of 100 uA
    // TODO: RDD: check this conversion works
    stim_chan.amplitude = ((uint16_t)(stim_pars->getVal(0)*10 + 0.5)) * 100;

    ExpEvent ev;
    ev.active_ms = stim_chan.duration/1000;
    // TODO: RDD: reevaluate ISI values
    // TODO: RDD: choose whether to use ISIs or fixed lockouts
    // Add ISI
    ev.event_ms = ev.active_ms +
      rng.GetRange(cps_specs.intertrial_range_ms[0],
          cps_specs.intertrial_range_ms[1]);

    CSStimProfile stim_profile;
    stim_profile += stim_chan;
    stim_profiles += stim_profile;
    delete stim_pars;
    exp_events += ev;
  }

  void ExperCPS::UpdateSearch(const CSStimProfile stim_info, const ExpEvent ev, const double biomarker) {
    if (stim_info.size() != 1) {
      Throw_RC_Error("ExperCPS::UpdateSearch: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }

    // ExpEvent ev currently unused; API allows for future experiments in which stim duration is tuned
    CMatrix stim_pars(1, 1);
    // map stim parameters for search model
    // TODO: RDD: remove magic numbers for conversion factors, 
    //            easy to forget to change in both UpdateSearch() and GetNextEvent()
    stim_pars(0) = ((double)(stim_info[0].amplitude))/1000;

    CMatrix biomarker_mat(biomarker);
    search.add_sample(stim_pars, biomarker_mat);
  }


  void ExperCPS::DoConfigEvent(const CSStimProfile& profile) {
    JSONFile config_event = MakeResp("CONFIG");
    config_event.Set(JSONifyCSStimProfile(profile), "data");
    status_panel->SetEvent("PRESET");
  }


  void ExperCPS::UpdateSearchPanel(const CSStimProfile& profile) {
    JSONFile update_event = MakeResp("UPDATE SEARCH");
    // update_event.Set(JSONifyCSStimProfile(profile), "data");
    for (int i = 0; i < profile.size(); i++) {
      CSStimChannel chan = profile[i];
      update_event.Set(chan.electrode_pos, "data", i, "electrode_pos");
      update_event.Set(chan.electrode_neg, "data", i, "electrode_neg");
      update_event.Set(chan.amplitude, "data", i, "amplitude");
    }
    hndl->event_log.Log(update_event.Line());
  }


  void ExperCPS::ClassifyingPanel() {
    JSONFile classif_event = MakeResp("CLASSIFYING");
    hndl->event_log.Log(classif_event.Line());
    status_panel->SetEvent("CLASSIFYING");
  }


  void ExperCPS::DoStimEvent(const CSStimProfile& profile) {
    JSONFile stim_event = MakeResp("STIM");
    stim_event.Set(JSONifyCSStimProfile(profile), "data");
    hndl->event_log.Log(stim_event.Line());
    status_panel->SetEvent("STIM");
  }


  JSONFile ExperCPS::JSONifyCSStimProfile(const CSStimProfile& profile) {
    JSONFile stim_event;
    for (int i = 0; i < profile.size(); i++) {
      CSStimChannel chan = profile[i];
      stim_event.Set(chan.electrode_pos, i, "electrode_pos");
      stim_event.Set(chan.electrode_neg, i, "electrode_neg");
      stim_event.Set(chan.amplitude, i, "amplitude");
      stim_event.Set(chan.frequency, i, "frequency");
      stim_event.Set(chan.duration, i, "duration");
      stim_event.Set(chan.area, i, "area");
      stim_event.Set(chan.burst_frac, i, "burst_frac");
      stim_event.Set(chan.burst_slow_freq, i, "burst_slow_freq");
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

    // Confirm window for run time.
    if (!ConfirmWin(RC::RStr("Total run time will be ") + experiment_duration / (60 * 1000) + " min. "
          + (experiment_duration * 1000) % 60 + " sec.", "Session Duration")) {
      hndl->StopExperiment();
      return;
    }

    exp_start = RC::Time::Get();

    JSONFile startlog = MakeResp("START");
    hndl->event_log.Log(startlog.Line());

    GetNextEvent();
    hndl->task_classifier_manager->ProcessClassifierEvent(
        ClassificationType::NORMALIZE, classify_ms, 0);
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
    
    // TODO RDD/RC: how could I safely make separate data logs in the session directory?
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
    hndl->event_log.Log(best_stim_json.Line());

    JSONFile data_log;
    // save all other data for analysis
    for (int i = 0; i < classif_results.size(); i++) {
      data_log.Set(classif_results[i], "analysis_data", "classif_results", i);
      data_log.Set(JSONifyCSStimProfile(stim_profiles[i]), "analysis_data", "stim_profiles", i);
      data_log.Set(exp_events[i].active_ms, "analysis_data", "exp_events", i, "active_ms");
      data_log.Set(exp_events[i].event_ms, "analysis_data", "exp_events", i, "event_ms");
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


  void ExperCPS::WaitUntil(uint64_t target_ms) {
    // delays until target_ms milliseconds from the start of the experiment
    // breaks delay every 50 ms to allow for stopping the experiment
    f64 cur_time = RC::Time::Get();
    uint64_t current_time_ms = uint64_t(1000*(cur_time - exp_start)+0.5);

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
    uint64_t delay_next = delay ? delay < min_delay : min_delay;
    while (delay > 0) {
      delay_next = delay ? delay < min_delay : min_delay;
      delay -= delay_next;
      QThread::msleep(delay_next);
    }
  }


  void ExperCPS::ClassifierDecision_Handler(const double& result,
        const TaskClassifierSettings& task_classifier_settings) {
    // record classifier outcomes
    classif_results += result;
    exper_classif_settings += task_classifier_settings;
  }


  void ExperCPS::StimDecision_Handler(const bool& stim_event, const TaskClassifierSettings& classif_settings, const f64& stim_time_from_start_sec) {
    // TODO: RDD: add docstrings for everything
    uint64_t cur_time_ms = uint64_t(1000*(stim_time_from_start_sec - exp_start)+0.5);

    // TODO: RDD: ensure that last events in all saved event arrays can be disambiguated 
    // (i.e., if any array is longer than another because the experiment stopped in some particular place, 
    // ensure the odd element out can be identified)
    // TODO: RDD/JPB: ensure that stim events are set off as close as possible to this callback being called
    //                might be simplest to let this function control the stim?
    //                do/can we receive a callback when a stim event ends?
    abs_event_times += cur_time_ms;
    // TODO: RDD: setting off classification events in this function starts the data collection
    //            process (currently), meaning that the classification EEG collection interval is 
    //            bounded by the time at which ExperCPS calls for classification and that time
    //            plus the classif_ms classification EEG collection duration
    //            need to update the event timing to record those times

    // TODO: RDD: times to record: EEG collection onset/offset for each classification event,
    //                             stim onset/offset for each stim/sham event
    //                             also, return callback return times both for this handler, 
    //                             ClassifierDecision_Handler, and for the StimManager handler

    // all classification requests indexed with classif_id (which will not align with pre-stim/post-stim event indices)
    double result = classif_results[classif_id];
    // TODO: RDD/JPB: confirm whether StimManager event from current cycle will always return before next Classifier event does to ensure correct ordering.
    TaskClassifierSettings task_classifier_settings = exper_classif_settings[classif_id];
    ClassificationType classif_state = task_classifier_settings.cl_type;

    // pre-stim/post-stim events indexed with cur_ev
    CSStimProfile stim_params = stim_profiles[cur_ev];
    ExpEvent cur_event = exp_events[cur_ev];

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler\n"));
    #endif

    // TODO: RDD: add code for computing final optimal parameters after experiment completed
    //      could be completed entirely off-line, probably simplest, but would be nice to simply compute live
    //      would then need to end experiment for subjet while continuing computation, more complicated...
    // TODO: RDD: need to set classifyms and id for all calls to classifier class
    // TODO define minimum numbers of events for each stim location/sham for experiment

    // store stim event results
    stim_event_flags += stim_event;

    ClassificationType next_classif_state;
    // TODO logging
    if (classif_state == ClassificationType::STIM ||
        classif_state == ClassificationType::SHAM) {  // received pre-stim/pre-sham classification event
      // TODO ensure/confirm with James that <stim_event == classif_prob < 0.5> and not <stim_event == whether stim event actually occurred, which would filter out sham events in addition to high memory states>
      // this could be a tangible difference between the NOSTIM/SHAM events, that stim_event := false for NOSTIM, stim_event := classif_prob < 0.5 for SHAM
      if (stim_event) { // stim event occurred or would have occurred if the event were not a sham
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler: stim decision\n"));
        #endif
        // TODO handle cases where say stim duration not equal between given and used params...
        //      use assert as a placeholder for initial testing
        // TODO: RDD/RC: when will CereStim change received stim parameters internally and will we receive that
        //               info back in some way? no, we cannot receive it back, should be covered with Elemem 
        //               stim constraints being a superset of CereStim constraints, should all throw exceptions
        //               and shut down the experiment
        // assert(stim_params == stim_profiles[cur_ev]);
        // if (stim_params != stim_profiles[cur_ev]) {
        //   // TODO: RDD: add warning message here
        //   warning
        //   stim_profiles[cur_ev] = stim_params;
        // }
        // prev_sham = classif_state == ClassificationType::SHAM;
        // get post-stim classifier output
        // TODO: RDD/JPB: when will stim event offset be relative to this callback being called?
        //                can we get that info from CereStim? Or can we use nominal duration instead?
        uint64_t stim_offset_ms = abs_event_times[abs_event_times.size()] + stim_params[0].duration;
        next_min_event_time = stim_offset_ms + poststim_classif_lockout_ms;
        next_classif_state = ClassificationType::NOSTIM;
        if (classif_state == ClassificationType::STIM) { DoStimEvent(stim_profiles[cur_ev]); }
        else { DoShamEvent(); }
      }
      else { // good memory state detected and stim event would not have occurred
        // TODO: RDD/JPB: add some timing control? or maximize classification rate? in any case should record event times
        //                to analyze event time frequencies, may want to add some jitter if fairly low variance
        // TODO: RDD/RC: need to make separate handler for grabbing time interval of classification features from FeatureFilters class
        // keep classifying until a poor memory state is detected
        next_min_event_time = cur_time_ms;
        next_classif_state = classif_state;
        ClassifyingPanel();

        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler: sham decision\n"));
        #endif
        // TODO always access results[], classif_results[], and any other event arrays with last event
      }
    }
    else if (classif_state == ClassificationType::NOSTIM) { // received post-stim/post-sham classification event
      // TODO: RDD/JPB: get stim_offset_ms from classifier or from CereStim handler class, don't compute here if possible
      // though errors wouldn't accumulate too much
      uint64_t stim_offset_ms = abs_event_times[abs_event_times.size() - 1] + stim_params[0].duration;
      next_min_event_time = stim_offset_ms + stim_lockout_ms;
      status_panel->SetEvent("UPDATING");

      double biomarker = result - classif_results[classif_results.size() - 1];
      // TODO add struct for storing everything related to a full pre-post event (i.e., pre-stim classifier event time, stim event time, post-stim classifier time, classifier results, biomarker, stim parameters, ideally Bayesian search hps? definitely should store those separately...)
      //      otherwise could just parse after the fact, this would prevent misparsing...
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler: nostim decision\n"));
      #endif
      if (!prev_sham) {
        #ifdef DEBUG_EXPERCPS
        RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler: nostim decision with prev stim\n"));
        #endif
        UpdateSearch(stim_params, cur_event, biomarker);
        GetNextEvent();
        cur_ev++;
        UpdateSearchPanel(stim_profiles[cur_ev]);
        
        // run pre-event (stim configuration) as soon as stim parameters are available
        DoConfigEvent(stim_profiles[cur_ev]);
        if (cur_ev < exp_events.size()) {
          CSStimProfile next_stim_profile = stim_profiles[cur_ev];
          hndl->stim_worker.ConfigureStimulation(next_stim_profile);
        }
        else {
          Throw_RC_Error("Event requested before being allocated by the search process.");
        }
      }
      
      // TODO update to select between sham and stim events based on some criteria
      // Randomize all the experiment events.
      // exp_events.Shuffle();
      if (cur_ev % 5) {
        next_classif_state = ClassificationType::STIM;
      }
      else {
        next_classif_state = ClassificationType::SHAM;
        // TODO: RDD: set sham duration/event info? how are sham events controlled in StimulationManager?
      }
      status_panel->SetEvent("CLASSIFYING");
    }
    else if (classif_state == ClassificationType::NORMALIZE) {
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler: normalization decision\n"));
      #endif
      ClassifyingPanel();
      next_min_event_time = cur_time_ms + normalize_lockout_ms;
      if (classif_results.size() < n_normalize_events) {
        // TODO log event info
        // TODO add some jitter? add (explicit) jitter to timeouts in general?
        next_classif_state = ClassificationType::NORMALIZE;
      }
      else if (classif_results.size() == n_normalize_events) {
        // TODO log event info
        // TODO update to select between sham and stim events based on some criteria
        next_classif_state = ClassificationType::STIM;
      }
      else {
        Throw_RC_Error("Normalization event requested after n_normalize_events normalization events completed.");
      }
    }
    else {
      Throw_RC_Error("Invalid classification type received.");
    }

    // run next classification result (which conditionally calls for stimulation events)
    if (next_min_event_time > experiment_duration) {
      // TODO: RDD/RC: preferred method for ending experiments?
      InternalStop();
    }
    prev_sham = next_classif_state == ClassificationType::SHAM;
    WaitUntil(next_min_event_time);
    // TODO: RDD/JPB: use id = 0 for now, how much performance improvement would classifier queueing add?
    // TODO: RDD: remove classify_ms
    hndl->task_classifier_manager->ProcessClassifierEvent(
        next_classif_state, classify_ms, classif_id);
    classif_id++;

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
    // TODO: RDD: extend to tuning with arbitrary stim parameters
    CMatrix* best_sol_mat = search.get_best_solution();
    CSStimChannel chan = stim_profiles[0][0];
    chan.amplitude = best_sol_mat->getVal(0);
    best_stim_profile += chan;

    delete best_sol_mat;
  }
}

