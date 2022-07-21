#include "ExperCPS.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"
#include "BayesGPc/CMatrix.h"
#include "Settings.h"

using namespace RC;

namespace CML {
  ExperCPS::ExperCPS(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    AddToThread(this);

    seed = 1;
    n_var = 1;
    obsNoise = 0.1;
    exp_bias = 0.25;

    #ifdef DEBUG_EXPERCPS
    verbosity = 1;
    n_init_samples = 5;
    #else
    verbosity = 0;
    n_init_samples = 100;
    #endif

    // kernel for each stim site
    CMatern32Kern k(n_var);
    kern = CCmpndKern(n_var);
    kern.addKern(&k);
    CWhiteKern whitek(n_var);
    kern.addKern(&whitek);
    CMatrix b(1, 2);
    b(0, 0) = 0.2;
    b(0, 1) = 2.0;
    kern.setBoundsByName("matern32_0__lengthScale", b);
    b(0, 0) = 0.25;
    b(0, 1) = 4.0;
    kern.setBoundsByName("matern32_0__variance", b);
    b(0, 0) = 0.01;
    b(0, 1) = 4.0;
    kern.setBoundsByName("white_1__variance", b);

    // TODO: RDD: LATER load parameters from config file? Or should experimental parameters be hard-coded to prevent accidental changes?

    // TODO: RDD: confirm values with design docs
    // TODO: RDD: what Morlet wavelet frequencies are being used? PS4 only went from 6 Hz to 180 Hz
    // TODO: RDD: original PS4 used only 500 ms classification interval for post-stim interval; can we get away with that for both intervals?

    // CPS task parameters
    #ifdef DEBUG_EXPERCPS
    n_normalize_events = 2;
    classify_ms = 500;
    #else
    n_normalize_events = 25;
    classify_ms = 1366;  // TODO: consider shortening for more events; check classifier performance with shorter feature intervals, higher min freqs
    #endif
    poststim_classif_lockout_ms = 30;

    // classifier event counter
    classif_id = 0;
    search_order_idx = 0;
    _init();
  }

  void ExperCPS::_init() {
    eeg_times.Clear();
    stim_times.Clear();
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
    beat_sham = false;
  }

  void ExperCPS::SetStimProfiles_Handler(
        const RC::Data1D<StimProfile>& new_min_stim_loc_profiles,
        const RC::Data1D<StimProfile>& new_max_stim_loc_profiles) {
//    #ifdef DEBUG_EXPERCPS
//    RC_DEBOUT(RC::RStr("ExperCPS::SetStimProfiles\n"));
//    #endif
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
    if (cps_specs.intertrial_range_ms[0] > cps_specs.intertrial_range_ms[1]) {
      Throw_RC_Error("Configuration file error: intertrial_range_ms[0] > intertrial_range_ms[1]");
    }
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
      #ifdef DEBUG_EXPERCPS
      #else
      uint64_t min_pre_post_len = new_min_stim_loc_profiles[i][0].duration/1000 + poststim_classif_lockout_ms + 2 * classify_ms;
      if (cps_specs.intertrial_range_ms[0] < min_pre_post_len) {
        Throw_RC_Error((string("Configuration file error: intertrial_range_ms[0] < minimum possible pre-post event length of ")
                       + to_string(min_pre_post_len)
                       + string("ms. Please update the configuration file.\n")).c_str());
      }
      #endif
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
    vector<size_t> init_samples;
    vector<int> rng_seeds;

    CMatrix bounds(n_var, 2);

    // search grids
    vector<vector<CMatrix>> all_grid_vals;

    // set search bounds
    for (size_t i = 0; i < n_searches; i++) {
      // TODO: LATER RDD: fix this to allow for searching over arbitrary parameters;
      // TODO: LATER RDD: unclear how to handle sham stim duration when tuning over duration

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
      for (uint64_t i = 0; i < n_var; i++) {
          CMatrix grid1D = linspace(bounds.getVal(0),
                                    bounds.getVal(1),
                                    n_grid);
          grid_vals.push_back(grid1D);
      }
      all_grid_vals.push_back(grid_vals);
    }

    search = CSearchComparison(n_searches, 0.05, kernels, param_bounds, observation_noises,
        exploration_biases, init_samples, rng_seeds, verbosity, all_grid_vals);

    // log parameter search metadata
    JSONFile metadata = MakeResp("PS_METADATA");
    metadata.Set(search.models[0].json_structure(), "data");
    hndl->event_log.Log(metadata.Line());

    min_stim_loc_profiles = new_min_stim_loc_profiles;
    max_stim_loc_profiles = new_max_stim_loc_profiles;
  }

