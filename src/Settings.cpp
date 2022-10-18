#include "Settings.h"
#include "RC/RC.h"
#include "ConfigFile.h"
#include "Popup.h"
#include "EEGDisplay.h"

#include <QCoreApplication>

//using namespace RC;


namespace CML {
  Settings::Settings() {
    Clear();
  }

  void Settings::LoadSystemConfig() {
    RC::RStr sys_conf_file =
      RC::File::FullPath(QCoreApplication::applicationDirPath(),
        "sys_config.json");

    RC::APtr<JSONFile> load_sys_conf = new JSONFile();
    RC::FileRead fr;
    if (!fr.Open(sys_conf_file)) {
      Throw_RC_Type(File, (sys_conf_file + " could not be opened.").c_str());
    }
    load_sys_conf->Load(fr);

    uint32_t chan_count;
    load_sys_conf->Get(chan_count, "channel_count");
    if (chan_count > std::numeric_limits<uint16_t>::max()) {
      Throw_RC_Type(File, (RC::RStr("channel_count exceeds maximum ") +
          "channel count of " +
          RC::RStr(std::numeric_limits<uint16_t>::max())).c_str());
    }

    sys_config = load_sys_conf.ExtractConst();
  }

  void Settings::Clear() {
    exp_config = nullptr;
    elec_config = nullptr;
    bipolar_config = nullptr;
    stimconf.Clear();
    min_stimconf.Clear();
    max_stimconf.Clear();
    stimgrid_amp_uA.Clear();
    stimgrid_freq_Hz.Clear();
    stimgrid_dur_us.Clear();

    ops_specs.Clear();

    stimloctest_chanind = size_t(-1);
    stimloctest_amp = size_t(-1);
    stimloctest_freq = size_t(-1);
    stimloctest_dur = size_t(-1);

    macro_sampling_rate = 0; // Desired
    micro_sampling_rate = 0; // Desired
    sampling_rate = 1000; // All channels. Micro if set, else macro value.
    binned_sampling_rate = sampling_rate;

    exper.clear();
    sub.clear();

    grid_exper = false;
    task_driven = true;
  }

  RC::Data1D<EEGChan> Settings::LoadElecConfig(RC::RStr dir) {
    RStr elecfilename =  exp_config->GetPath("electrode_config_file");
    if (File::Basename(elecfilename) == elecfilename) {
      elecfilename = File::FullPath(dir, elecfilename);
    }

    APtr<CSVFile> elecs = new CSVFile();
    elecs->Load(elecfilename);
    elec_config = elecs.ExtractConst();
    if (elec_config->data.size1() < 3) {
      elec_config.Delete();
      Throw_RC_Type(Note, "Montage CSV file has insufficient columns.");
    }

    uint32_t chan_count;
    sys_config->Get(chan_count, "channel_count");

    RC::Data1D<EEGChan> new_chans(elec_config->data.size2());
    for (size_t r=0; r<elec_config->data.size2(); r++) {
      uint32_t chan = elec_config->data[r][1].Get_u32() - 1; // Subtract 1 to convert to 0-indexing
      RC::RStr label = elec_config->data[r][0];
      if (chan >= chan_count) {
        Throw_RC_Type(File, ("Electrode channel (" + RC::RStr(chan+1) + ") "
              "in Montage CSV (item " + RC::RStr(r+1) +
              ") is greater than " + RC::RStr(chan_count)).c_str());
      }
      new_chans[r] = EEGChan(static_cast<uint16_t>(chan), chan, label);
    }

    return new_chans;
  }

  bool Settings::BipolarElecConfigUsed() {
    RStr elecfilename;
    return exp_config->TryGet(elecfilename, "bipolar_electrode_config_file");
  }

