#include "ConfigFile.h"
#include "Handler.h"
#include "MainWindow.h"
#include "Popup.h"
#include "Utils.h"
#include "RC/RC.h"
#include <QDir>

using namespace std;
using namespace RC;


namespace CML {
  Handler::Handler()
    : stim_worker(this),
      edf_save(this),
      net_worker(this),
      elemem_dir(File::FullPath(GetDesktop(), "ElememData")) {

    File::MakeDir(elemem_dir);
  }

  // Defaulting this here after ConfigFile is included ensures
  // proper deletion of the forward delcared APtr elements.
  Handler::~Handler() = default;

  void Handler::SetMainWindow(Ptr<MainWindow> new_main) {
    main_window = new_main;
  }

  void Handler::CerebusTest_Handler() {
    eeg_acq.CloseCerebus();

    APITests::CereLinkTest();
  }

  void Handler::CereStimTest_Handler() {
    if (stim_api_test_warning) {
      if (!ConfirmWin("WARNING!  CereStim Test should not be run with a participant connected.  Continue?")) {
        return;
      }

      stim_api_test_warning = false;
    }

    stim_worker.CloseCereStim();

    APITests::CereStimTest();
  }

  void Handler::SetStimSettings_Handler(const size_t& index,
      const StimSettings& updated_settings) {
    if (index >= stim_settings.size()) {
      return;
    }

    // No.  Put that back.
    if (experiment_running) {
      main_window->GetStimConfigBox(index).SetParameters(
          stim_settings[index].params);
      return;
    }

    // Safety checks.
    // In principle this is redundant with the set gui limits, but this
    // should guard against programming errors now and under future
    // maintenance.
    if (
      (updated_settings.params.electrode_pos !=
       stim_settings[index].params.electrode_pos) ||
      (updated_settings.params.electrode_neg !=
       stim_settings[index].params.electrode_neg) ||
      (updated_settings.params.amplitude >
       max_stim_settings[index].params.amplitude) ||
      (updated_settings.params.amplitude <
       min_stim_settings[index].params.amplitude) ||
      (updated_settings.params.frequency >
       max_stim_settings[index].params.frequency) ||
      (updated_settings.params.frequency <
       min_stim_settings[index].params.frequency) ||
      (updated_settings.params.duration >
       max_stim_settings[index].params.duration) ||
      (updated_settings.params.duration <
       min_stim_settings[index].params.duration)) {
      main_window->GetStimConfigBox(index).SetParameters(
        stim_settings[index].params);
      ErrorWin("Attempted to set invalid stim settings.", "Safety check");
      return;
    }

    stim_settings[index] = updated_settings;
  }

  void Handler::TestStim_Handler(const size_t& index) {
    if (index >= stim_settings.size()) {
      ErrorWin("Stim channel not configured.");
      return;
    }

    if (experiment_running) {
      ErrorWin("Cannot test stim while experiment running.");
      return;
    }

    CSStimProfile profile;
    profile += stim_settings[index].params;

    stim_worker.ConfigureStimulation(profile);
    stim_worker.Stimulate();
  }

  void Handler::StartExperiment_Handler() {
    std::string stim_mode_str;
    exp_config->Get(stim_mode_str, "experiment", "stim_mode");
    RStr stim_mode = stim_mode_str;
    stim_mode.ToLower();
    if (stim_mode != "open" && stim_mode != "none") {
      ErrorWin("Configuration file experiment:stim_mode must be \"open\" "
          "for this version of Elemem.  Cannot start experiment.");
      return;
    }

    CSStimProfile profile;
    for (size_t c=0; c<stim_settings.size(); c++) {
      if (stim_settings[c].approved) {
        profile += stim_settings[c].params;
      }
    }
    if (profile.size() == 0 && stim_mode != "none") {
      ConfirmWin("No stim channels approved on experiment configured "
          "with stimulation.  Proceed?");
    }
    stim_worker.ConfigureStimulation(profile);

    session_dir = File::FullPath(elemem_dir, "Elemem_"+Time::GetDateTime());
    File::MakeDir(session_dir);

    // Save updated experiment configuration.
    JSONFile current_config = *exp_config;
    for (size_t c=0; c<stim_settings.size(); c++) {
      current_config.Set(stim_settings[c].params.amplitude,
          "experiment", "stim_channels", c, "amplitude_mA");
      current_config.Set(stim_settings[c].params.frequency,
          "experiment", "stim_channels", c, "frequency_Hz");
      current_config.Set(stim_settings[c].params.duration,
          "experiment", "stim_channels", c, "duration_ms");
    }
    current_config.Save(File::FullPath(session_dir,
          "experiment_config.json"));

    // Save copy of loaded electrode config.
    FileWrite fw(File::FullPath(session_dir,
          File::Basename(elec_config->GetFilename())));
    fw.Put(elec_config->file_lines, true);
    fw.Close();

    event_log.StartFile(File::FullPath(session_dir, "event.log"));

    // Start acqusition
    edf_save.StartFile(File::FullPath(session_dir,
          "eeg_data.edf"));

    // Defaults should always work on standard setup.
    RStr ipaddress = "192.168.137.1";
    uint16_t port = 8889
    try {
      exp_config->Get(ipaddress, "ipaddress");
    }
    catch (...) { }
    try {
      exp_config->Get(port, "port");
    }
    catch (...) { }

    net_worker.Listen(ipaddress, port);
  }

