#ifndef HANDLER_H
#define HANDLER_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "APITests.h"
#include "EEGAcq.h"
#include "EEGFileSave.h"
#include "TaskClassifierManager.h"
#include "TaskStimManager.h"
#include "FeatureFilters.h"
#include "Classifier.h"
#include "EventLog.h"
#include "ExperCPS.h"
#include "ExperOPS.h"
#include "NetWorker.h"
#include "Settings.h"
#include "StimWorker.h"
#include "LocGUIConfig.h"
#include "StimGUIConfig.h"
#include <QObject>


namespace CML {
  class MainWindow;
  class JSONFile;
  class CSVFile;
  enum class StimMode { NONE=0, OPEN=1, CLOSED=2 };
  StimMode ToStimMode(const RC::RStr& stim_mode_str);
  RC::RStr FromStimMode(StimMode stim_mode);


  class Handler : public RCqt::WorkerThread {
    public:

    Handler();
    ~Handler();

    // Rule of 3
    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;

    void SetMainWindow(RC::Ptr<MainWindow> new_main);

    RCqt::TaskBlocker<> LoadSysConfig =
      TaskHandler(Handler::LoadSysConfig_Handler);
    RCqt::TaskBlocker<> Initialize =
      TaskHandler(Handler::Initialize_Handler);

    RCqt::TaskCaller<> CerebusTest =
      TaskHandler(Handler::CerebusTest_Handler);

    RCqt::TaskCaller<> CereStimTest =
      TaskHandler(Handler::CereStimTest_Handler);


    RCqt::TaskCaller<const size_t, const StimSettings> SetStimSettings =
      TaskHandler(Handler::SetStimSettings_Handler);
    RCqt::TaskCaller<const size_t> TestStim =
      TaskHandler(Handler::TestStim_Handler);
    RCqt::TaskCaller<> TestLocStim =
      TaskHandler(Handler::TestLocStim_Handler);

    RCqt::TaskCaller<const size_t> TestSelLocChan =
      TaskHandler(Handler::TestSelLocChan_Handler);
    RCqt::TaskCaller<const size_t> TestSelLocAmp =
      TaskHandler(Handler::TestSelLocAmp_Handler);
    RCqt::TaskCaller<const size_t> TestSelLocFreq =
      TaskHandler(Handler::TestSelLocFreq_Handler);
    RCqt::TaskCaller<const size_t> TestSelLocDur =
      TaskHandler(Handler::TestSelLocDur_Handler);

    RCqt::TaskCaller<const RC::Data1D<bool>> SetLocChansApproved =
      TaskHandler(Handler::SetLocChansApproved_Handler);
    RCqt::TaskCaller<const RC::Data1D<bool>> SetLocAmpApproved =
      TaskHandler(Handler::SetLocAmpApproved_Handler);
    RCqt::TaskCaller<const RC::Data1D<bool>> SetLocFreqApproved =
      TaskHandler(Handler::SetLocFreqApproved_Handler);
    RCqt::TaskCaller<const RC::Data1D<bool>> SetLocDurApproved =
      TaskHandler(Handler::SetLocDurApproved_Handler);

    RCqt::TaskCaller<> InitializeChannels =
      TaskHandler(Handler::InitializeChannels_Handler);
    RCqt::TaskCaller<const RC::RStr> SelectStim =
      TaskHandler(Handler::SelectStim_Handler);

    RCqt::TaskCaller<> StartExperiment =
      TaskHandler(Handler::StartExperiment_Handler);

    RCqt::TaskCaller<> StopExperiment =
      TaskHandler(Handler::StopExperiment_Handler);

    RCqt::TaskCaller<> ExperimentExit =
      TaskHandler(Handler::ExperimentExit_Handler);

    RCqt::TaskCaller<RC::FileRead> OpenConfig =
      TaskHandler(Handler::OpenConfig_Handler);

    RCqt::TaskGetter<FullConf> GetConfig =
      TaskHandler(Handler::GetConfig_Handler);

    RCqt::TaskBlocker<> Shutdown =
      TaskHandler(Handler::Shutdown_Handler);

    StimWorker stim_worker;
    EEGAcq eeg_acq;
    RC::APtr<EEGFileSave> eeg_save;
    RC::APtr<TaskClassifierManager> task_classifier_manager;
    RC::APtr<FeatureFilters> feature_filters;
    RC::APtr<Classifier> classifier;
    RC::APtr<TaskStimManager> task_stim_manager;
    NetWorker net_worker;
    EventLog event_log;

    private:

    RC::Ptr<MainWindow> main_window;

    void LoadSysConfig_Handler();
    void Initialize_Handler();

    void CerebusTest_Handler();
    void CereStimTest_Handler();

    void SetStimSettings_Handler(const size_t& index,
                                 const StimSettings& settings_callback);
    void TestStim_Handler(const size_t& index);
    void TestLocStim_Handler();

    void TestSelLocChan_Handler(const size_t& selected);
    void TestSelLocAmp_Handler(const size_t& selected);
    void TestSelLocFreq_Handler(const size_t& selected);
    void TestSelLocDur_Handler(const size_t& selected);

    void SetLocChansApproved_Handler(const RC::Data1D<bool>& approved);
    void SetLocAmpApproved_Handler(const RC::Data1D<bool>& approved);
    void SetLocFreqApproved_Handler(const RC::Data1D<bool>& approved);
    void SetLocDurApproved_Handler(const RC::Data1D<bool>& approved);

    void InitializeChannels_Handler();
    void SelectStim_Handler(const RC::RStr& stimtag);

    void StartExperiment_Handler();
    void StopExperiment_Handler();
    void ExperimentExit_Handler();
    void HandleExit();

    void OpenConfig_Handler(RC::FileRead& fr);
    FullConf GetConfig_Handler() {
      return {settings.exp_config, settings.elec_config};
    }
    void Shutdown_Handler();

    void NewEEGSave();
    void SaveDefaultEEG();
    RC::Data1D<CSStimProfile> CreateGridProfiles();
    RC::Data1D<CSStimProfile> CreateDiscreteStimProfiles();
    void SetupClassifier();

    void CloseExperimentComponents();

    Settings settings;

    RC::RStr elemem_dir;
    RC::RStr non_session_dir;
    RC::RStr session_dir;

    ExperOPS exper_ops;
    ExperCPS exper_cps;
    StimMode stim_mode = StimMode::NONE;

    RC::APtr<QTimer> exit_timer;
    bool do_exit = false;

    bool experiment_running = false;
    bool stim_api_test_warning = true;
  };
}



#endif // HANDLER_H