  RC::Data1D<EEGChan> Settings::LoadBipolarElecConfig(RC::RStr dir, RC::Data1D<EEGChan> mono_chans) {
    RStr elecfilename =  exp_config->GetPath("bipolar_electrode_config_file");
    if (File::Basename(elecfilename) == elecfilename) {
      elecfilename = File::FullPath(dir, elecfilename);
    }

    APtr<CSVFile> elecs = new CSVFile();
    elecs->Load(elecfilename);
    bipolar_config = elecs.ExtractConst();
    if (bipolar_config->data.size1() < 3) {
      bipolar_config.Delete();
      Throw_RC_Type(Note, "Bipolar CSV file has insufficient columns.");
    }

    uint32_t chan_count;
    sys_config->Get(chan_count, "channel_count");

    RC::Data1D<EEGChan> new_chans(bipolar_config->data.size2());
    for (size_t r=0; r<bipolar_config->data.size2(); r++) {
      auto& chan = bipolar_config->data[r];
      RC::RStr label = chan[0];
      RC::RStr pos_str = chan[1];
      RC::RStr neg_str = chan[2];

      // Convert from string to u32
      if (! pos_str.Is_u32(10, true)) {
        Throw_RC_Type(File, ("Positive electrode (" + pos_str + ") of bipolar pair (" + RC::RStr(label) + ") "
              "in Bipolar CSV (item " + r + ") is not a valid u32").c_str());
      }

      if (! neg_str.Is_u32(10, true)) {
        Throw_RC_Type(File, ("Negative electrode (" + neg_str + ") of bipolar pair (" + RC::RStr(label) + ") "
              "in Bipolar CSV (item " + r + ") is not a valid u32").c_str());
      }

      uint32_t pos = pos_str.Get_u32() - 1; // Subract 1 to convert to 0-indexing
      uint32_t neg = neg_str.Get_u32() - 1; // Subract 1 to convert to 0-indexing

      // Validate values within channel limits.
      if (pos >= chan_count) {
        Throw_RC_Type(File, ("Positive channel (" + RC::RStr(pos+1) +
              ") of biopolar pair (" + RC::RStr(label) +
              ") in Bipolar CSV (item " + RC::RStr(r+1) +
              ") is greater than " + RC::RStr(chan_count)).c_str());
      }

      if (neg >= chan_count) {
        Throw_RC_Type(File, ("Negative channel (" + RC::RStr(pos+1) +
              ") of biopolar pair (" + RC::RStr(label) +
              ") in Bipolar CSV (item " + RC::RStr(r+1) +
              ") is greater than " + RC::RStr(chan_count)).c_str());
      }

      // Validate that bipolar electrodes are present in mono electrode config
      auto check_pos = [&](const EEGChan& chan) { return chan.GetMonoChannel() == pos; };
      if (std::none_of(mono_chans.begin(), mono_chans.end(), check_pos)) {
        Throw_RC_Type(File, ("Positive electrode (" + RC::RStr(pos) + ") of bipolar pair (" + RC::RStr(label) + ") "
              "in Bipolar CSV (item " + r + ") is not in the Mono CSV").c_str());
      }

      auto check_neg = [&](const EEGChan& chan) { return chan.GetMonoChannel() == neg; };
      if (std::none_of(mono_chans.begin(), mono_chans.end(), check_neg)) {
        Throw_RC_Type(File, ("Negative electrode (" + RC::RStr(neg) + ") of bipolar pair (" + RC::RStr(label) + ") "
              "in Bipolar CSV (item " + r + ") is not in the Mono CSV").c_str());
      }

      // Construct EEG channel
      new_chans[r] = EEGChan(static_cast<uint16_t>(pos), static_cast<uint16_t>(neg), r, label);
    }

    return new_chans;
  }

