#include "ConfigFile.h"
#include "TaskNetWorker.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  TaskNetWorker::TaskNetWorker(RC::Ptr<Handler> hndl)
    : NetWorker(hndl, "Task"), hndl(hndl) {
  }

  void TaskNetWorker::DisconnectedBefore() {
    status_panel->Clear();
  }

  void TaskNetWorker::SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel) {
      status_panel = set_panel;
  }

  void TaskNetWorker::LogAndSend(JSONFile& msg) {
    RC::RStr line = msg.Line();
    hndl->event_log.Log(line);

    Send(line);
  }

  void TaskNetWorker::ProcessCommand(RC::RStr cmd) {
#ifdef NETWORKER_TIMING
    timer.Start();
#endif // NETWORKER_TIMING

// #define TESTING
#if TESTING
    RC_DEBOUT(cmd);
#endif // TESTING

    JSONFile inp;
    inp.SetFilename("TaskLaptopCommand");
    inp.Parse(cmd);

    std::string type;
    uint64_t id = uint64_t(-1);
    if (!inp.TryGet(type, "type")) {
      // Not a command.  Ignore.
      return;
    }
    if (!inp.TryGet(id, "id")) {
      // Leave as uint64_t(-1) to disable.
    }

    inp.Set(Time::Get()*1e3, "time");
    hndl->event_log.Log(inp.Line());

    if (type == "CONNECTED") {
      JSONFile response = MakeResp("CONNECTED_OK");
      LogAndSend(response);
      status_panel->SetEvent(type);
    }
    else if (type == "CONFIGURE") {
      ProtConfigure(inp);
    }
    else {
      if (!configured) {
        ErrorWin("Unapproved commands received from task laptop on "
                 "connection without verified CONFIGURE.");
        return;
      }
      if (type == "READY") {
        hndl->eeg_acq.StartingExperiment();  // notify, replay needs this.
        JSONFile response = MakeResp("START");
        LogAndSend(response);
      }
      else if (type == "HEARTBEAT") {
        JSONFile response = MakeResp("HEARTBEAT_OK");
        try {
          uint64_t count;
          inp.Get(count, "data", "count");
          response.Set(count, "data", "count");
        }
        catch (...) { }
        LogAndSend(response);
      }
      else if (type == "WORD") {
        ProtWord(inp);
        status_panel->SetEvent(type);
      }
      else if (type == "STIMSELECT") {
        RC::RStr stimtag;
        inp.Get(stimtag, "data", "stimtag");
        hndl->SelectStim(stimtag);
      }
      else if (type == "STIM") {
        hndl->stim_worker.Stimulate();
      }
      else if (type == "CLSTIM") {
        uint64_t classify_ms;
        inp.Get(classify_ms, "data", "classifyms");
        hndl->task_classifier_manager->ProcessClassifierEvent(
            ClassificationType::STIM, classify_ms, id);
      }
      else if (type == "CLSHAM") {
        uint64_t classify_ms;
        inp.Get(classify_ms, "data", "classifyms");
        hndl->task_classifier_manager->ProcessClassifierEvent(
            ClassificationType::SHAM, classify_ms, id);
      }
      else if (type == "CLNORMALIZE") {
        uint64_t classify_ms;
        inp.Get(classify_ms, "data", "classifyms");
        hndl->task_classifier_manager->ProcessClassifierEvent(
            ClassificationType::NORMALIZE, classify_ms, id);
      }
      else if (type == "CCLSTARTSTIM") {
        uint64_t duration_s;
        inp.Get(duration_s, "data", "duration");
        hndl->exper_cps.Start(duration_s);
      }
      else if (type == "SESSION") {
        int64_t session;
        inp.Get(session, "data", "session");
        status_panel->SetSession(session);
      }
      else if (type == "TRIAL") {
        int64_t trial;
        bool stim;
        inp.Get(trial, "data", "trial");
        status_panel->SetTrial(trial);
        inp.Get(stim, "data", "stim");
        status_panel->SetStimList(stim);
      }
      else if (type == "EXIT") {
        status_panel->SetEvent(type);
        hndl->ExperimentExit();
      }
      else {
        if (type ==
            RC::OneOf("ORIENT", "COUNTDOWN", "DISTRACT", "RECALL", "REST",
                      "INSTRUCT", "TRIALEND", "MATH", "ENCODING")) {
          status_panel->SetEvent(type);
        }
      }
    }
  }


  void TaskNetWorker::ProtConfigure(const JSONFile& inp) {
    Data1D<RStr> errors;
    Data1D<RStr> stimtags;

    std::string task_stim_mode;
    std::string task_experiment;
    std::string task_subject;
    std::string host_stim_mode;
    std::string host_experiment;
    std::string host_subject;

    try {
      auto conf = hndl->GetConfig();
      inp.Get(task_stim_mode, "data", "stim_mode");
      inp.Get(task_experiment, "data", "experiment");
      inp.Get(task_subject, "data", "subject");
      conf.exp_config->Get(host_stim_mode, "experiment", "stim_mode");
      conf.exp_config->Get(host_experiment, "experiment", "type");
      conf.exp_config->Get(host_subject, "subject");
      Compare(errors, "stim_mode", task_stim_mode, host_stim_mode);
      Compare(errors, "experiment", task_experiment, host_experiment);
      Compare(errors, "subject", task_subject, host_subject);

      if (inp.TryGet(stimtags, "data", "stimtags")) {
        for (size_t s=0; s<stimtags.size(); s++) {
          hndl->SelectStim(stimtags[s]);
        }
      }
    }
    catch (ErrorMsg& e) {
      errors += RStr(e.what()).SplitFirst("\n")[0];
    }
    if (errors.size() > 0) {
      JSONFile response = MakeResp("CONFIGURE_ERROR");
      RStr json_msg = RC::RStr::Join(errors, "; ");
      response.Set(json_msg.c_str(), "data", "error");
      LogAndSend(response);

      hndl->StopExperiment();
      RStr human_msg = RC::RStr::Join(errors, "\n");
      ErrorWin("Experiment halted, configuration error:\n" + human_msg);
    }
    else {
      JSONFile response = MakeResp("CONFIGURE_OK");
      configured = true;
      LogAndSend(response);
    }
  }

  void TaskNetWorker::ProtWord(const JSONFile& inp) {
    bool do_stim;

    if (inp.TryGet(do_stim, "data", "stim")) {
      if (do_stim) {
        hndl->stim_worker.Stimulate();
      }
    }
  }

  void TaskNetWorker::Compare(Data1D<RStr>& errors, const RC::RStr& label,
      const std::string& a, const std::string& b) {
    if (RStr(a).ToLower() != RStr(b).ToLower()) {
      errors += RStr(label + " does not match, task: " + a + ", host: " + b);
    }
  }
}