  void ExperCPS::GetNextEvent(unsigned int model_idx) {
    // model_idx is zero-indexed into the set of stim sites while search_order_idx and model_idxs are zero-indexed from sham and
    // then follow same order as the stim sites
    CMatrix next_pars = search.get_next_sample(model_idx);

    // TODO: all: LATER currently assuming that stim parameters not being searched over are set separately in
    //            stim_loc_profiles, should add these to Settings.cpp
    StimChannel stim_chan((max_stim_loc_profiles[model_idx][0]));

    // set stim parameter values for next search point
    // TODO: RDD: LATER may want to scale down into more desirable interval, e.g., [0, 1]

    // pars amplitude in mA
    // StimChannel.amplitude in uA but allowed stim values in increments of 100 uA
    stim_chan.amplitude = ((uint16_t)(next_pars.getVal(0)*10 + 0.5)) * 100;

    ExpEvent ev;
    ev.active_ms = stim_chan.duration/1000;

    // TODO: RDD/RC: does controlling classification timing instead of stim timing make sense?
    // Add ITI between classification events since stim onsets cannot be precisely controlled with variable classification processing time intervals
    ev.event_ms = rng.GetRange(cps_specs.intertrial_range_ms[0],
        cps_specs.intertrial_range_ms[1]);

    StimProfile next_stim_profile;
    next_stim_profile += stim_chan;
    stim_profiles[model_idx] += next_stim_profile;
    // index zero for sham
    exp_events[model_idx + 1] += ev;
  }


  void ExperCPS::LogUpdate(UpdateEvent ev) {
    // TODO: RDD/RC: discuss logging StimProfile instead of single StimChan (as in OPS) for future multi-site stim

    JSONFile update_json = MakeResp("UPDATE");
    update_json.Set(ev.start_time, "time");
    update_json.Set(ev.loaded, "loaded");
    update_json.Set(ev.state, "data");
    update_json.Set(ev.model_idx, "data", "model_index");
    // log (x, biomarker) pairs
    update_json.Set(ev.opt_params, "data", "x");  // only contains parameters that are tuned in format/units converted for optimization
    update_json.Set(ev.biomarker, "data", "biomarker");
    update_json.Set(StimChannel2JSON(ev.stim_params[0]), "data", "stim_params");
    hndl->event_log.Log(update_json.Line());
  }


  // updates parameter optimization model, gets next optimization sample, and logs update
  void ExperCPS::UpdateSearch(const unsigned int model_idx, const StimProfile stim_profile, const double biomarker) {
    // model_idx is zero-indexed into the set of stim sites while search_order_idx and model_idxs are zero-indexed from sham and
    // then follow same order as the stim sites
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::UpdateSearch\n"));
    #endif

    UpdateSearchPanel();
    if (stim_profile.size() != 1) {
      Throw_RC_Error("ExperCPS::UpdateSearch: Only stimulation events with stimulation profiles of length 1 are supported currently.");
    }
    UpdateEvent ev;
    ev.loaded = false;
    // TODO: RDD: fill in with BO state; need to add BO/CSearch state/structure json functions

    ev.biomarker = biomarker;
    ev.model_idx = model_idx;
    ev.stim_params = stim_profile;

    // TODO: RDD: need to handle all RC errors; InternalStop() and shut down experiment

    CMatrix pars(1, 1);
    // map stim parameters for search model
    // TODO: RDD: remove magic numbers for conversion factors, 
    //            easy to forget to change in both UpdateSearch() and GetNextEvent()
    pars(0) = ((double)(ev.stim_params[0].amplitude))/1000;

    CMatrix biomarker_mat(ev.biomarker);
    ev.start_time = RC::Time::Get()*1e3;
    search.add_sample(model_idx, pars, biomarker_mat);
    ev.opt_params = to_vector(pars);
    GetNextEvent(model_idx);

    // get state after update
    ev.state = search.models[model_idx].json_state();
    LogUpdate(ev);
  }

  JSONFile ExperCPS::StimChannel2JSON(StimChannel chan) {
    JSONFile j;
    j.Set(uint32_t(chan.electrode_pos), "electrode_pos");
    j.Set(uint32_t(chan.electrode_neg), "electrode_neg");
    j.Set(chan.amplitude*1e-3, "amplitude");
    j.Set(chan.frequency, "frequency");
    j.Set(chan.duration*1e-3, "duration");

    // Log burst data if it's a burst.
    if (chan.burst_frac != 1) {
      j.Set(chan.burst_slow_freq, "burst_slow_frequency");
      j.Set(chan.burst_frac, "burst_fraction");
    }
    return j;
  }


  StimChannel ExperCPS::JSON2StimChannel(JSONFile j) {
    StimChannel chan;
    double amplitude;
    double duration;
    j.Get(chan.electrode_pos, "electrode_pos");
    j.Get(chan.electrode_neg, "electrode_neg");
    j.Get(amplitude, "amplitude");
    chan.amplitude = amplitude * 1e3;
    j.Get(chan.frequency, "frequency");
    j.Get(duration, "duration");
    chan.duration = duration * 1e3;

    // Log burst data if it's a burst.
    if (j.json.contains("burst_frac")) {
      j.Get(chan.burst_slow_freq, "burst_slow_frequency");
      j.Get(chan.burst_frac, "burst_fraction");
    }
    return chan;
  }

