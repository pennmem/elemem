#include "ChannelSelector.h"
#include "ConfigFile.h"
#include "Handler.h"
#ifdef NO_HDF5
#include "EDFSave.h"
#else
#include "HDF5Save.h"
#endif
#include "MainWindow.h"
#include "Popup.h"
#include "Utils.h"
#include "RC/RC.h"
#include <QDir>
#include <QObject>

using namespace std;
using namespace RC;


namespace CML {
  Handler::Handler()
    : stim_worker(this),
#ifdef NO_HDF5
      eeg_save(new EDFSave(this)),
#else
      eeg_save(new HDF5Save(this)),
#endif
      net_worker(this),
      elemem_dir(File::FullPath(GetDesktop(), "ElememData")),
      non_session_dir(File::FullPath(elemem_dir, "NonSessionData")),
      exper_ops(this) {

    File::MakeDir(elemem_dir);
    File::MakeDir(non_session_dir);
  }

  Handler::~Handler() {
    // Worker tasks are already stopped before getting here.
  }

  void Handler::SetMainWindow(Ptr<MainWindow> new_main) {
    main_window = new_main;
    net_worker.SetStatusPanel(main_window->GetStatusPanel());
    stim_worker.SetStatusPanel(main_window->GetStatusPanel());
    exper_ops.SetStatusPanel(main_window->GetStatusPanel());
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
    if (index >= settings.stimconf.size()) {
      return;
    }

    // No.  Put that back.
    if (experiment_running) {
      main_window->GetStimConfigBox(index).SetParameters(
          settings.stimconf[index].params);
      return;
    }

    // Safety checks.
    // In principle this is redundant with the set gui limits, but this
    // should guard against programming errors now and under future
    // maintenance.
    if (
      (updated_settings.params.electrode_pos !=
       settings.stimconf[index].params.electrode_pos) ||
      (updated_settings.params.electrode_neg !=
       settings.stimconf[index].params.electrode_neg) ||
      (updated_settings.params.amplitude >
       settings.max_stimconf[index].params.amplitude) ||
      (updated_settings.params.amplitude <
       settings.min_stimconf[index].params.amplitude) ||
      (updated_settings.params.frequency >
       settings.max_stimconf[index].params.frequency) ||
      (updated_settings.params.frequency <
       settings.min_stimconf[index].params.frequency) ||
      (updated_settings.params.duration >
       settings.max_stimconf[index].params.duration) ||
      (updated_settings.params.duration <
       settings.min_stimconf[index].params.duration)) {
      main_window->GetStimConfigBox(index).SetParameters(
        settings.stimconf[index].params);
      ErrorWin("Attempted to set invalid stim settings.", "Safety check");
      return;
    }

    settings.stimconf[index] = updated_settings;
  }

  void Handler::TestStim_Handler(const size_t& index) {
    if (experiment_running) {
      ErrorWin("Cannot test stim while experiment running.");
      return;
    }

    if (index >= settings.stimconf.size()) {
      ErrorWin("Stim channel not configured.");
      return;
    }

    CSStimProfile profile;
    profile += settings.stimconf[index].params;

    stim_worker.ConfigureStimulation(profile);
    stim_worker.Stimulate();
  }

  void Handler::TestLocStim_Handler() {
    if (experiment_running) {
      ErrorWin("Cannot test stim while experiment running.");
      return;
    }

    if (settings.stimloctest_chanind >= settings.stimconf.size() ||
        settings.stimloctest_amp >= settings.stimgrid_amp_uA.size() ||
        settings.stimloctest_freq >= settings.stimgrid_freq_Hz.size() ||
        settings.stimloctest_dur >= settings.stimgrid_dur_us.size()) {
      ErrorWin("A valid set of parameters must be selected to test "
               "stimulation");
      return;
    }

    CSStimProfile profile;
    CSStimChannel stimchan =
      settings.stimconf[settings.stimloctest_chanind].params;
    stimchan.amplitude = settings.stimgrid_amp_uA[settings.stimloctest_amp];
    stimchan.frequency = settings.stimgrid_freq_Hz[settings.stimloctest_freq];
    stimchan.duration = settings.stimgrid_dur_us[settings.stimloctest_dur];
    profile += stimchan;

    stim_worker.ConfigureStimulation(profile);
    stim_worker.Stimulate();
  }

