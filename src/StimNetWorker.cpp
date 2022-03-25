#include "StimNetWorker.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  StimNetWorker::StimNetWorker(RC::Ptr<Handler> hndl, const StimNetWorkerSettings& settings)
    : NetWorker(hndl, "Stim"), settings(settings){
  }

  void StimNetWorker::DisconnectedBefore() {
    status_panel->Clear();
  }

  void StimNetWorker::ConfigureStimulationHelper(StimProfile profile) {
    RC::RStr config = "SPSTIMCONFIG," + RC::RStr(profile.size());
    RC_ForRange(i, 0, profile.size()) {
      const StimChannel& sc = profile[i];
      config += "," + RC::RStr::Join(RC::Data1D<uint32_t>
          {sc.electrode_pos, sc.electrode_neg, sc.amplitude, sc.frequency, sc.duration}, ",");
    }
    Send(config);
  }

  void StimNetWorker::StimulateHelper() {
    Send("SPSTIMSTART");
  }

  void StimNetWorker::OpenHelper() {
    Listen(settings.ip, settings.port); // ip and port
    Send(settings.subject); // Subject number
  }

  void StimNetWorker::CloseHelper() {
    // Other device closes on disconnect
  }

  void StimNetWorker::SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
  }

  void StimNetWorker::ProcessCommand(RC::RStr cmd) {
#ifdef NETWORKER_TIMING
    timer.Start();
#endif // NETWORKER_TIMING

    Data1D<RC::RStr> cmdParts = cmd.SplitFirst(",");
    RC::RStr& cmdName = cmdParts[0];
    RC::RStr& cmdVals = cmdParts[1]; 

    hndl->event_log.Log(cmd);

    if (cmdName == "SPREADY") {
      // Do Nothing
    }
    else if (cmdName != "SPERROR") {
      ErrorWin("Stim client error: " + cmdVals);
    }
    else if (cmdName != "SPSTIMCONFIGDONE") {
      // Do Nothing
    }
    else if (cmdName != "SPSTIMCONFIGERROR") {
      ErrorWin("Stim client configure error: " + cmdVals);
    }
    else if (cmdName != "SPSTIMSTARTDONE") {
      // Do Nothing
    }
    else if (cmdName != "SPSTIMSTARTERROR") {
      ErrorWin("Stim client start error: " + cmdVals);
    }
    else {
      ErrorWin("Unapproved command received from stim client: " + cmd);
    }
  }
}