  void ExperCPS::ValidateStimulationProfile(const StimProfile& profile, const unsigned int profile_idx) {
    // validates whether a given stim profile parameters fall within the safe stimulation ranges
    // determined within the experiment configuration file and the safe stimulation determination clinical process

    if (profile.size() != 1) {
        Abort();
        Throw_RC_Error("CPS: Only single-channel stimulation profiles supported.");
    }

    StimProfile max_profile = max_stim_loc_profiles[profile_idx];
    StimProfile min_profile = min_stim_loc_profiles[profile_idx];
    RC::RStr message;
    std::string correct_value;
    std::string received_value;
    if (profile[0].electrode_pos != max_profile[0].electrode_pos) {
      message = RC::RStr("CPS: Stimulation profile with incorrect cathode requested for configuration");
      correct_value = to_string(max_profile[0].electrode_pos);
      received_value = to_string(profile[0].electrode_pos);
    }
    else if (profile[0].electrode_neg != max_profile[0].electrode_neg) {
      message = RC::RStr("CPS: Stimulation profile with incorrect anode requested for configuration");
      correct_value = to_string(max_profile[0].electrode_neg);
      received_value = to_string(profile[0].electrode_neg);
    }
    else if (profile[0].frequency > max_profile[0].frequency) {
      message = RC::RStr("CPS: Stimulation profile with frequency above max safe frequency requested for configuration");
      correct_value = to_string(max_profile[0].frequency);
      received_value = to_string(profile[0].frequency);
    }
    else if (profile[0].frequency < min_profile[0].frequency) {
      message = RC::RStr("CPS: Stimulation profile with frequency below min safe frequency requested for configuration");
      correct_value = to_string(min_profile[0].frequency);
      received_value = to_string(profile[0].frequency);
    }
    else if (profile[0].duration > max_profile[0].duration) {
      message = RC::RStr("CPS: Stimulation profile with duration above max safe duration requested for configuration");
      correct_value = to_string(max_profile[0].duration);
      received_value = to_string(profile[0].duration);
    }
    else if (profile[0].duration < min_profile[0].duration) {
      message = RC::RStr("CPS: Stimulation profile with duration below min safe duration requested for configuration");
      correct_value = to_string(min_profile[0].duration);
      received_value = to_string(profile[0].duration);
    }
    else if (profile[0].area != max_profile[0].area) {  // fixed with contact
      message = RC::RStr("CPS: Stimulation profile with incorrect contact area requested for configuration");
      correct_value = to_string(max_profile[0].area);
      received_value = to_string(profile[0].area);
    }
    else if (profile[0].burst_frac > max_profile[0].burst_frac) {
      message = RC::RStr("CPS: Stimulation profile with burst_frac above max safe burst_frac requested for configuration");
      correct_value = to_string(max_profile[0].burst_frac);
      received_value = to_string(profile[0].burst_frac);
    }
    else if (profile[0].burst_frac < min_profile[0].burst_frac) {
      message = RC::RStr("CPS: Stimulation profile with burst_frac below min safe burst_frac requested for configuration");
      correct_value = to_string(min_profile[0].burst_frac);
      received_value = to_string(profile[0].burst_frac);
    }
    else if (profile[0].burst_slow_freq > max_profile[0].burst_slow_freq) {
      message = RC::RStr("CPS: Stimulation profile with burst_slow_freq above max safe burst_slow_freq requested for configuration");
      correct_value = to_string(max_profile[0].burst_slow_freq);
      received_value = to_string(profile[0].burst_slow_freq);
    }
    else if (profile[0].burst_slow_freq < min_profile[0].burst_slow_freq) {
      message = RC::RStr("CPS: Stimulation profile with burst_slow_freq below min safe burst_slow_freq requested for configuration");
      correct_value = to_string(min_profile[0].burst_slow_freq);
      received_value = to_string(profile[0].burst_slow_freq);
    }
    else if (profile[0].amplitude > max_profile[0].amplitude) {
      message = RC::RStr("CPS: Stimulation profile with amplitude above max safe amplitude requested for configuration");
      correct_value = to_string(max_profile[0].amplitude);
      received_value = to_string(profile[0].amplitude);
    }
    else if (profile[0].amplitude < min_profile[0].amplitude) {
      message = RC::RStr("CPS: Stimulation profile with amplitude below min safe amplitude requested for configuration");
      correct_value = to_string(min_profile[0].amplitude);
      received_value = to_string(profile[0].amplitude);
    }
    if (message.length() > 0) {
      message = message
          + RC::RStr(" for stim profile ") + RC::RStr(to_string(profile_idx))
          + RC::RStr(". Correct value: ") + RC::RStr(correct_value)
          + RC::RStr(" . Received value: ") + RC::RStr(received_value)
          + RC::RStr(" . Aborting experiment.");
      DebugLog(message);
      Abort();
      Throw_RC_Error(message.c_str());
    }
  }