  void Settings::LoadChannelSettings() {
    auto stim_channels = exp_config->Node("experiment",
        "stim_channels");
    stimconf.Resize(stim_channels.size());
    min_stimconf.Resize(stim_channels.size());
    max_stimconf.Resize(stim_channels.size());
    min_stimconf_range.Resize(stim_channels.size());
    max_stimconf_range.Resize(stim_channels.size());
    stimgrid_chan_on.Resize(stim_channels.size());
    stimgrid_chan_on.Zero();

    float f;
    std::vector<float> vf;
    std::vector<uint32_t> vi;

    for (size_t c=0; c<stim_channels.size(); c++) {
      std::string label;
      std::vector<u8> elecs;
      try {
        stim_channels[c].Get(label, "electrodes");
        auto split = RStr(label).Split('_');
        if (split.size() != 2 || split[0] == split[1]) {
          Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
              " string must be in LA1_LA2 bipolar format").c_str());
        }
        elecs.resize(2);
        size_t found = 0;
        for (size_t i=0; i<elec_config->data.size2(); i++) {
          if (elec_config->data[i][0] == split[0]) {
            elecs[0] = u8(elec_config->data[i][1].Get_u32());
            found++;
          }
          if (elec_config->data[i][0] == split[1]) {
            elecs[1] = u8(elec_config->data[i][1].Get_u32());
            found++;
          }
        }
        if (found != 2) {
          Throw_RC_Type(File, (RStr("Expected 2 channels in CSV for ")
                + label + ", but found " + RStr(found)).c_str());
        }
      }
      catch (ErrorMsg& e) {
        try {
          stim_channels[c].Get(elecs, "electrodes");
          if (elecs.size() != 2) {
            Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
                  " specifies wrong number of electrodes").c_str());
          }
        }
        catch (ErrorMsg&) {
          throw (e);  // From outer catch
        }
      }

      if (elecs[0] == elecs[1]) {
        Throw_RC_Type(File, ("Invalid bipolar pair ("+RStr(elecs[0])+", "+
            RStr(elecs[1])+")").c_str());
      }

      stimconf[c].label = label;
      stimconf[c].params.electrode_pos = elecs[0];
      stimconf[c].params.electrode_neg = elecs[1];

      // burst settings
      float burst_frac = 1;
      uint32_t burst_slow_freq = 0;
      if (!stim_channels[c].TryGet(burst_frac, "burst_fraction")) {
        uint32_t throwaway = 0;
        if (stim_channels[c].TryGet(throwaway, "burst_slow_freq_Hz")) {
          Throw_RC_Type(File, "In experiment stim_channels config, "
              "burst_slow_freq_Hz cannot be processed if "
              "burst_fraction is not set.");
        }
      }

      if (burst_frac != 1) {
        stim_channels[c].Get(burst_slow_freq, "burst_slow_freq_Hz");
        if (burst_slow_freq == 0) {
          Throw_RC_Type(File, "In experiment stim_channels config, if "
              "burst_fraction is set and not equal to 1, "
              "burst_slow_freq_Hz must be set and non-zero.");
        }
      }

      stimconf[c].params.burst_frac = burst_frac;
      stimconf[c].params.burst_slow_freq = burst_slow_freq;


      RC::RStr stimtag;
      stim_channels[c].TryGet(stimtag, "stimtag");
      stimconf[c].stimtag = stimtag;


      // Get the electrode area from the csv file.  Min of the pairs.
      float area_mmsq = std::numeric_limits<float>::max();
      size_t found_pos = 0;
      size_t found_neg = 0;
      for (size_t i=0; i<elec_config->data.size2(); i++) {
        uint32_t check_elec = elec_config->data[i][1].Get_u32();
        float check_area = elec_config->data[i][2].Get_f32();
        if (check_elec == stimconf[c].params.electrode_pos) {
          area_mmsq = std::min(area_mmsq, check_area);
          found_pos++;
        }
        else if (check_elec == stimconf[c].params.electrode_neg) {
          area_mmsq = std::min(area_mmsq, check_area);
          found_neg++;
        }
      }

      if ((found_pos != 1) || (found_neg != 1)) {
        Throw_RC_Type(File, ("Could not look up montage area info for pair ("
            + RStr(elecs[0]) + ", " + RStr(elecs[1]) + ")").c_str());
      }

      stimconf[c].params.area = area_mmsq;

      min_stimconf[c] = stimconf[c];
      max_stimconf[c] = stimconf[c];

      stim_channels[c].Get(vf, "amplitude_range_mA");
      if (vf.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " amplitude_range_mA needs 2 values").c_str());
      }

      min_stimconf[c].params.amplitude = uint16_t(vf[0]*1000+0.5f);
      max_stimconf[c].params.amplitude = uint16_t(vf[1]*1000+0.5f);

      stim_channels[c].Get(vi, "frequency_range_Hz");
      if (vi.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " frequency_range_Hz needs 2 values").c_str());
      }
      min_stimconf[c].params.frequency = vi[0];
      max_stimconf[c].params.frequency = vi[1];

      stim_channels[c].Get(vi, "duration_range_ms");
      if (vi.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " duration_range_ms needs 2 values").c_str());
      }
      min_stimconf[c].params.duration = vi[0] * 1000;
      max_stimconf[c].params.duration = vi[1] * 1000;

      // The set values aren't present for some experiments.
      // Fall back on safest value in range.
      try {
        stim_channels[c].Get(f, "amplitude_mA");
        stimconf[c].params.amplitude = uint16_t(f*1000+0.5f);
      }
      catch (ErrorMsgFile&) {
        stimconf[c].params.amplitude =
          min_stimconf[c].params.amplitude;
      }
      try {
        stim_channels[c].Get(f, "amplitude_search_range_mA", 0);
        min_stimconf_range[c].params.amplitude = uint16_t(f*1000+0.5f);
        stim_channels[c].Get(f, "amplitude_search_range_mA", 1);
        max_stimconf_range[c].params.amplitude = uint16_t(f*1000+0.5f);
        // use stimconf for setting stimulation upper bound for now
        stimconf[c].params.amplitude =
          max_stimconf_range[c].params.amplitude;
      }
      catch (ErrorMsgFile&) {
        min_stimconf_range[c].params.amplitude =
          min_stimconf[c].params.amplitude;
        max_stimconf_range[c].params.amplitude =
          min_stimconf[c].params.amplitude;
      }
      try {
        stim_channels[c].Get(stimconf[c].params.frequency,
            "frequency_Hz");
      }
      catch (ErrorMsgFile&) {
        stimconf[c].params.frequency =
          min_stimconf[c].params.frequency;
      }
      try {
        stim_channels[c].Get(stimconf[c].params.frequency,
            "frequency_Hz");
      }
      catch (ErrorMsgFile&) {
        stimconf[c].params.frequency =
          min_stimconf[c].params.frequency;
      }
      try {
        stim_channels[c].Get(stimconf[c].params.duration,
            "duration_ms");
        stimconf[c].params.duration *= 1000;
      }
      catch (ErrorMsgFile&) {
        stimconf[c].params.duration =
          min_stimconf[c].params.duration;
      }

      // for parameter search, set all fixed parameters to stimconf values
      min_stimconf_range[c].label = stimconf[c].label;
      max_stimconf_range[c].label = stimconf[c].label;
      min_stimconf_range[c].params.electrode_pos = stimconf[c].params.electrode_pos;
      max_stimconf_range[c].params.electrode_pos = stimconf[c].params.electrode_pos;
      min_stimconf_range[c].params.electrode_neg = stimconf[c].params.electrode_neg;
      max_stimconf_range[c].params.electrode_neg = stimconf[c].params.electrode_neg;
      min_stimconf_range[c].params.frequency = stimconf[c].params.frequency;
      max_stimconf_range[c].params.frequency = stimconf[c].params.frequency;
      min_stimconf_range[c].params.duration = stimconf[c].params.duration;
      max_stimconf_range[c].params.duration = stimconf[c].params.duration;
      min_stimconf_range[c].params.area = stimconf[c].params.area;
      max_stimconf_range[c].params.area = stimconf[c].params.area;
      min_stimconf_range[c].params.burst_frac = stimconf[c].params.burst_frac;
      max_stimconf_range[c].params.burst_frac = stimconf[c].params.burst_frac;
      min_stimconf_range[c].params.burst_slow_freq = stimconf[c].params.burst_slow_freq;
      max_stimconf_range[c].params.burst_slow_freq = stimconf[c].params.burst_slow_freq;

      stimconf[c].approved = false;
      min_stimconf_range[c].approved = false;
      max_stimconf_range[c].approved = false;