  void Handler::TestSelLocChan_Handler(const size_t& selected) {
    if (selected < settings.stimconf.size()) {
      settings.stimloctest_chanind = selected;
    }
  }

  void Handler::TestSelLocAmp_Handler(const size_t& selected) {
    if (selected < settings.stimgrid_amp_uA.size()) {
      settings.stimloctest_amp = selected;
    }
  }

  void Handler::TestSelLocFreq_Handler(const size_t& selected) {
    if (selected < settings.stimgrid_freq_Hz.size()) {
      settings.stimloctest_freq = selected;
    }
  }

  void Handler::TestSelLocDur_Handler(const size_t& selected) {
    if (selected < settings.stimgrid_dur_us.size()) {
      settings.stimloctest_dur = selected;
    }
  }

  void Handler::SetLocChansApproved_Handler(const RC::Data1D<bool>& approved) {
    if (approved.size() != settings.stimconf.size()) {
      Throw_RC_Error("Channel approval size assertion failed.");
    }
    settings.stimgrid_chan_on = approved;
  }

  void Handler::SetLocAmpApproved_Handler(const RC::Data1D<bool>& approved) {
    if (approved.size() != settings.stimgrid_amp_uA.size()) {
      Throw_RC_Error("Amplitude approval size assertion failed.");
    }
    settings.stimgrid_amp_on = approved;
  }

  void Handler::SetLocFreqApproved_Handler(const RC::Data1D<bool>& approved) {
    if (approved.size() != settings.stimgrid_freq_Hz.size()) {
      Throw_RC_Error("Frequency approval size assertion failed.");
    }
    settings.stimgrid_freq_on = approved;
  }

  void Handler::SetLocDurApproved_Handler(const RC::Data1D<bool>& approved) {
    if (approved.size() != settings.stimgrid_dur_us.size()) {
      Throw_RC_Error("Duration approval size assertion failed.");
    }
    settings.stimgrid_dur_on = approved;
  }

  void Handler::StartExperiment_Handler() {
    if (settings.exp_config.IsNull() || settings.elec_config.IsNull()) {
      ErrorWin("You must load a valid experiment configuration file before "
               "starting an experiment session.", "Unconfigured");
      return;
    }

    std::string stim_mode_str;
    settings.exp_config->Get(stim_mode_str, "experiment", "stim_mode");
    RStr stim_mode = stim_mode_str;
    stim_mode.ToLower();
    if (stim_mode != "open" && stim_mode != "none") {
      ErrorWin("Configuration file experiment:stim_mode must be \"open\" or "
          "\"none\" for this version of Elemem.  Cannot start experiment.");
      return;
    }

    if (settings.grid_exper) {
      size_t grid_size = settings.GridSize();
      if (grid_size == 0) {
        ErrorWin("Error!  At least one value must be approved for each "
            "stim parameter of a grid search experiment.");

        return;
      }

      Data1D<CSStimProfile> grid_profiles = CreateGridProfiles();
      exper_ops.SetOPSSpecs(settings.ops_specs);
      exper_ops.SetStimProfiles(grid_profiles);
    }
    else {
      CSStimProfile profile;
      for (size_t c=0; c<settings.stimconf.size(); c++) {
        if (settings.stimconf[c].approved) {
          profile += settings.stimconf[c].params;
        }
      }
      if (profile.size() == 0 && stim_mode != "none") {
        if (!ConfirmWin("No stim channels approved on experiment configured "
             "with stimulation.  Proceed?")) {
          return;
        }
      }
      if (profile.size() > 0 && stim_mode == "none") {
        ErrorWin("Error!  Stim channels enabled on experiment with "
                 "experiment:stim_mode set to \"none\".");
        return;
      }

      stim_worker.ConfigureStimulation(profile);
    }

    experiment_running = true;
    main_window->SetReadyToStart(false);
    for (size_t i=0; i<main_window->StimConfigCount(); i++) {
      main_window->GetStimConfigBox(i).SetEnabled(false);
    }

    session_dir = File::FullPath(elemem_dir, RStr(settings.sub) + "_" +
        Time::GetDateTime());
    File::MakeDir(session_dir);

    // Save updated experiment configuration.
    JSONFile current_config = *(settings.exp_config);
    if (settings.exper.find("OPS") == 0) {
      settings.UpdateConfOPS(current_config);
    }
    else {
      settings.UpdateConfFR(current_config);
    }
    current_config.Save(File::FullPath(session_dir,
          "experiment_config.json"));

    // Save copy of loaded electrode config.
    FileWrite fw(File::FullPath(session_dir,
          File::Basename(settings.elec_config->GetFilename())));
    fw.Put(settings.elec_config->file_lines, true);
    fw.Close();

    event_log.StartFile(File::FullPath(session_dir, "event.log"));

    // Start acqusition
    eeg_save->StartFile(File::FullPath(session_dir,
          RC::RStr("eeg_data.") + eeg_save->GetExt()));

    if (settings.grid_exper) {
      exper_ops.Start();
    }
    else { // Network experiment.
      // Defaults should always work on standard setup.
      std::string ipaddress = "192.168.137.1";
      uint16_t port = 8889;
      try {
        settings.exp_config->Get(ipaddress, "ipaddress");
      }
      catch (ErrorMsgFile&) { }
      try {
        settings.exp_config->Get(port, "port");
      }
      catch (ErrorMsgFile&) { }

      net_worker.Listen(ipaddress, port);
      main_window->GetStatusPanel()->SetEvent("WAITING");
    }
  }