  void ExperCPS::DoConfigEvent(const StimProfile& profile, const unsigned int profile_idx) {
    // model_idx is zero-indexed to stim parameter sets, not sham
    // #ifdef DEBUG_EXPERCPS
    // RC_DEBOUT(RC::RStr("ExperCPS: enter DoConfigEvent \n\n"));
    // #endif
    JSONFile config_event = MakeResp("CONFIG_STIM");
    config_event.Set(JSONifyStimProfile(profile), "data");
    hndl->event_log.Log(config_event.Line());
    status_panel->SetEvent("PRESET");

    ValidateStimulationProfile(profile, profile_idx);
    hndl->stim_worker.ConfigureStimulation(profile);
  }


  void ExperCPS::UpdateSearchPanel() {
    status_panel->SetEvent("UPDATE SEARCH");
  }


  void ExperCPS::LogNormalize(NormalizeEvent ev) {
    JSONFile resp = MakeResp("NORMALIZE");
    resp.Set(ev.start_time, "time");
    hndl->event_log.Log(resp.Line());
  }


  void ExperCPS::NormalizingPanel() {
    status_panel->SetEvent("NORMALIZING");
  }


  void ExperCPS::LogClassify(ClassifyEvent ev) {
    JSONFile d;
    d.Set(ev.result, "result");
    d.Set(ev.decision, "decision");
    d.Set(ev.settings.duration_ms, "duration");
    const RC::RStr type = [&] {
        switch (ev.settings.cl_type) {
          case ClassificationType::STIM: return "CLASSIFY_STIM_CPS";
          case ClassificationType::SHAM: return "CLASSIFY_SHAM_CPS";
          case ClassificationType::NOSTIM: return "CLASSIFY_NOSTIM_CPS";
          default: Throw_RC_Error("Invalid classification type received.");
        }
    }();
    JSONFile resp = MakeResp(type, ev.settings.classif_id, d);
    resp.Set(ev.start_time, "time");
    hndl->event_log.Log(resp.Line());
  }


  void ExperCPS::ClassifyingPanel() {
    status_panel->SetEvent("CLASSIFYING");
  }


  void ExperCPS::StimPanel() {
    status_panel->SetEvent("STIM");
  }


  // TODO: RDD: uses same format as code that logs stim events in StimManager (?)
  JSONFile ExperCPS::JSONifyStimProfile(const StimProfile& profile) {
    JSONFile stim_event;
    stim_event.Set(nlohmann::json::array(), "stim_profile");
    for (size_t i = 0; i < profile.size(); i++) {
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

  void ExperCPS::LogSham(ShamEvent ev) {
    JSONFile sham_event = MakeResp("SHAM");
    sham_event.Set(cps_specs.sham_duration_ms, "data", "duration");
    sham_event.Set(ev.start_time, "time");
    hndl->event_log.Log(sham_event.Line());
  }

  void ExperCPS::DoShamEvent() {
    status_panel->SetEvent("SHAM");
  }


  void ExperCPS::Setup_Handler(const RC::Data1D<RC::RStr>& prev_sessions) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::Setup_Handler\n"));
    #endif

    // load update events from previous sessions
    for (size_t idx = 0;  idx < prev_sessions.size(); idx++) {
      RC::RStr filename = File::FullPath(prev_sessions[idx], "event.log");
      RC::FileRead fr;
      if (!fr.Open(filename)) {  // check that session directory has experiment config file
        RC::RStr message = RC::RStr("CPS: Previous session at\n") + filename
                                    + RC::RStr("\nfor subject could not be loaded.");
        if (!ConfirmWin(RC::RStr("WARNING!  ") + message + RC::RStr("  Continue?"))) {
          Abort();
          Throw_RC_Error(message.c_str());
        }
        continue;
      }

      RC::Data1D<RC::RStr> events;
      fr.ReadAll(events);

      for (size_t i = 0; i < events.size(); i++) {
        RC::RStr event_str(events[i]);
        json event = json::parse(event_str.Raw());
        RC::RStr ev_type(event.at("type").dump());
        if ((ev_type.length() > 0) && (ev_type.find("UPDATE") < ev_type.length())) {
          // Don't load events that were loaded into previous sessions since those will
          // be loaded separately assuming the previously loaded session directory is present
          if (event["loaded"]) { continue; }
          unsigned int neg = event["data"]["stim_params"]["electrode_neg"];
          unsigned int pos = event["data"]["stim_params"]["electrode_pos"];
          // map model indices from previous sessions based on contact numbers
          bool no_match = true;
          unsigned int m;
          for (size_t j = 0; j < n_searches; j++) {
            if (max_stim_loc_profiles[j][0].electrode_neg == neg
                    && max_stim_loc_profiles[j][0].electrode_pos == pos) {
              m = j;
              no_match = false;
              break;
            }
          }
          if (no_match) { continue; }

          double bio = event["data"]["biomarker"];
          CMatrix biomarker_mat(bio);
          std::vector<std::vector<double>> x = event["data"]["x"];
          CMatrix xmat = from_vector(x);
          JSONFile j;
          j.json = event["data"]["stim_params"];
          StimChannel chan = JSON2StimChannel(j);

          StimProfile prof;
          prof += chan;
          // TODO: RDD: add bounds checking on CBayesianSearch functions
          search.add_sample(m, xmat, biomarker_mat);

          UpdateEvent ev;
          ev.loaded = true;
          ev.start_time = RC::Time::Get()*1e3;
          ev.biomarker = bio;
          ev.model_idx = m;
          ev.stim_params = prof;
          ev.opt_params = to_vector(xmat);
          // null model state in ev indicates pre-loaded update events
          LogUpdate(ev);
        }
      }
    }

    // add initial stim profiles
    stim_profiles = RC::Data1D<RC::Data1D<StimProfile>>(n_searches);
    exp_events = RC::Data1D<RC::Data1D<ExpEvent>>(n_searches + 1);  // index zero corresponds to sham events
    for (size_t i = 0; i < n_searches; i++) { GetNextEvent(i); }
    // separately add ExpEvent for first sham
    ExpEvent ev;
    ev.active_ms = cps_specs.sham_duration_ms;
    ev.event_ms = rng.GetRange(cps_specs.intertrial_range_ms[0],
        cps_specs.intertrial_range_ms[1]);
    exp_events[0] += ev;

    // Randomize order of stim locations/sham. Ensure stim locations and/or sham are not repeated consecutively.
    search_order.Shuffle();

    // configuration for initial event
    if (search_order[search_order_idx]) {  // stim event
      unsigned int next_idx = search_order[search_order_idx] - 1;
      StimProfile next_stim_profile = stim_profiles[next_idx][stim_profiles[next_idx].size() - 1];
      DoConfigEvent(next_stim_profile, next_idx);
    }

    model_idxs += search_order[search_order_idx];

    //    cur_ev = 0;
    event_time = 0;
    next_min_event_time = 0;
  }


  void ExperCPS::Start_Handler(const uint64_t& duration_s) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::Start_Handler\n"));
    #endif

    exp_start = RC::Time::Get();

    JSONFile startlog = MakeResp("START");
    hndl->event_log.Log(startlog.Line());

    // TODO: RDD: confirm duration_s is logged; add option to set this to whatever I want for testing

    // Total run time (ms), fixes experiment length for CPS.
    experiment_duration = duration_s * 1000;

    if (!ShouldAbort()) {
      TriggerAt(0, ClassificationType::NORMALIZE);
    }
  }