//      cout << "min_stimconf" << endl;
//      cout << min_stimconf[0].params.amplitude << endl;
//      cout << min_stimconf[0].params.frequency << endl;
//      cout << min_stimconf[0].params.duration << endl;

//      cout << "max_stimconf" << endl;
//      cout << max_stimconf[0].params.amplitude << endl;
//      cout << max_stimconf[0].params.frequency << endl;
//      cout << max_stimconf[0].params.duration << endl;

//      cout << "stimconf" << endl;
//      cout << stimconf[0].params.amplitude << endl;
//      cout << stimconf[0].params.frequency << endl;
//      cout << stimconf[0].params.duration << endl;

//      cout << "min_stimconf_range" << endl;
//      cout << min_stimconf_range[0].params.amplitude << endl;
//      cout << min_stimconf_range[0].params.frequency << endl;
//      cout << min_stimconf_range[0].params.duration << endl;

//      cout << "max_stimconf_range" << endl;
//      cout << max_stimconf_range[0].params.amplitude << endl;
//      cout << max_stimconf_range[0].params.frequency << endl;
//      cout << max_stimconf_range[0].params.duration << endl;

      if ( ! (
        (min_stimconf[c].params.amplitude <=
         Betw(stimconf[c].params.amplitude) <=
         max_stimconf[c].params.amplitude) &&
        (min_stimconf[c].params.amplitude <=
         Betw(min_stimconf_range[c].params.amplitude) <=
         max_stimconf[c].params.amplitude) &&
        (min_stimconf[c].params.amplitude <=
         Betw(max_stimconf_range[c].params.amplitude) <=
         max_stimconf[c].params.amplitude) &&
        (min_stimconf[c].params.frequency <=
         Betw(stimconf[c].params.frequency) <=
         max_stimconf[c].params.frequency) &&
        (min_stimconf[c].params.frequency <=
         Betw(min_stimconf_range[c].params.frequency) <=
         max_stimconf[c].params.frequency) &&
        (min_stimconf[c].params.frequency <=
         Betw(max_stimconf_range[c].params.frequency) <=
         max_stimconf[c].params.frequency) &&
        (min_stimconf[c].params.duration <=
         Betw(stimconf[c].params.duration) <=
         max_stimconf[c].params.duration) &&
        (min_stimconf[c].params.duration <=
         Betw(min_stimconf_range[c].params.duration) <=
         max_stimconf[c].params.duration) &&
        (min_stimconf[c].params.duration <=
         Betw(max_stimconf_range[c].params.duration) <=
         max_stimconf[c].params.duration))) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " default stim value outside of allowed range").c_str());
      }
    }
  }


  template<class D, class R>
  void ValidateRange(D data, R min, R max, size_t c, RC::RStr errtype) {
    for (size_t i=0; i<data.size(); i++) {
      if ((data[i] < min) || (data[i] > max)) {
        Throw_RC_Type(File, (RC::RStr("Grid ") + errtype + " #" +
            RC::RStr(i) + " outside of valid range for config channel entry #"
            + RC::RStr(c)).c_str());
      }
    }
  }


  void Settings::LoadStimParamGrid() {
    // stim_parameter ranges for grid search, if they exist.

    std::vector<float> vf;
    std::vector<uint32_t> vi;

    // Grab grid params from config.  Exception if missing.
    exp_config->Get(vf, "experiment", "stim_parameters",
        "amplitudes_mA");
    stimgrid_amp_uA.Resize(vf.size());
    for (size_t i=0; i<vf.size(); i++) {
      stimgrid_amp_uA[i] = uint16_t(vf[i]*1000+0.5);
    }
    stimgrid_amp_on.Resize(stimgrid_amp_uA.size());
    stimgrid_amp_on.Zero();

    exp_config->Get(stimgrid_freq_Hz, "experiment", "stim_parameters",
        "frequencies_Hz");
    stimgrid_freq_on.Resize(stimgrid_freq_Hz.size());
    stimgrid_freq_on.Zero();

    exp_config->Get(vi, "experiment", "stim_parameters",
        "durations_ms");
    stimgrid_dur_us.Resize(vi.size());
    for (size_t i=0; i<vi.size(); i++) {
      stimgrid_dur_us[i] = uint32_t(vi[i]*1000+0.5);
    }
    stimgrid_dur_on.Resize(stimgrid_dur_us.size());
    stimgrid_dur_on.Zero();

    // Validate values within min/max.
    size_t chcnt = min_stimconf.size();
    for (size_t c=0; c<chcnt; c++) {
      ValidateRange(stimgrid_amp_uA,
          min_stimconf[c].params.amplitude,
          max_stimconf[c].params.amplitude,
          c, "amplitude");
      ValidateRange(stimgrid_freq_Hz,
          min_stimconf[c].params.frequency,
          max_stimconf[c].params.frequency,
          c, "frequency");
      ValidateRange(stimgrid_dur_us,
          min_stimconf[c].params.duration,
          max_stimconf[c].params.duration,
          c, "duration");
    }

    exp_config->Get(ops_specs.num_stim_trials, "experiment",
        "experiment_specs", "num_stim_trials");
    exp_config->Get(ops_specs.num_sham_trials, "experiment",
        "experiment_specs", "num_sham_trials");
    exp_config->Get(ops_specs.intertrial_range_ms, "experiment",
        "experiment_specs", "intertrial_range_ms");
    exp_config->Get(ops_specs.sham_duration_ms, "experiment",
        "experiment_specs", "sham_duration_ms");
  }


  void Settings::LoadStimParamsCPS() {
    exp_config->Get(cps_specs.intertrial_range_ms, "experiment",
        "experiment_specs", "intertrial_range_ms");

    exp_config->Get(cps_specs.sham_duration_ms, "experiment",
        "experiment_specs", "sham_duration_ms");

    exp_config->Get(cps_specs.n_normalize_events, "experiment",
        "experiment_specs", "n_normalize_events");

    exp_config->Get(cps_specs.classify_ms, "experiment",
        "experiment_specs", "classify_ms");

    exp_config->Get(cps_specs.poststim_biomarker_lockout_ms, "experiment",
        "experiment_specs", "poststim_biomarker_lockout_ms");

    exp_config->Get(cps_specs.obsNoise, "experiment",
        "experiment_specs", "Bayesian_optimization", "observation_noise");

    exp_config->Get(cps_specs.exp_bias, "experiment",
        "experiment_specs", "Bayesian_optimization", "exploration_bias");

    exp_config->Get(cps_specs.n_init_samples, "experiment",
        "experiment_specs", "Bayesian_optimization", "n_initial_samples");

    exp_config->Get(cps_specs.kern_lengthscale_lb, "experiment",
        "experiment_specs", "Bayesian_optimization", "kernel_lengthscale_lowerbound");

    exp_config->Get(cps_specs.kern_lengthscale_ub, "experiment",
        "experiment_specs", "Bayesian_optimization", "kernel_lengthscale_upperbound");

    if (cps_specs.kern_lengthscale_ub < cps_specs.kern_lengthscale_lb) {
      Throw_RC_Type(File, (RC::RStr("ERROR: Kernel lengthscale upper bound below") +
                           "lower bound in experiment config. Please fix.").c_str());
    }

    exp_config->Get(cps_specs.kern_var_lb, "experiment",
        "experiment_specs", "Bayesian_optimization", "kernel_variance_lowerbound");

    exp_config->Get(cps_specs.kern_var_ub, "experiment",
        "experiment_specs", "Bayesian_optimization", "kernel_variance_upperbound");

    if (cps_specs.kern_var_ub < cps_specs.kern_var_lb) {
      Throw_RC_Type(File, (RC::RStr("ERROR: Kernel variance upper bound below") +
                           "lower bound in experiment config. Please fix.").c_str());
    }

    exp_config->Get(cps_specs.kern_white_lb, "experiment",
        "experiment_specs", "Bayesian_optimization", "kernel_white_variance_lowerbound");

    exp_config->Get(cps_specs.kern_white_ub, "experiment",
        "experiment_specs", "Bayesian_optimization", "kernel_white_variance_upperbound");

    if (cps_specs.kern_white_ub < cps_specs.kern_white_lb) {
      Throw_RC_Type(File, (RC::RStr("ERROR: Kernel white variance upper bound below") +
                           "lower bound in experiment config. Please fix.").c_str());
    }
  }


  void Settings::UpdateConfFR(JSONFile& current_config) {
    for (size_t c=0; c<stimconf.size(); c++) {
      current_config.Set(stimconf[c].params.amplitude*1e-3,
          "experiment", "stim_channels", c, "amplitude_mA");
      current_config.Set(stimconf[c].params.frequency,
          "experiment", "stim_channels", c, "frequency_Hz");
      current_config.Set(uint32_t((stimconf[c].params.duration+500)/1000),
          "experiment", "stim_channels", c, "duration_ms");
    }
  }


  void Settings::UpdateConfOPS(JSONFile& current_config) {
    RC::Data1D<RC::RStr> sel_chans;
    for (size_t i=0; i<stimconf.size(); i++) {
      if (stimgrid_chan_on[i]) {
        sel_chans += RC::RStr("[") + stimconf[i].params.electrode_pos + ", " +
          stimconf[i].params.electrode_neg + "]";
      }
    }
    RC::RStr sel_chan_str = RC::RStr("[") + RC::RStr::Join(sel_chans, ", ")
      + "]";
    JSONFile sel_chan_json;
    sel_chan_json.Parse(sel_chan_str);
    current_config.Set(sel_chan_json, "experiment", "stim_parameters",
        "channels");

    RC::Data1D<float> sel_amp;
    for (size_t i=0; i<stimgrid_amp_uA.size(); i++) {
      if (stimgrid_amp_on[i]) {
        sel_amp.Append(stimgrid_amp_uA[i]/1000.0f);
      }
    }
    current_config.Set(sel_amp, "experiment", "stim_parameters",
        "amplitudes_mA");

    RC::Data1D<uint32_t> sel_freq;
    for (size_t i=0; i<stimgrid_freq_Hz.size(); i++) {
      if (stimgrid_freq_on[i]) {
        sel_freq.Append(stimgrid_freq_Hz[i]);
      }
    }
    current_config.Set(sel_freq, "experiment", "stim_parameters",
        "frequencies_Hz");

    RC::Data1D<uint32_t> sel_dur;
    for (size_t i=0; i<stimgrid_dur_us.size(); i++) {
      if (stimgrid_dur_on[i]) {
        sel_dur.Append(stimgrid_dur_us[i]/1000.0f+0.5f);
      }
    }
    current_config.Set(sel_dur, "experiment", "stim_parameters",
        "durations_ms");
  }


  RC::Data1D<RC::RStr> Settings::UpdateConfCPS(JSONFile& current_config,
                               const RC::RStr& elemem_path,
                               const RC::RStr& session_path,
                               const RC::RStr& current_elec_config_filename) {
    for (size_t c=0; c<max_stimconf_range.size(); c++) {
      // configuration ranges for parameters to search over
      current_config.Set(min_stimconf[c].params.amplitude*1e-3,
          "experiment", "stim_channels", c, "amplitude_range_mA", 0);
      current_config.Set(max_stimconf[c].params.amplitude*1e-3,
          "experiment", "stim_channels", c, "amplitude_range_mA", 1);

      // fixed parameters
      current_config.Set(min_stimconf_range[c].params.amplitude*1e-3,
          "experiment", "stim_channels", c, "amplitude_search_range_mA", 0);
      current_config.Set(max_stimconf_range[c].params.amplitude*1e-3,
          "experiment", "stim_channels", c, "amplitude_search_range_mA", 1);

      current_config.Set(min_stimconf_range[c].params.frequency,
          "experiment", "stim_channels", c, "frequency_search_range_Hz", 0);
      current_config.Set(max_stimconf_range[c].params.frequency,
          "experiment", "stim_channels", c, "frequency_search_range_Hz", 1);

      current_config.Set(uint32_t((min_stimconf_range[c].params.duration+500)/1000),
          "experiment", "stim_channels", c, "duration_search_range_ms", 0);
      current_config.Set(uint32_t((max_stimconf_range[c].params.duration+500)/1000),
          "experiment", "stim_channels", c, "duration_search_range_ms", 1);
    }

    // log which stim parameters are being optimized
    current_config.Set(nlohmann::json::array(), "experiment", "optimized_stim_parameters");
    current_config.json["experiment"]["optimized_stim_parameters"].push_back(std::string("amplitude"));
    current_config.json["experiment"]["optimized_stim_parameters"].push_back(std::string("location"));
    // whether session should be excluded from being loaded in subsequent CPS sessions (due to
    // e.g., bad electrodes/incorrect config settings).
    current_config.Set(false, "exclude_session");

    // get previous session event logs for loading search events

    Data1D<RStr> elemem_subpaths = RC::File::DirList(elemem_path, true);
    Data1D<RStr> prev_sess_paths;

    // get localization/montage for current subject from electrode config filename
    RC::RStr elec_file_no_ext(File::NoExtension(File::Basename(current_elec_config_filename)));
    RC::RStr loc_mont(elec_file_no_ext.substr(elec_file_no_ext.find_last_of(RC::RStr("_")) + 1));

    current_config.Set(nlohmann::json::array(), "experiment", "previous_sessions");
    size_t n_prev_sess = 0;
    for (size_t i = 0; i < elemem_subpaths.size(); i++) {
      RStr sess_dir = File::Basename(elemem_subpaths[i]);
      if (elemem_subpaths[i] == session_path) { continue; }  // don't include current session
      size_t p = sess_dir.find(RC::RStr("_"));

      if (p < sess_dir.size() && sess_dir.substr(0, p) == sub) {  // only include session directories from same subject
        RC::RStr prev_sess_conf = File::FullPath(elemem_subpaths[i], "experiment_config.json");
        RC::FileRead fr;
        // TODO: RDD: remove this warning if a subject session doesn't load? Session may not be for current experiment
        if (!fr.Open(prev_sess_conf)) {  // check that session directory has experiment config file
          RC::RStr message = RC::RStr("CPS: Previous session at\n") + prev_sess_conf
                                      + RC::RStr("\nfor subject could not be loaded.");
          DEBLOG_OUT(message);
          // TODO: RDD: is there an error catching mechanism for Throw_RC_Error? It looks like elsewhere only Throw_RC_Error is called?
          if (!ConfirmWin(RC::RStr("WARNING!  ") + message + RC::RStr("  Continue?"))) {
            Throw_RC_Error(message.c_str());
          }
          continue;
        }
        JSONFile sess_conf(prev_sess_conf);
        RC::RStr sess_exper = RC::RStr(sess_conf.json.at("experiment").at("type").dump());

        if (sess_exper.find("CPS") < sess_exper.size()) {
          // exclude bad sessions
          bool exc_sess = sess_conf.json["exclude_session"];
          if (exc_sess) { continue; }

          // filter sessions for matching montage/localization
          // TODO: RDD: currently just comparing file suffix of electrode config files; should probably just compare config files directly
          RC::RStr prev_elec_file_no_ext(File::NoExtension(sess_conf.json.at("electrode_config_file").dump()));
          RC::RStr prev_loc_mont(prev_elec_file_no_ext.substr(prev_elec_file_no_ext.find_last_of(RC::RStr("_")) + 1));
          if (prev_loc_mont.find(loc_mont) != 0) { continue; }

          // add previous session info
          current_config.json["experiment"]["previous_sessions"].push_back(nlohmann::json({}));
          current_config.Set(elemem_subpaths[i].Raw(), "experiment", "previous_sessions", n_prev_sess, "path");
          prev_sess_paths += elemem_subpaths[i];

          // TODO: RDD/JPB: need to get video paths for each experiment and then
          //                load them up here to ensure videos are watched in correct order
//          current_config.Set(elemem_subpaths[i], "experiment", "previous_sessions", n_prev_sess, "video");
          n_prev_sess++;
        }
      }
    }
    return prev_sess_paths;
  }


  size_t SumBool(const RC::Data1D<bool> &data) {
    size_t sum = 0;
    for (size_t i=0; i<data.size(); i++) {
      sum += size_t(data[i]);
    }
    return sum;
  }


  size_t Settings::GridSize() const {
    return SumBool(stimgrid_chan_on) * SumBool(stimgrid_amp_on) *
      SumBool(stimgrid_freq_on) * SumBool(stimgrid_dur_on);
  }
}

