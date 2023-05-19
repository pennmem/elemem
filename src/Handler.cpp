#include "ChannelSelector.h"
#include "ConfigFile.h"
#include "Handler.h"
#ifdef NO_HDF5
#include "EDFSave.h"
#else
#include "HDF5Save.h"
#endif
#ifdef CEREBUS_HW
#include "Cerebus.h"
#endif
#include "CerebusSim.h"
#include "StimNetWorker.h"
#include "ClassifierLogReg.h"
#include "EDFReplay.h"
#include "EDFSynch.h"
#include "JSONLines.h"
#include "About.h"
#include "MainWindow.h"
#include "Popup.h"
#include "Utils.h"
#include "RC/RC.h"
#include <QDir>
#include <QObject>
#include <type_traits>

#include "Testing.h"

using namespace std;
using namespace RC;

namespace CML {
  StimMode ToStimMode(const RC::RStr& stim_mode_str) {
    if (stim_mode_str == "none") { return StimMode::NONE; }
    if (stim_mode_str == "open") { return StimMode::OPEN; }
    if (stim_mode_str == "closed") { return StimMode::CLOSED; }
    Throw_RC_Error((stim_mode_str + " is not a valid stim mode.").c_str());
  }

  RC::RStr FromStimMode(StimMode stim_mode) {
    if (stim_mode == StimMode::NONE) { return "none"; }
    if (stim_mode == StimMode::OPEN) { return "open"; }
    if (stim_mode == StimMode::CLOSED) { return "closed"; }
    Throw_RC_Error((RC::RStr(uint64_t(stim_mode)) +
        " does not represent a recognized StimMode.").c_str());
  }

  Handler::Handler()
    : stim_worker(this),
      task_net_worker(this),
      sig_quality(&eeg_acq),
      exper_cps(this),
      exper_ops(this) {
    // For error management, everything that could error must go into
    // Initialize_Handler()
  }

  Handler::~Handler() {
    // Worker tasks are already stopped before getting here.
  }

  void Handler::SetMainWindow(Ptr<MainWindow> new_main) {
    main_window = new_main;
    task_net_worker.SetStatusPanel(main_window->GetStatusPanel());
    stim_worker.SetStatusPanel(main_window->GetStatusPanel());
    exper_ops.SetStatusPanel(main_window->GetStatusPanel());
    exper_cps.SetStatusPanel(main_window->GetStatusPanel());
  }

  void Handler::LoadSysConfig_Handler() {
    if (experiment_running) {
      Throw_RC_Error("Attempted to load system config while experiment "
          "running!");
    }

    settings.LoadSystemConfig();

    // EEG System
    RC::RStr eeg_system;
    settings.sys_config->Get(eeg_system, "eeg_system");
    RC::APtr<EEGSource> eeg_source;
    if (eeg_system == "Cerebus") {
      #ifdef CEREBUS_HW
      uint32_t chan_count;
      std::vector<uint32_t> extra_chans;
      settings.sys_config->Get(chan_count, "channel_count");
      settings.sys_config->Get(extra_chans, "extra_channels");
      eeg_source = new Cerebus(chan_count, extra_chans);
      #else
      Throw_RC_Type(File, "sys_config.json eeg_system set to \"Cerebus\", "
          "but this build does not have Cerebus Hardware support.");
      #endif
    }
    else if (eeg_system == "CerebusSim") {
      eeg_source = new CerebusSim();
    }
    else if (eeg_system == "EDFReplay") {
      RC::RStr edfreplay_file =
        settings.sys_config->GetPath("replay_file");
      eeg_source = new EDFReplay(edfreplay_file);
    }
    else {
      Throw_RC_Type(File, "Unknown sys_config.json eeg_system value");
    }
    eeg_acq.SetSource(eeg_source);
    InitializeChannels_Handler();

    // Stim System
    RC::RStr stim_system;
    settings.sys_config->Get(stim_system, "stim_system");
    RC::APtr<StimInterface> stim_interface;
    if (stim_system == "CereStim") {
      stim_interface = new CereStim();
    }
    else if (stim_system == "StimNetWorker") {
      std::string stim_ip;
      uint16_t stim_port;
      settings.sys_config->Get(stim_ip, "stimcom_ip");
      settings.sys_config->Get(stim_port, "stimcom_port");
      RC::Ptr<StimNetWorker> stim_net_worker =
        new StimNetWorker(this, {.ip=stim_ip, .port=stim_port});
      stim_net_worker->StopOnDisconnect(true);
      stim_interface = stim_net_worker;
    }
    else {
      Throw_RC_Type(File, "Unknown sys_config.json stim_system value");
    }
    stim_worker.SetStimInterface(stim_interface);
    stim_worker.Open();
  }

