#include "StimNetWorker.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"
#include "RC/RStr.h"

using namespace RC;

namespace CML {
  StimNetWorker::StimNetWorker(RC::Ptr<Handler> hndl,
      const StimNetWorkerSettings& settings)
    : NetWorker(hndl, "Stim"), hndl(hndl), settings(settings) {
  }

  void StimNetWorker::DisconnectedBefore() {
    status_panel->Clear();
  }

  void StimNetWorker::LogAndSend(const RC::RStr &msg) {
    JSONFile response = MakeResp("STIMNETMSG");
    response.Set(msg, "data", "msg");
    hndl->event_log.Log(response.Line());

    Send(msg);
  }

  void StimNetWorker::ConfigureStimulationHelper(StimProfile profile) {
    // CONNECT (this is here because it needs the experiment to be loaded first)
    RC::RStr subject;
    try {
      auto conf = hndl->GetConfig();
      conf.exp_config->Get(subject, "subject");
    } catch (ErrorMsg& e){
      Throw_RC_Type(Net, ("Could not find subject in experiment config.  "
              "The experiment likely has not been loaded yet." +
               RStr(e.what()).SplitFirst("\n")[0]).c_str());
    }

    if ( ! IsConnected_Handler() ) {
      StopExpIfShould();
      Throw_RC_Type(Net, "Cannot configure stim.  Stim Network Process not "
          "connected.");
    }

    LogAndSend(subject); // Subject number

    // CONFIGURE
    RC::RStr config = "SPSTIMCONFIG," + RC::RStr(profile.size());
    RC_ForRange(i, 0, profile.size()) {
      const StimChannel& sc = profile[i];
      config += "," + RC::RStr::Join(RC::Data1D<uint32_t>
          {sc.electrode_pos, sc.electrode_neg, sc.amplitude, sc.frequency, sc.duration}, ",");
    }
    LogAndSend(config);
  }

  void StimNetWorker::StimulateHelper() {
    LogAndSend("SPSTIMSTART");
  }

  void StimNetWorker::OpenHelper() {
    Listen(settings.ip, settings.port); // ip and port
  }

  void StimNetWorker::CloseHelper() {
    // Other device closes on disconnect
    // TODO: JPB: (need) Diamond Inheritance
    //Close();
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
      // TODO: JPB: (need) Make this stop the experiment
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