  void Handler::StopExperiment_Handler() {
    stim_worker.Abort();
    CloseExperimentComponents();
    stim_worker.Abort();

    SaveDefaultEEG();

    for (size_t i=0; i<main_window->StimConfigCount(); i++) {
      main_window->GetStimConfigBox(i).SetEnabled(true);
    }

    do_exit = false;
    if (experiment_running) {
      experiment_running = false;
      main_window->SetReadyToStart(true);
      main_window->GetStatusPanel()->SetEvent("RECORDING");
    }
  }

  void Handler::ExperimentExit_Handler() {
    if (exit_timer.IsNull()) {
      exit_timer = new QTimer();
      exit_timer->setTimerType(Qt::PreciseTimer);
      exit_timer->setSingleShot(true);
      // Okay because timer allocated within Handler thread here.
      QObject::connect(exit_timer, &QTimer::timeout,
        RC::MakeCaller(this, &Handler::HandleExit));
    }

    do_exit = true;
    exit_timer->setSingleShot(5000);
  }

  void Handler::HandleExit() {
    if (do_exit) {
      StopExperiment_Handler();
    }
  }

  void Handler::OpenConfig_Handler(RC::FileRead& fr) {
    if (experiment_running) {
      ErrorWin("You must stop the experiment before opening a new "
               "configuration");
      return;
    }

    // Clear the gui stim elements.
    for (size_t c=0; c<main_window->StimConfigCount(); c++) {
      main_window->GetStimConfigBox(c).Clear();
    }
    main_window->GetLocConfigChan().Clear();
    main_window->GetLocConfigAmp().Clear();
    main_window->GetLocConfigFreq().Clear();
    main_window->GetLocConfigDur().Clear();

    APtr<JSONFile> conf = new JSONFile();
    conf->Load(fr);
    RStr base_dir = File::Dirname(fr.GetFilename());

    settings.Clear();
    settings.exp_config = conf.ExtractConst();


    Data1D<EEGChan> new_chans;
    try {
      settings.exp_config->Get(settings.sub, "subject");
      settings.exp_config->Get(settings.exper, "experiment", "type");

      if (settings.exper.find("OPS") == 0) {
        settings.grid_exper = true;
        settings.task_driven = false;
      }

      new_chans = settings.LoadElecConfig(base_dir);
      settings.LoadChannelSettings();

      if (settings.grid_exper) {
        settings.LoadStimParamGrid();
      }
    }
    catch (ErrorMsg&) {
      // Something was wrong with this file.  Cannot proceed.
      settings.Clear();
      throw;
    }


    // Setup gui elements.
    main_window->GetStatusPanel()->SetSubject(settings.sub);
    main_window->GetChannelSelector()->SetChannels(new_chans);

    if (settings.exper.find("OPS") == 0) {
      Data1D<RStr> chan_strs(settings.stimconf.size());
      Data1D<RStr> amp_strs(settings.stimgrid_amp_uA.size());
      Data1D<RStr> freq_strs(settings.stimgrid_freq_Hz.size());
      Data1D<RStr> dur_strs(settings.stimgrid_dur_us.size());

      for (size_t i=0; i<chan_strs.size(); i++) {
        chan_strs[i] = RStr(settings.stimconf[i].params.electrode_pos) + "_" +
          RStr(settings.stimconf[i].params.electrode_neg) + " (" +
          settings.stimconf[i].label + ")";
      }
      for (size_t i=0; i<amp_strs.size(); i++) {
        amp_strs[i] = RStr(settings.stimgrid_amp_uA[i]/1000.0, AUTO, 8) +
          " mA";
      }
      for (size_t i=0; i<freq_strs.size(); i++) {
        freq_strs[i] = RStr(settings.stimgrid_freq_Hz[i]) + " Hz";
      }
      for (size_t i=0; i<dur_strs.size(); i++) {
        dur_strs[i] = RStr(settings.stimgrid_dur_us[i]/1000.0, AUTO, 8) +
          " ms";
      }

      main_window->GetLocConfigChan().SetOptions(chan_strs);
      main_window->GetLocConfigAmp().SetOptions(amp_strs);
      main_window->GetLocConfigFreq().SetOptions(freq_strs);
      main_window->GetLocConfigDur().SetOptions(dur_strs);

      main_window->SwitchToStimPanelLoc();
    }
    else {
      size_t config_box_cnt = std::min(settings.stimconf.size(),
          main_window->StimConfigCount());
      for (size_t c=0; c<config_box_cnt; c++) {
        main_window->GetStimConfigBox(c).SetChannel(
            settings.min_stimconf[c].params, settings.max_stimconf[c].params,
            settings.stimconf[c].label, c);
        main_window->GetStimConfigBox(c).SetParameters(
            settings.stimconf[c].params);
      }

      main_window->SwitchToStimPanelFR();
    }

    main_window->GetStatusPanel()->SetExperiment(settings.exper);
    main_window->GetStatusPanel()->SetEvent("RECORDING");

    SaveDefaultEEG();
    main_window->SetReadyToStart(true);
  }