  void Handler::StopExperiment_Handler() {
    edf_save.StopSaving();
    SaveDefaultEEG();

    net_worker.Close();

    event_log.Close();
  }

  void Handler::OpenConfig_Handler(RC::FileRead& fr) {
    for (size_t c=0; c<main_window->StimConfigCount(); c++) {
      main_window->GetStimConfigBox(c).Clear();
    }

    APtr<JSONFile> conf = new JSONFile();
    conf->Load(fr);
    exp_config = conf.ExtractConst();

    std::string elecfilename_str;
    exp_config->Get(elecfilename_str, "electrode_config_file");
    RStr elecfilename;
    if (File::Basename(elecfilename_str) == elecfilename_str) {
      elecfilename = File::FullPath(File::Dirname(fr.GetFilename()),
          elecfilename_str);
    }
    else {
      elecfilename = elecfilename_str;
    }

    APtr<CSVFile> elecs = new CSVFile();
    elecs->Load(elecfilename);
    elec_config = elecs.ExtractConst();

    auto stim_channels = exp_config->Node("experiment", "stim_channels");
    stim_settings.Resize(stim_channels.size());
    min_stim_settings.Resize(stim_channels.size());
    max_stim_settings.Resize(stim_channels.size());
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

      stim_settings[c].label = label;
      stim_settings[c].params.electrode_pos = elecs[0]-1;
      stim_settings[c].params.electrode_neg = elecs[1]-1;
      float f;
      stim_channels[c].Get(f, "amplitude_mA");
      stim_settings[c].params.amplitude = uint16_t(f*1000+0.5f);
      stim_channels[c].Get(stim_settings[c].params.frequency, "frequency_Hz");
      stim_channels[c].Get(stim_settings[c].params.duration, "duration_ms");
      stim_settings[c].params.duration *= 1000;
      stim_settings[c].approved = false;

      min_stim_settings = stim_settings;
      max_stim_settings = stim_settings;

      std::vector<float> vf;
      std::vector<uint32_t> vi;

      stim_channels[c].Get(vf, "amplitude_range_mA");
      if (vf.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " amplitude_range_mA needs 2 values").c_str());
      }

      min_stim_settings[c].params.amplitude = uint16_t(vf[0]*1000+0.5f);
      max_stim_settings[c].params.amplitude = uint16_t(vf[1]*1000+0.5f);

      stim_channels[c].Get(vi, "frequency_range_Hz");
      if (vi.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " frequency_range_Hz needs 2 values").c_str());
      }
      min_stim_settings[c].params.frequency = vi[0];
      max_stim_settings[c].params.frequency = vi[1];

      stim_channels[c].Get(vi, "duration_range_ms");
      if (vi.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " duration_range_ms needs 2 values").c_str());
      }
      min_stim_settings[c].params.duration = vi[0] * 1000;
      max_stim_settings[c].params.duration = vi[1] * 1000;

      if ( ! (
        (min_stim_settings[c].params.amplitude <=
         Betw(stim_settings[c].params.amplitude) <=
         max_stim_settings[c].params.amplitude) &&
        (min_stim_settings[c].params.frequency <=
         Betw(stim_settings[c].params.frequency) <=
         max_stim_settings[c].params.frequency) &&
        (min_stim_settings[c].params.duration <=
         Betw(stim_settings[c].params.duration) <=
         max_stim_settings[c].params.duration))) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " default stim value outside of allowed range").c_str());
      }

      if (c < main_window->StimConfigCount()) {
        main_window->GetStimConfigBox(c).SetChannel(min_stim_settings[c].params,
            max_stim_settings[c].params, stim_settings[c].label, c);
        main_window->GetStimConfigBox(c).SetParameters(
            stim_settings[c].params);
      }
    }

    SaveDefaultEEG();
  }

  void Handler::SaveDefaultEEG() {
    std::string sub_name;
    exp_config->Get(sub_name, "subject");

    RStr eeg_file = File::FullPath(elemem_dir,
        RStr(sub_name)+"_nonsession_eeg_"+Time::GetDateTime()+".edf");

    edf_save.StartFile(eeg_file);
  }
}