  void ExperCPS::Stop_Handler() {
    Abort();
  }


  void ExperCPS::InternalStop() {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::InternalStop\n"));
    #endif
//    ComputeBestStimProfile();

    // log best stim profile

    // TODO: RDD: logging needs to occur throughout the experiment, not just at the ends
    
    // TODO RDD/RC: safely make separate data logs in the session directory
    //              e.g. for convenience of analysis

    // JSONFile best_stim_log;
    // best_stim_log.Load(File::FullPath(hndl->session_dir,
    //       "experiment_config.json"));
//    JSONFile best_stim_json;
//    best_stim_json.Set(JSONifyStimProfile(best_stim_profile), "data", "best_stim_profile");
    // best_stim_log.Set("", "experiment", "stim_channels");
    // best_stim_log.Set(best_stim_json, "experiment", "stim_channels", 0);
    // best_stim_log.Save(File::FullPath(hndl->session_dir,
    //       "best_stim_parameters.json"));
//    if (beat_sham) {
//      best_stim_json.Set("true", "data", "beat sham");
//    }
//    else {
//      best_stim_json.Set("false", "data", "beat sham");
//    }
//    hndl->event_log.Log(best_stim_json.Line());

    JSONFile data_log;
    // save all data for analysis
    data_log.Set(nlohmann::json::array(), "analysis_data", "classif_results");
    data_log.Set(nlohmann::json::array(), "analysis_data", "stim_profiles");
    data_log.Set(nlohmann::json::array(), "analysis_data", "exp_events");
    for (size_t i = 0; i < classif_results.size(); i++) {
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
    for (size_t i = 0; i < n_searches; i++) {
      data_log.json["analysis_data"]["stim_profiles"].push_back(nlohmann::json::array());
      for (size_t j = 0; j < stim_profiles[i].size(); j++) {
        data_log.json["analysis_data"]["stim_profiles"][i].push_back(JSONifyStimProfile(stim_profiles[i][j]).json);
      }
    }

    // TODO: RDD: move these log statements and all others to be as early as possible? then can't have a single JSON line
    data_log.Set(nlohmann::json::array(), "analysis_data", "min_stim_loc_profiles");
    data_log.Set(nlohmann::json::array(), "analysis_data", "max_stim_loc_profiles");
    for (size_t i = 0; i < min_stim_loc_profiles.size(); i++) {
      data_log.json["analysis_data"]["min_stim_loc_profiles"].push_back(JSONifyStimProfile(min_stim_loc_profiles[i]).json);
      data_log.json["analysis_data"]["max_stim_loc_profiles"].push_back(JSONifyStimProfile(max_stim_loc_profiles[i]).json);
    }

    // data_log.Set(exper_classif_settings, "analysis_data", "exper_classif_settings");
    // data_log.Set(abs_event_times, "analysis_data", "abs_event_times");
    // data_log.Save(File::FullPath(hndl->session_dir,
    //       "experiment_data.json"));
//    hndl->event_log.Log(data_log.Line());

    Stop_Handler();
    JSONFile stoplog = MakeResp("EXIT");
    hndl->event_log.Log(stoplog.Line());
    hndl->StopExperiment();

    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::InternalStop exit\n"));
    #endif
  }


  uint64_t ExperCPS::TimeSinceExpStartMs() {
    f64 time_s = RC::Time::Get() - exp_start;
    return static_cast<uint64_t>(1000 * time_s + 0.5);
  }

  // convert from times relative to start to absolute times
  f64 ExperCPS::ToAbsoluteTime(uint64_t time_ms) { return static_cast<f64>(time_ms) + exp_start * 1e3; }

  uint64_t ExperCPS::WaitUntil(uint64_t target_time_ms) {
    // Delays until target_time_ms milliseconds from the start of the experiment
    // Breaks delay every 50 ms to allow for stopping the experiment
    // All times are in ms

    #ifdef DEBUG_EXPERCPS
    const u64 wait_start_time = TimeSinceExpStartMs();
    #endif

    const i64 max_inner_delay = 50;  // inner delay to ensure senitivity to stops
    event_time = target_time_ms;

    i64 remaining_delay = target_time_ms - TimeSinceExpStartMs();
    while (remaining_delay > 0)
    {
      if (ShouldAbort()) { break; }
      // Ensures the inner delay is at most max_inner_delay ms
      // TODO: RDD: James was using u64, i64 instead of uint64_t, change back?
      u64 inner_delay = std::min(remaining_delay, max_inner_delay);
      QThread::msleep(inner_delay);
      remaining_delay = target_time_ms - TimeSinceExpStartMs();
    }

//    #ifdef DEBUG_EXPERCPS
//    RC_DEBOUT(RC::RStr("ExperCPS::WaitUntil()\nIntended delay: ") + to_string(static_cast<i64>(target_time_ms - wait_start_time))
//              + RC::RStr(";\nActual delay: ") + to_string(TimeSinceExpStartMs() - wait_start_time)
//              + RC::RStr("\n"));
//    #endif

    // Return the current time after delay offset from the beginning of the experiment
    // (this could differ from target due to e.g. operating system delays)
    return TimeSinceExpStartMs();
  }


  // handles the timing of setting off the next event; handles pre-event general logging
  void ExperCPS::TriggerAt(const uint64_t& next_min_event_time, const ClassificationType& next_classif_state) {
    // run next classification result (which conditionally calls for stimulation events)
//    #ifdef DEBUG_EXPERCPS
//    RC_DEBOUT(RC::RStr("ExperCPS::TriggerAt ") + RC::RStr(to_string(next_min_event_time)) + RC::RStr("\n"));
//    #endif
    if (next_min_event_time > experiment_duration) { InternalStop(); }

    if (ShouldAbort()) { return; }
    eeg_times += WaitUntil(next_min_event_time);
    if (ShouldAbort()) { return; }

    #ifdef DEBUG_EXPERCPS
    clf_start_time = TimeSinceExpStartMs();
    #endif

    if (next_classif_state == ClassificationType::NORMALIZE) {
      // TODO: RDD: is logging handled by a separate thread? will logging be potentially bogged down with disk accesses?
      // TODO: RDD: move all logging to the end of the loop with proper constraints to ensure event ordering with race conditions? would
      NormalizeEvent ev {RC::Time::Get()*1e3};
      LogNormalize(ev);
    }
    else if (next_classif_state == ClassificationType::STIM) {
      // check that stim happens after lockout period
      if (eeg_times[eeg_times.size() - 1] + classify_ms - prev_stim_offset_ms < stim_lockout_ms) {
        Throw_RC_Error((string("Stimulation requested with onset before ") +
                        to_string(stim_lockout_ms) +
                        string(" ms after offset of previous stimulation event. Aborting experiment for safety.")).c_str());
        Abort();
      }
    }

    hndl->task_classifier_manager->ProcessClassifierEvent(
        next_classif_state, classify_ms, classif_id);
  }


  void ExperCPS::HandleNormalization_Handler(
      RC::APtr<const EEGPowers>& data,
      const TaskClassifierSettings& task_classifier_settings) {
     #ifdef DEBUG_EXPERCPS
     RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler\n"));
     #endif

    // TODO: RDD: confirm that last events in all saved event arrays can be disambiguated 
    // (i.e., if any array is longer than another because the experiment stopped in some particular place, 
    // ensure the odd element out can be identified)
    // experiment could abort at any point
    ClassificationType classif_state = task_classifier_settings.cl_type;
    exper_classif_settings += task_classifier_settings;

    ClassificationType next_classif_state;

    if (classif_state != ClassificationType::NORMALIZE) { return; }

    // TODO: RDD: log normalization event intervals; just make separate log function for each event type
    NormalizingPanel();
    next_min_event_time = event_time +
        rng.GetRange(cps_specs.intertrial_range_ms[0], cps_specs.intertrial_range_ms[1]);
    if (eeg_times.size() < n_normalize_events) {
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler request normalization event\n"));
      #endif
      // TODO log event info
      // TODO consider adding some jitter here and to timeouts in general
      next_classif_state = ClassificationType::NORMALIZE;
    }
    else if (eeg_times.size() == n_normalize_events) {
      // TODO log event info
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("ExperCPS::HandleNormalization_Handler request sham/stim event ")
                + RC::RStr(to_string(bool(search_order[search_order_idx]))) + RC::RStr("\n"));
      #endif
      if (search_order[search_order_idx]) { next_classif_state = ClassificationType::STIM; }
      else { next_classif_state = ClassificationType::SHAM; }
    }
    else {
      Throw_RC_Error("Normalization event requested after n_normalize_events normalization events completed.\n");
    }
    TriggerAt(next_min_event_time, next_classif_state);
  }