  void Handler::Shutdown_Handler() {
    eeg_acq.CloseCerebus();
    CloseExperimentComponents();
  }

  void Handler::SaveDefaultEEG() {
    if (settings.exp_config.IsNull()) {
      return;
    }

    std::string sub_name;
    settings.exp_config->Get(sub_name, "subject");

    RStr event_file = File::FullPath(non_session_dir,
        RStr(settings.sub)+"_event_log_"+Time::GetDateTime()+".json");
    RStr eeg_file = File::FullPath(non_session_dir,
        RStr(settings.sub)+"_nonsession_eeg_"+Time::GetDateTime() +
        "." + eeg_save->GetExt());

    event_log.StartFile(event_file);
    eeg_save->StartFile(eeg_file);
  }

  RC::Data1D<CSStimProfile> Handler::CreateGridProfiles() {
    Data1D<CSStimProfile> grid_profiles;
    
    for (size_t c=0; c<settings.stimgrid_chan_on.size(); c++) {
      if (!settings.stimgrid_chan_on[c]) { continue; }
      auto& chan = settings.stimconf[c];
      for (size_t a=0; a<settings.stimgrid_amp_on.size(); a++) {
        if (!settings.stimgrid_amp_on[a]) { continue; }
        auto& amp = settings.stimgrid_amp_uA[a];
        for (size_t f=0; f<settings.stimgrid_freq_on.size(); f++) {
          if (!settings.stimgrid_freq_on[f]) { continue; }
          auto& freq = settings.stimgrid_freq_Hz[f];
          for (size_t d=0; d<settings.stimgrid_dur_on.size(); d++) {
            if (!settings.stimgrid_dur_on[d]) { continue; }
            auto& dur = settings.stimgrid_dur_us[d];

            CSStimChannel target = chan.params;
            target.amplitude = amp;
            target.frequency = freq;
            target.duration = dur;

            CSStimProfile profile;
            profile += target;

            grid_profiles += profile;
          }
        }
      }
    }

    return grid_profiles;
  }

  void Handler::CloseExperimentComponents() {
    net_worker.Close();
    exper_ops.Stop();

    eeg_save->StopSaving();
    event_log.CloseFile();
  }
}

