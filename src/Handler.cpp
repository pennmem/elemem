#include "ConfigFile.h"
#include "Handler.h"
#include "MainWindow.h"
#include "Popup.h"
#include "RC/RC.h"

using namespace std;
using namespace RC;


namespace CML {
  Handler::Handler() : edf_save(this) {
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
    if (stim_test_warning) {
      if (!ConfirmWin("WARNING!  CereStim Test should not be run with a participant connected.  Continue?")) {
        return;
      }

      stim_test_warning = false;
    }

    stim_worker.CloseCereStim();

    APITests::CereStimTest();
  }

  void Handler::SetStimSettings_Handler(const StimSettings& settings_callback) {
    // TODO - implement
  }

  void Handler::OpenConfig_Handler(RC::FileRead& fr) {
    APtr<JSONFile> conf = new JSONFile();
    conf->Load(fr);
    exp_config = conf.ExtractConst();

    std::string elecfilename;
    exp_config->Get(elecfilename, "electrode_config_file");

    APtr<CSVFile> elecs = new CSVFile();
    elecs->Load(elecfilename);
    elec_config = elecs.ExtractConst();
  }
}