  void ExperCPS::ClassifierDecision_Handler(const double& result,
        const TaskClassifierSettings& task_classifier_settings) {
//    #ifdef DEBUG_EXPERCPS
//    RC_DEBOUT(RC::RStr("ExperCPS::ClassifierDecision_Handler\n"));
//    // ~220 ms on old clinical laptop for classification interval of 500 ms, ~470 ms for clf interval of 1366 ms
//    clf_handler_time = TimeSinceExpStartMs();
//    RC_DEBOUT(RC::RStr("\nTotal data collection/clf time: ") + to_string(static_cast<i64>(clf_handler_time - clf_start_time)));
//    #endif

    classif_decision_arrived = true;
    // record classifier outcomes
    classif_results += result;
    classif_id++;

    if (stim_decision_arrived) { ProcessEvent(); }
  }

  void ExperCPS::StimDecision_Handler(const bool& stim_event, const TaskClassifierSettings& classif_settings, const f64& stim_time_sec) {
    #ifdef DEBUG_EXPERCPS
    RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler decision: ") + RC::RStr(to_string(stim_event)) + RC::RStr("\n"));
    // 0 ms as expected
//    uint64_t stim_handler_time = TimeSinceExpStartMs();
//    if (classif_decision_arrived) {
//      RC_DEBOUT(RC::RStr("\nTime between clf and stim handlers: ") + to_string(static_cast<i64>(stim_handler_time - clf_handler_time)));
//    }
    #endif

    stim_decision_arrived = true;
    uint64_t cur_time_ms = uint64_t(1000*(stim_time_sec - exp_start)+0.5);

    stim_times += cur_time_ms;

    #ifdef DEBUG_EXPERCPS
//    RC_DEBOUT(RC::RStr("\nstim decision time: ") + to_string(static_cast<i64>(stim_times[stim_times.size() - 1])));
//    if (stim_times.size() > 1) {
//      RC_DEBOUT(RC::RStr("\nstim decision time diff: ") + to_string(static_cast<i64>(stim_times[stim_times.size() - 1] - stim_times[stim_times.size() - 2])));
//      RC_DEBOUT(RC::RStr("\nclf decision time diff: ") + to_string(static_cast<i64>(eeg_times[eeg_times.size() - 1] - eeg_times[eeg_times.size() - 2])));
//    }
//    if (stim_times.size() > 2) {
//      RC_DEBOUT(RC::RStr("\nstim decision time diff 2: ") + to_string(static_cast<i64>(stim_times[stim_times.size() - 1] - stim_times[stim_times.size() - 3])));
//      RC_DEBOUT(RC::RStr("\nclf decision time diff: ") + to_string(static_cast<i64>(eeg_times[eeg_times.size() - 1] - eeg_times[eeg_times.size() - 3]))
//      + RC::RStr("\n"));
//    }
    #endif

    // store stim event results
    stim_event_flags += stim_event;
    if (classif_decision_arrived) { ProcessEvent(); }
  }

