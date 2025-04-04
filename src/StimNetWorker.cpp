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

  void StimNetWorker::DisconnectedAfter() {
    is_configured = false;
  }

  void StimNetWorker::LogAndSend(const RC::RStr &msg) {
    JSONFile response = MakeResp("STIMNETMSG");
    response.Set(msg, "data", "msg");
    hndl->event_log.Log(response.Line());

    Send(msg);
  }

  void StimNetWorker::ConfigureStimulation_Helper(const StimProfile& profile) {
    if ( ! IsConnected_Handler() ) {
      hndl->StopExperiment();
      Throw_RC_Type(Net ,"Cannot configure stim.  Stim Network Process not "
          "connected.");
    }

    // CONNECT
    // This is here because it needs the experiment to be loaded first
    if ( ! is_configured ) { // Only send connect message when first connecting
      RC::RStr subject;
      try {
        auto conf = hndl->GetConfig();
        conf.exp_config->Get(subject, "subject");
      }
      catch (ErrorMsg& e){
        Throw_RC_Type(Net, ("Could not find subject in experiment config.  "
                "The experiment likely has not been loaded yet." +
                 RStr(e.what()).SplitFirst("\n")[0]).c_str());
      }
      subject += "\n";
      LogAndSend(subject); // Subject number
    }

    // CONFIGURE
    RC::RStr config;
    if (burst_frac < 1) { // Theta-burst
      config = "SPSTIMTHETACONFIG," + RC::RStr(profile.size());
    }
    else {
      config = "SPSTIMCONFIG," + RC::RStr(profile.size());
    }
    RC_ForRange(i, 0, profile.size()) {
      const StimChannel& sc = profile[i];
      config += "," + RC::RStr::Join(RC::Data1D<uint32_t>
          {sc.electrode_pos, sc.electrode_neg, sc.amplitude, sc.frequency}
          , ",");
      if (burst_frac < 1) { // Theta-burst
        uint64_t num_bursts = size_t(std::round(1e-6 * sc.duration *
              sc.burst_slow_freq));
        if (num_bursts > 60) {
          Throw_RC_Error(("Theta burst num_bursts "+ RC::RStr(num_bursts) +
              " exceeded hard limit of 60.").c_str());
        }
        config += "," + RC::RStr(burst_slow_freq) +
                  "," + RC::RStr(burst_frac) +
                  "," + RC::RStr(num_bursts);
      }
      else {
        config += "," + RC::RStr(uint32_t(std::round(sc.duration/1000.0)));
      }
    }
    config += "\n";
    LogAndSend(config);
  }

  void StimNetWorker::Stimulate_Helper() {
    if ( ! is_configured ) {
      throw std::runtime_error("Stimulation attempted when no stim pattern "
          "was internally configured.");
    }
    LogAndSend("SPSTIMSTART\n");
  }

  void StimNetWorker::OpenInterface_Helper() {
    Listen(settings.ip, settings.port); // ip and port
  }

  void StimNetWorker::CloseInterface_Helper() {
    Close(); // Network device
  }

  void StimNetWorker::ProcessCommand(RC::RStr cmd) {
#ifdef NETWORKER_TIMING
    timer.Start();
#endif // NETWORKER_TIMING

    JSONFile cmdJson = MakeResp("STIMNETMSG");
    cmdJson.Set(cmd, "data", "msg");
    hndl->event_log.Log(cmdJson.Line());

    Data1D<RC::RStr> cmdParts = cmd.Chomp().SplitFirst(",");
    RC::RStr& cmdName = cmdParts[0];
    RC::RStr& cmdVals = cmdParts[1];

    if (cmdName == "SPREADY") {
      // Do Nothing
    }
    else if (cmdName == "SPERROR") {
      hndl->StopExperiment();
      ErrorWin("Stim client error: " + cmdVals);
    }
    else if (cmdName == "SPSTIMCONFIGDONE") {
      // Do Nothing
    }
    else if (cmdName == "SPSTIMCONFIGERROR") {
      hndl->StopExperiment();
      ErrorWin("Stim client configure error: " + cmdVals);
    }
    else if (cmdName == "SPSTIMSTARTDONE") {
      // Do Nothing
    }
    else if (cmdName == "SPSTIMSTARTERROR") {
      hndl->StopExperiment();
      ErrorWin("Stim client start error: " + cmdVals);
    }
    else {
      hndl->StopExperiment();
      ErrorWin("Unapproved command received from stim client: " + cmd);
    }
  }
}

