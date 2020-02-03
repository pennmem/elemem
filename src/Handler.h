#ifndef HANDLER_H
#define HANDLER_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "APITests.h"
#include "EDFSave.h"
#include "EEGAcq.h"
#include "EventLog.h"
#include "NetWorker.h"
#include "StimWorker.h"
#include "StimGUIConfig.h"
#include <QObject>


namespace CML {
  class MainWindow;
  class JSONFile;
  class CSVFile;
  
  class Handler : public RCqt::WorkerThread {
    public:

    Handler();
    ~Handler();

    void SetMainWindow(RC::Ptr<MainWindow> new_main);

    RCqt::TaskCaller<> CerebusTest =
      TaskHandler(Handler::CerebusTest_Handler);

    RCqt::TaskCaller<> CereStimTest =
      TaskHandler(Handler::CereStimTest_Handler);


    RCqt::TaskCaller<const size_t, const StimSettings> SetStimSettings =
      TaskHandler(Handler::SetStimSettings_Handler);
    RCqt::TaskCaller<const size_t> TestStim =
      TaskHandler(Handler::TestStim_Handler);

    RCqt::TaskCaller<> StartExperiment =
        TaskHandler(Handler::StartExperiment_Handler);

    RCqt::TaskCaller<> StopExperiment =
        TaskHandler(Handler::StopExperiment_Handler);

    RCqt::TaskCaller<RC::FileRead> OpenConfig =
      TaskHandler(Handler::OpenConfig_Handler);

    StimWorker stim_worker;
    EEGAcq eeg_acq;
    EDFSave edf_save;
    NetWorker net_worker;
    EventLog event_log;
    RC::APtr<const JSONFile> exp_config;
    RC::APtr<const CSVFile> elec_config;

    const RC::RStr elemem_dir;

    private:

    RC::Ptr<MainWindow> main_window;

    void CerebusTest_Handler();
    void CereStimTest_Handler();

    void SetStimSettings_Handler(const size_t& index,
                                 const StimSettings& settings_callback);
    void TestStim_Handler(const size_t& index);

    void StartExperiment_Handler();
    void StopExperiment_Handler();

    void OpenConfig_Handler(RC::FileRead& fr);
    void SaveDefaultEEG();

    RC::Data1D<StimSettings> stim_settings;
    RC::Data1D<StimSettings> min_stim_settings;
    RC::Data1D<StimSettings> max_stim_settings;

    RC::RStr session_dir;

    bool experiment_running = false;
    bool stim_api_test_warning = true;
  };
}



#endif // HANDLER_H