  void ExperCPS::ProcessEvent() {
    // reset state variables
    classif_decision_arrived = false;
    stim_decision_arrived = false;

    uint64_t cur_time_ms = stim_times[stim_times.size() - 1];
    const bool stim_event = stim_event_flags[stim_event_flags.size() - 1];

    double result = classif_results[classif_results.size() - 1];
    TaskClassifierSettings task_classifier_settings = exper_classif_settings[exper_classif_settings.size() - 1];
    ClassificationType classif_state = task_classifier_settings.cl_type;

    ClassifyEvent ev {ToAbsoluteTime(eeg_times[eeg_times.size() - 1]), result, stim_event, task_classifier_settings};
    LogClassify(ev);

    unsigned int model_idx = model_idxs[model_idxs.size() - 1];
    ExpEvent cur_event = exp_events[model_idx][exp_events[model_idx].size() - 1];
    // stim duration of either next stim event (for pre-event) or previous stim event (for post-event)
    uint64_t event_duration_ms = cur_event.active_ms;

    StimProfile stim_params;
    ClassificationType next_classif_state;

    // main event loop logic
    if (classif_state == ClassificationType::STIM ||
        classif_state == ClassificationType::SHAM) {  // received pre-stim/pre-sham classification event
      if (stim_event) {  // stim event occurred or would have occurred if the event were not a sham
        #ifdef DEBUG_EXPERCPS
        if (classif_state == ClassificationType::STIM)
          { RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: STIM event received\n")); }
        else { RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: SHAM event received\n")); }
        #endif

        uint64_t stim_offset_ms = stim_times[stim_times.size() - 1] + event_duration_ms;
        //#ifdef DEBUG_EXPERCPS
        //RC_DEBOUT(RC::RStr("ExperCPS::StimDecision_Handler: just after stim_offset_ms assignment\n"));
        //#endif
        next_min_event_time = stim_offset_ms + poststim_classif_lockout_ms;
        next_classif_state = ClassificationType::NOSTIM;
        if (classif_state == ClassificationType::STIM) {
          prev_sham = false;
          StimPanel();
        }
        else {  // SHAM event
          prev_sham = true;
          ShamEvent sham_ev {RC::Time::Get()*1e3, cps_specs.sham_duration_ms};
          LogSham(sham_ev);
          DoShamEvent();
        }
        prev_stim_offset_ms = stim_offset_ms;
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

      // TODO: RDD: eliminate ExpEvent structs? Don't really need them?
      //            They add clutter and no value beyond API for extensions... should really just compress all data for each event type into a standard struct
      // get next event
      next_min_event_time = eeg_times[eeg_times.size() - 2] + cur_event.event_ms;

//      #ifdef DEBUG_EXPERCPS
//      RC_DEBOUT(RC::RStr("\nITI: ") + to_string(static_cast<i64>(cur_event.event_ms)));
//      RC_DEBOUT(RC::RStr("\npre-time: ") + to_string(static_cast<i64>(stim_times[stim_times.size() - 2])));
//      RC_DEBOUT(RC::RStr("\npost-time: ") + to_string(static_cast<i64>(stim_times[stim_times.size() - 1])));
//      RC_DEBOUT(RC::RStr("\nEvent duration: ") + to_string(static_cast<i64>(event_duration_ms)));
//      RC_DEBOUT(RC::RStr("\nIntended delay: ") + to_string(static_cast<i64>(next_min_event_time - TimeSinceExpStartMs())));
//      #endif

      status_panel->SetEvent("UPDATING");

      double biomarker = result - classif_results[classif_results.size() - 2];

      if (prev_sham) {
          sham_results.push_back(biomarker);
          ExpEvent ev;
          ev.active_ms = cps_specs.sham_duration_ms;
          // Add ISI
          ev.event_ms = rng.GetRange(cps_specs.intertrial_range_ms[0],
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

        UpdateSearch(model_idx - 1, stim_params, biomarker);
      }
      
      search_order_idx++;
      // Randomize stim locations/sham. Ensure stim locations and/or sham are not repeated consecutively.
      if (search_order_idx == search_order.size()) {
        RC_DEBOUT(RC::RStr("\nshuffling search_order\n"));
        ShuffleNoConsecutive(search_order);
        search_order_idx = 0;
      }
      #ifdef DEBUG_EXPERCPS
      RC_DEBOUT(RC::RStr("\nExperCPS::StimDecision_Handler: search_order[search_order_idx] ") + to_string(search_order[search_order_idx]) + RC::RStr("\n"));
      #endif

      if (search_order[search_order_idx]) {  // stim event
        next_classif_state = ClassificationType::STIM;
        // run pre-event (stim configuration) once stim parameters are available (after any randomization)
        // if needed, could reduce latency after sham events by configuring next stim set earlier 
        // immediately after sham events
        unsigned int next_idx = search_order[search_order_idx] - 1;
        StimProfile next_stim_profile = stim_profiles[next_idx][stim_profiles[next_idx].size() - 1];
        DoConfigEvent(next_stim_profile, next_idx);
      }
      else { next_classif_state = ClassificationType::SHAM; }  // sham event
      // log which stim location (or whether sham) is requested next
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
  }


  // Shuffle array. Ensure first element in shuffled array does not equal last element in <d> and that there are no equal consecutive elements.
  template <typename T>
  void ExperCPS::ShuffleNoConsecutive(RC::Data1D<T>& d) {
    T prev = d[d.size() - 1];
    bool two_in_row = true;
    while (d[0] == prev || two_in_row) {
      d.Shuffle();
      two_in_row = false;
      for (size_t p = 0; p < d.size() - 1; p++) {
        if (d[p] == d[p + 1]) { two_in_row = true; }
      }
    }
  }
}
