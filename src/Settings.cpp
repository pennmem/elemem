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

    sys_config = load_sys_conf.ExtractConst();
  }

  void Settings::Clear() {
    exp_config = nullptr;
    elec_config = nullptr;
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
    Data1D<EEGChan> new_chans(elec_config->data.size2());
    for (size_t r=0; r<elec_config->data.size2(); r++) {
      new_chans[r] = EEGChan(elec_config->data[r][1].Get_u32()-1,
                             elec_config->data[r][0]);
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
    exp_config->Get(cps_specs.experiment_duration_secs, "experiment",
        "experiment_specs", "experiment_duration_secs");
    exp_config->Get(cps_specs.intertrial_range_ms, "experiment",
        "experiment_specs", "intertrial_range_ms");
    exp_config->Get(cps_specs.sham_duration_ms, "experiment",
        "experiment_specs", "sham_duration_ms");
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


  void Settings::UpdateConfCPS(JSONFile& current_config) {
    RC_DEBOUT(RC::RStr("Settings::UpdateConfCPS enter\n"));
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
    RC_DEBOUT(RC::RStr("Settings::UpdateConfCPS exit\n"));
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