  void Handler::Initialize_Handler() {
    EDFSynch::Inst();  // edflib thread synch initialize

    RStr data_dir = settings.sys_config->GetPath("data_dir");

    elemem_dir = data_dir;
    non_session_dir = File::FullPath(elemem_dir, "NonSessionData");

    File::MakeDir(elemem_dir);
    File::MakeDir(non_session_dir);

    RC::RStr error_log_dir = File::FullPath(elemem_dir, "ErrorLogs");
    File::MakeDir(error_log_dir);
    RC::RStr error_log_file = File::FullPath(error_log_dir,
        "error_log_" + Time::GetDate());
    PopupManager::GetManager()->SetLogFile(error_log_file);

    NewEEGSave();

//#define TESTING
#ifdef TESTING
    std::cerr << "TESTING ENABLED:" << std::endl;
    TestAllCode();
#endif // TESTING
  }

  void Handler::CerebusTest_Handler() {
    if (experiment_running) {
      ErrorWin("Attempted CerebusTest while experiment running!");
      return;
    }

    eeg_acq.CloseSource();

    APITests::CereLinkTest();
  }

  void Handler::CereStimTest_Handler() {
    if (stim_api_test_warning) {
      if (!ConfirmWin("WARNING!  CereStim Test should not be run with a participant connected.  Continue?")) {
        return;
      }

      stim_api_test_warning = false;
    }

    if (experiment_running) {
      ErrorWin("Attempted CereStimTest while experiment running!");
      return;
    }

    stim_worker.CloseStim();

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
      if (settings.exper.find("CPS") == 0) {
        Throw_RC_Error("Cannot edit CPS parameter search range while experiment is running.");
      }
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
       settings.min_stimconf[index].params.duration) ||
      (updated_settings.params.area !=
       settings.stimconf[index].params.area) ||
      (updated_settings.params.burst_frac !=
       settings.stimconf[index].params.burst_frac) ||
      (updated_settings.params.burst_slow_freq !=
       settings.stimconf[index].params.burst_slow_freq) ||
      (updated_settings.stimtag !=
       settings.stimconf[index].stimtag)
      ) {
      main_window->GetStimConfigBox(index).SetParameters(
        settings.stimconf[index].params);
      ErrorWin("Attempted to set invalid stim settings.", "Safety check");
      return;
    }

    settings.stimconf[index] = updated_settings;

    // set CPS parameter search ranges
    if (settings.exper.find("CPS") == 0) {
      settings.min_stimconf_range[index] = updated_settings;
      // hardcode min amplitude to min value of 0.1 mA; non-zero stim values will never
      // be below 0.1 mA/100 uA (CereStim/StimProfile/StimChannel amplitudes are recorded in uA)
      settings.min_stimconf_range[index].params.amplitude = 100;
      settings.max_stimconf_range[index] = updated_settings;
    }
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

    StimProfile profile;
    profile += settings.stimconf[index].params;

    stim_worker.ConfigureStimulation(profile);
    RC::Time::Sleep(0.5); // Allow time to configure.
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

    StimProfile profile;
    StimChannel stimchan =
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

  void Handler::InitializeChannels_Handler() {
    eeg_acq.InitializeChannels(settings.sampling_rate, settings.binned_sampling_rate);
  }

  // SelectStim_Handler goes through settings.stimconf and extracts values
  // that are both approved and matching the given stimtag.
  void Handler::SelectStim_Handler(const RC::RStr& stimtag) {
    if (stim_mode == StimMode::NONE) {
      Throw_RC_Error("Attempted to select a stim tag outside of a "
          "stimulation experiment.");
    }

    StimProfile profile;
    for (size_t c=0; c<settings.stimconf.size(); c++) {
      if (settings.stimconf[c].approved &&
          (settings.stimconf[c].stimtag == stimtag)) {
        profile += settings.stimconf[c].params;
      }
    }

    if (profile.size() == 0) {
      ErrorWin("No approved stim configurations match stim tag '" +
          stimtag + "'", "Stim Tag Selection Failed");
      StopExperiment_Handler();
    }

    stim_worker.ConfigureStimulation(profile);
  }

  void Handler::StartExperiment_Handler() {
    if (settings.exp_config.IsNull() || settings.elec_config.IsNull()) {
      ErrorWin("You must load a valid experiment configuration file before "
               "starting an experiment session.", "Unconfigured");
      return;
    }

    if (sigqual_running) {
      ErrorWin("The Signal Quality check must complete before starting "
               "an experiment.");
    }

    if ( ! CheckStorage(elemem_dir, 1024*1024*1024) ) {
      ErrorWin("Cannot start experiment with less than 1GB of free space at "
               "configured ElememData directory: " + elemem_dir);
      return;
    }

    if (settings.grid_exper) {  // OPS
      size_t grid_size = settings.GridSize();
      if (grid_size == 0) {
        ErrorWin("Error!  At least one value must be approved for each "
            "stim parameter of a grid search experiment.");

        return;
      }

      Data1D<StimProfile> grid_profiles = CreateGridProfiles();
      exper_ops.SetOPSSpecs(settings.ops_specs);
      exper_ops.SetStimProfiles(grid_profiles);
    }
    else if (settings.exper.find("CPS") == 0) {
      // Count all approved.
      StimProfile max_cnt_profile;
      StimProfile min_cnt_profile;
      for (size_t c=0; c<settings.max_stimconf_range.size(); c++) {
        if (settings.max_stimconf_range[c].approved) {
          min_cnt_profile += settings.min_stimconf_range[c].params;
          max_cnt_profile += settings.max_stimconf_range[c].params;
        }
      }
      if (max_cnt_profile.size() == 0 && stim_mode != StimMode::NONE) {
        if (!ConfirmWin("No stim channels approved on experiment configured "
             "with stimulation.  Proceed?")) {
          return;
        }
      }
      if (max_cnt_profile.size() > 0 && stim_mode == StimMode::NONE) {
        ErrorWin("Error!  Stim channels enabled on experiment with "
                 "experiment:stim_mode set to \"none\".");
        return;
      }

      // But default select only those with no stimtag.
      Data1D<StimProfile> max_discrete_stim_param_sets;
      Data1D<StimProfile> min_discrete_stim_param_sets;
      for (size_t c=0; c<settings.max_stimconf_range.size(); c++) {
        if (settings.max_stimconf_range[c].approved &&
            settings.max_stimconf_range[c].stimtag.empty()) {
          StimProfile min_profile;
          min_profile += settings.min_stimconf_range[c].params;
          min_discrete_stim_param_sets += min_profile;
          StimProfile max_profile;
          max_profile += settings.max_stimconf_range[c].params;
          max_discrete_stim_param_sets += max_profile;
        }
      }

      exper_cps.SetCPSSpecs(settings.cps_specs);

      // discrete stim param sets only give fixed (non-optimized) stim parameters
      exper_cps.SetStimProfiles(min_discrete_stim_param_sets, max_discrete_stim_param_sets);
    }
    else {
      // Count all approved.
      StimProfile cnt_profile;
      for (size_t c=0; c<settings.stimconf.size(); c++) {
        if (settings.stimconf[c].approved) {
          cnt_profile += settings.stimconf[c].params;
        }
      }
      if (cnt_profile.size() == 0 && stim_mode != StimMode::NONE) {
        if (!ConfirmWin("No stim channels approved on experiment configured "
             "with stimulation.  Proceed?")) {
          return;
        }
      }
      if (cnt_profile.size() > 0 && stim_mode == StimMode::NONE) {
        ErrorWin("Error!  Stim channels enabled on experiment with "
                 "experiment:stim_mode set to \"none\".");
        return;
      }

      // But default select only those with no stimtag.
      StimProfile profile;
      for (size_t c=0; c<settings.stimconf.size(); c++) {
        if (settings.stimconf[c].approved &&
            settings.stimconf[c].stimtag.empty()) {
          profile += settings.stimconf[c].params;
        }
      }

      stim_worker.ConfigureStimulation(profile);
    }

    if (stim_mode == StimMode::CLOSED) {
      SetupClassifier();
    }

    experiment_running = true;
    main_window->SetReadyToStart(false);
    for (size_t i=0; i<main_window->StimConfigCount(); i++) {
      main_window->GetStimConfigBox(i).SetEnabled(false);
    }

    for (size_t i=0; i<main_window->MinMaxStimConfigCount(); i++) {
      main_window->GetMinMaxStimConfigBox(i).SetEnabled(false);
    }

    RStr sub_dir = RStr(settings.sub) + "_" + Time::GetDateTime();
    session_dir = File::FullPath(elemem_dir, sub_dir);
    File::MakeDir(session_dir);

    // Save updated experiment configuration.
    if (stim_mode == StimMode::CLOSED) {
      RStr classif_json =
          settings.exp_config->GetPath("experiment", "classifier",
          "classifier_file");
      // Save copy of classifier file.
      auto jf = JSONFile(File::FullPath(config_dir, classif_json));
      jf.Save(File::FullPath(session_dir, File::Basename(classif_json)));
    }

    cps_setup.current_config = *(settings.exp_config);
    // for CPS
    if (settings.exper.find("OPS") == 0) {
      settings.UpdateConfOPS(cps_setup.current_config);
    }
    else if (settings.exper.find("CPS") == 0) {
      cps_setup.prev_sessions = settings.UpdateConfCPS(cps_setup.current_config,
          elemem_dir, session_dir, settings.elec_config->GetFilename());
      settings.grid_exper = false;
      settings.task_driven = false;
      classifier->RegisterCallback("CPSClassifierDecision", exper_cps.ClassifierDecision);
      feature_filters->RegisterCallback("CPSHandleNormalization", exper_cps.HandleNormalization);
      task_stim_manager->SetCallback(exper_cps.StimDecision);
    }
    else {
      settings.UpdateConfFR(cps_setup.current_config);
    }
    cps_setup.current_config.Save(File::FullPath(session_dir,
          "experiment_config.json"));

    // Save copy of loaded electrode config.
    FileWrite fw_mono(File::FullPath(session_dir,
          File::Basename(settings.elec_config->GetFilename())));
    fw_mono.Put(settings.elec_config->file_lines, true);
    fw_mono.Close();

    // Save copy of bipolar electrode config if available.
    if (settings.bipolar_config.IsSet()) {
      FileWrite fw_bi(File::FullPath(session_dir,
            File::Basename(settings.bipolar_config->GetFilename())));
      fw_bi.Put(settings.bipolar_config->file_lines, true);
      fw_bi.Close();
    }

    eeg_acq.StartingExperiment();  // notify, replay needs this.
    event_log.StartFile(File::FullPath(session_dir, "event.log"));

    JSONFile version_info;
    version_info.Set(ElememVersion(), "version");
    event_log.Log(MakeResp("ELEMEM", 0, version_info).Line());

    RC::RStr eeg_file = File::FullPath(session_dir,
        RC::RStr("eeg_data.") + eeg_save->GetExt());

    // Estimate storage needed for one hour of acquisition, 2-bytes/sample.
    size_t size_needed = 2 * settings.sampling_rate *
        settings.elec_config->data.size2() * 60 * 60;
    if ( ! CheckStorage(session_dir, size_needed) ) {
      ErrorWin(RC::RStr("Insufficient space to complete experiment!")
               + "  Approximately " + RC::RStr(1+size_needed/1024/1024) +
               "MB needed for 1 hour of acquisition.");
    }

    // Start acqusition
    eeg_save->StartFile(eeg_file, GetConfig_Handler());

    // Mark when eeg file started in event log.
    JSONFile evlog_start_data;
    evlog_start_data.Set(sub_dir, "sub_dir");
    event_log.Log(MakeResp("EEGSTART", 0, evlog_start_data).Line());

    bool skip_signal_quality = false;
    if (settings.sys_config->TryGet(skip_signal_quality, "skip_signal_quality")
        && skip_signal_quality) {
      RunExperiment();
    }
    else {
      // Set monopolar montage channels for signal quality check.
      u64 chan_count;
      settings.sys_config->Get(chan_count, "channel_count");
      RC::Data1D<bool> chan_mask(chan_count);
      chan_mask.Zero();
      for (size_t i=0; i<settings.elec_config->data.size2(); i++) {
        u64 chnum = settings.elec_config->data[i][1].Get_u64();
        if (chnum-1 < chan_mask.size()) {
          chan_mask[chnum-1] = true;
        }
      }
      sig_quality.SetChannelMask(chan_mask);

      // Run signal quality check.
      main_window->GetStatusPanel()->SetEvent("SIGQUAL");
      PopupWin("Running 30 second signal quality check", "Signal Quality");
      sig_quality.RegisterCallback("StartExperiment",
          Handler::SigQualityDone);
      sig_quality.Start();  // Upon success this ends up in RunExperiment.
      sigqual_running = true;
    }
  }


  RC::RStr Handler::SigQualityResultsLine(const SigQualityResults& results) {
    RStr rep_str = RStr::Join(results.report_messages, " ");
    JSONFile data;
    data.Set(results.linenoise_frac, "linenoise_frac");
    data.Set(results.worst_channum, "worst_channum");
    data.Set(results.worst_channel, "worst_channel");
    data.Set(rep_str, "report");
    data.Set(results.measured_freq, "measured_freq");
    data.Set(results.success, "success");
    return MakeResp("signal_quality", 0, data).Line();
  }


  void Handler::RunSignalQuality_Handler() {
    if (experiment_running) {
      ErrorWin("Cannot manually run signal quality check while experiment "
          "running.");
      return;
    }
    main_window->SetReadyToStart(false);
    PopupWin("Running 30 second signal quality check", "Signal Quality");
    sig_quality.RegisterCallback("RunSignalQuality",
        Handler::RunSigQualDone);
    sig_quality.Start();  // Upon success this ends up in RunExperiment.
    sigqual_running = true;
  }


  void Handler::RunSigQualDone_Handler(const SigQualityResults& results) {
    sigqual_running = false;
    RStr msg = "Signal Quality Check ";
    msg += results.success ? "passed.\n\n" : "failed.\n\n";
    msg += RStr::Join(results.report_messages, "\n");
    msg += "\n\nAdditional information in ErrorLog.";
    DebugLog(SigQualityResultsLine(results));
    PopupWin(msg, "Signal Quality Check");
    main_window->SetReadyToStart(settings.exp_config.IsSet());
  }


  void Handler::SigQualityDone_Handler(const SigQualityResults& results) {
    sigqual_running = false;
    event_log.Log(SigQualityResultsLine(results));
    if (results.success) {
      RunExperiment();
    }
    else {
      StopExperiment_Handler();
      RStr msg = RStr("Signal Quality Check failed.\n\n") +
        RStr::Join(results.report_messages, "\n") +
        "\nAdditional information in event log.";
      ErrorWin(msg);
    }
  }


  void Handler::RunExperiment() {
    auto SetupNetworkTask = [&]() {
      // Note:  Binding to a specific LAN address is a safety feature.
      std::string ipaddress = "192.168.137.1";
      uint16_t port = 8889;
      // if (stim_worker.GetStimulatorType() == StimulatorType::Simulator) {
      //  // It's safe to accept connections from anywhere with simulators.
      //  ipaddress = "0.0.0.0";
      // }
      settings.sys_config->Get(ipaddress, "taskcom_ip");
      settings.sys_config->Get(port, "taskcom_port");

      task_net_worker.Listen(ipaddress, port);
      main_window->GetStatusPanel()->SetEvent("WAITING");
    };

    if (settings.grid_exper) {
      exper_ops.Start();
    }
    else if (settings.exper.find("CPS") == 0) {
      exper_cps.Setup(cps_setup.prev_sessions);
      bool with_video;
      cps_setup.current_config.Get(with_video, "experiment",
          "experiment_specs", "with_video_task");
      if (with_video) { SetupNetworkTask(); }
      else {  // for testing without the video task
          if (!ConfirmWin("Running CPS without video task. Continue?")) { return; }
          size_t runtime_s;
          cps_setup.current_config.Get(runtime_s, "experiment",
              "experiment_specs", "default_experiment_duration_secs");
          exper_cps.Start(runtime_s);
      }
    }
    else {  // Network experiment.
      SetupNetworkTask();
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
    for (size_t i=0; i<main_window->MinMaxStimConfigCount(); i++) {
      main_window->GetMinMaxStimConfigBox(i).SetEnabled(true);
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
      AddToThread(exit_timer);  // For maintenance robustness.
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

    if (sigqual_running) {
      ErrorWin("The Signal Quality check must complete before "
               "opening a new configuration");
      return;
    }

    CloseExperimentComponents();

    // Clear the gui stim elements.
    for (size_t c=0; c<main_window->StimConfigCount(); c++) {
      main_window->GetStimConfigBox(c).Clear();
    }
    for (size_t c=0; c<main_window->MinMaxStimConfigCount(); c++) {
      main_window->GetMinMaxStimConfigBox(c).Clear();
    }
    main_window->GetLocConfigChan().Clear();
    main_window->GetLocConfigAmp().Clear();
    main_window->GetLocConfigFreq().Clear();
    main_window->GetLocConfigDur().Clear();

    APtr<JSONFile> conf = new JSONFile();
    conf->Load(fr);
    config_dir = File::Dirname(fr.GetFilename());

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

      settings.exp_config->Get(settings.macro_sampling_rate,
          "global_settings", "macro_sampling_rate");
      settings.sampling_rate = settings.macro_sampling_rate;
      try { // If available.
        settings.exp_config->Get(settings.micro_sampling_rate,
            "global_settings", "micro_sampling_rate");
        settings.sampling_rate = settings.micro_sampling_rate;
      }
      catch(ErrorMsgFile&) { }

      settings.exp_config->Get(settings.binned_sampling_rate,
          "global_settings", "binned_sampling_rate");

      InitializeChannels_Handler();

      new_chans = settings.LoadElecConfig(config_dir);
      if (settings.BipolarElecConfigUsed()) {
        new_chans = settings.LoadBipolarElecConfig(config_dir, new_chans);
        eeg_acq.SetBipolarChannels(new_chans);
      }
      else {
        RC::Data1D<EEGChan> empty;
        eeg_acq.SetBipolarChannels(empty);
      }
      settings.LoadChannelSettings();

      if (settings.grid_exper) {
        settings.LoadStimParamGrid();
      }
      else if (settings.exper.find("CPS") == 0) {
        settings.LoadStimParamsCPS();
      }

      RStr stim_mode_str;
      settings.exp_config->Get(stim_mode_str, "experiment", "stim_mode");
      stim_mode_str.ToLower();
      stim_mode = ToStimMode(stim_mode_str);

      if (stim_mode == StimMode::CLOSED) {
        RStr classif_json =
          settings.exp_config->GetPath("experiment", "classifier",
            "classifier_file");
        settings.weight_manager = MakeAPtr<WeightManager>(
            File::FullPath(config_dir, classif_json), settings.elec_config);
      }
    }
    catch (ErrorMsg&) {
      // Something was wrong with this file.  Cannot proceed.
      settings.Clear();
      throw;
    }

    // Resets sampling rate from the config file value.
    NewEEGSave();


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
    // TODO: RDD: update for CPS with min/max safety testing interface
    else if (settings.exper.find("CPS") == 0) {
      size_t config_box_cnt = std::min(settings.stimconf.size(),
          main_window->MinMaxStimConfigCount());
      for (size_t c=0; c<config_box_cnt; c++) {
        main_window->GetMinMaxStimConfigBox(c).SetChannel(
            settings.min_stimconf[c].params, settings.max_stimconf[c].params,
            settings.stimconf[c].label, settings.stimconf[c].stimtag, c);
        // for now just pass stimconf through to MinMaxStimConfigBox
        main_window->GetMinMaxStimConfigBox(c).SetParameters(
            settings.stimconf[c].params);
      }
      main_window->SwitchToStimPanelCPS();
    }
    else {
      size_t config_box_cnt = std::min(settings.stimconf.size(),
          main_window->StimConfigCount());
      for (size_t c=0; c<config_box_cnt; c++) {
        main_window->GetStimConfigBox(c).SetChannel(
            settings.min_stimconf[c].params, settings.max_stimconf[c].params,
            settings.stimconf[c].label, settings.stimconf[c].stimtag, c);
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
    eeg_acq.CloseSource();
    CloseExperimentComponents();
  }

  void Handler::NewEEGSave() {
#ifdef NO_HDF5
    eeg_save = new EDFSave(this, settings.sampling_rate);
#else
    eeg_save = new HDF5Save(this, settings.sampling_rate);
#endif
  }


  void Handler::SaveDefaultEEG() {
    if (settings.exp_config.IsNull()) {
      return;
    }

    static bool ran_once = false;
    // Don't even try if there is less than 1GB.
    if ( ! CheckStorage(elemem_dir, 1024*1024*1024) ) {
      if ( ! ran_once ) {
        ErrorWin("Cannot start saving non-session EEG data with less than "
                 "1GB of free space at configured ElememData directory: "
                 + elemem_dir);
      }
      return;
    }
    ran_once = true;

    std::string sub_name;
    settings.exp_config->Get(sub_name, "subject");

    RStr event_file = File::FullPath(non_session_dir,
        RStr(settings.sub)+"_event_log_"+Time::GetDateTime()+".json");
    RStr eeg_file = File::FullPath(non_session_dir,
        RStr(settings.sub)+"_nonsession_eeg_"+Time::GetDateTime() +
        "." + eeg_save->GetExt());

    event_log.StartFile(event_file);
    eeg_save->StartFile(eeg_file, GetConfig_Handler());
  }


  RC::Data1D<StimProfile> Handler::CreateGridProfiles() {
    Data1D<StimProfile> grid_profiles;

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

            StimChannel target = chan.params;
            target.amplitude = amp;
            target.frequency = freq;
            target.duration = dur;

            StimProfile profile;
            profile += target;

            grid_profiles += profile;
          }
        }
      }
    }

    return grid_profiles;
  }


  RC::Data1D<StimProfile> Handler::CreateDiscreteStimProfiles() {
    Data1D<StimProfile> profiles;

    for (size_t c=0; c<settings.stimgrid_chan_on.size(); c++) {
      if (!settings.stimgrid_chan_on[c]) { continue; }
      auto& chan = settings.stimconf[c];
      // TODO: RDD: tuning amplitude for now; update to tune arbitrary stim parameters
      for (size_t f=0; f<settings.stimgrid_freq_on.size(); f++) {
        if (!settings.stimgrid_freq_on[f]) { continue; }
        auto& freq = settings.stimgrid_freq_Hz[f];
        for (size_t d=0; d<settings.stimgrid_dur_on.size(); d++) {
          if (!settings.stimgrid_dur_on[d]) { continue; }
          auto& dur = settings.stimgrid_dur_us[d];

          StimChannel target = chan.params;
          target.amplitude = 0;
          target.frequency = freq;
          target.duration = dur;

          StimProfile profile;
          profile += target;

          profiles += profile;
        }
      }
    }

    return profiles;
  }


  void Handler::SetupClassifier() {
    size_t circ_buf_duration_ms;
    settings.exp_config->Get(circ_buf_duration_ms, "experiment", "classifier",
        "circular_buffer_duration_ms");

    auto& freqs = settings.weight_manager->weights->freqs;
    auto& chans = settings.weight_manager->weights->chans;

    double max_freq = *std::max_element(freqs.begin(), freqs.end());
    if (settings.binned_sampling_rate < 2.99 * max_freq) { // This is equivalent to 3 with float rounding errors
      Throw_RC_Error(("Binned frequency (" + RC::RStr(settings.binned_sampling_rate) + ") " +
            "is less than 2.99 times the maximum frequency " +
            "(" + RC::RStr(max_freq) + ")").c_str());
    }

    // Setup all settings first.  Config failures happen here.
    ButterworthSettings but_set;
    but_set.channels = chans;
    but_set.sampling_rate = settings.binned_sampling_rate;
    settings.sys_config->Get(but_set.cpus, "closed_loop_thread_level");

    MorletSettings mor_set;
    mor_set.channels = chans;
    mor_set.frequencies = freqs;
    mor_set.sampling_rate = settings.binned_sampling_rate;
    settings.exp_config->Get(mor_set.cycle_count, "experiment", "classifier",
        "morlet_cycles");
    settings.sys_config->Get(mor_set.cpus, "closed_loop_thread_level");

    NormalizePowersSettings np_set;
    np_set.eventlen = 1; // This is set to 1 because data is averaged first
    np_set.chanlen = chans.size();
    np_set.freqlen = freqs.size();

    ClassifierLogRegSettings classifier_settings;

    // Allocate components.
    task_classifier_manager = new TaskClassifierManager(this,
        settings.binned_sampling_rate, circ_buf_duration_ms);

    feature_filters = new FeatureFilters(this, mor_set.channels, but_set,
        mor_set, np_set);

    classifier = new ClassifierLogReg(this, classifier_settings,
        settings.weight_manager->weights);

    task_stim_manager = new TaskStimManager(this);

    // Register the callbacks.
    task_classifier_manager->SetCallback(feature_filters->Process);
    feature_filters->RegisterCallback("ClassifierClassify",
        classifier->Classify);
    classifier->RegisterCallback("ClassifierDecision",
        task_stim_manager->StimDecision);

    classifier_running = true;
  }


  void Handler::ShutdownClassifier() {
    if ( ! classifier_running ) {
      return;
    }

    // Remove EEGAcq input.
    if (task_classifier_manager.IsSet()) {
      task_classifier_manager->Shutdown();
    }

    // Shut down all the threads.
    if (task_classifier_manager.IsSet()) {
      task_classifier_manager->ExitWait();
    }
    if (feature_filters.IsSet()) {
      feature_filters->ExitWait();
    }
    if (classifier.IsSet()) {
      classifier->ExitWait();
    }
    if (task_stim_manager.IsSet()) {
      task_stim_manager->ExitWait();
    }

    // Clean up data.
    task_classifier_manager.Delete();
    feature_filters.Delete();
    classifier.Delete();
    task_stim_manager.Delete();

    classifier_running = false;
  }

  void Handler::CloseExperimentComponents() {
    ShutdownClassifier();
    task_net_worker.Close();
    exper_ops.Stop();
    exper_cps.Stop();
    sig_quality.Stop();
    sigqual_running = false;

    eeg_save->StopSaving();
    event_log.CloseFile();
  }
}

