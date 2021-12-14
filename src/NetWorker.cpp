#include "NetWorker.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "StatusPanel.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  NetWorker::NetWorker(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    AddToThread(this);
  }

  NetWorker::~NetWorker() {
    Close_Handler();
  }

  void NetWorker::Listen_Handler(const RC::RStr& address,
                                 const uint16_t& port) {
    server = new QTcpServer();
    connect(server, &QTcpServer::newConnection, this,
        &NetWorker::NewConnection);

    server->listen(QHostAddress(address.ToQString()), port);
    connected = false;
  }

  void NetWorker::Close_Handler() {
    if (con.IsSet()) {
      connected = false;
      con->close();
      con.Delete();
    }
    if (server.IsSet()) {
      server->close();
      server.Delete();
    }
  }

  bool NetWorker::IsConnected_Handler() {
    return (con.IsSet() && con->isOpen());
  }

  void NetWorker::WarnOnDisconnect_Handler(const bool& warn) {
    stop_on_disconnect = warn;
  }

  void NetWorker::NewConnection() {
    configured = false;
    if (server.IsNull()) {
      return;
    }
    con = server->nextPendingConnection();
    if (con.IsSet()) {
      connect(con, &QTcpSocket::readyRead, this, &NetWorker::DataReady);
      connect(con, &QTcpSocket::disconnected, this, &NetWorker::Disconnected);
      connected = true;
    }
  }

  void NetWorker::DataReady() {
    auto new_data = con->readAll();
    buffer += RStr(new_data.data(), size_t(new_data.size()));
    Data1D<RStr> split;
    while (buffer.Contains("\n")) {
      split = buffer.SplitFirst('\n');
      buffer = split[1];
      ProcessCommand(split[0]);
    }
  }

  void NetWorker::Disconnected() {
    status_panel->Clear();
    buffer.clear();
    if (stop_on_disconnect) {
      hndl->StopExperiment();
    }
    // Message required, unplanned disconnect.
    if (connected) {
      connected = false;
      if (stop_on_disconnect) {
        ErrorWin("Task laptop disconnected.  Experiment stopped.");
      }
      else {
        ErrorWin("Task laptop disconnected.  Waiting for reconnection.  "
                 "Click \"Stop Experiment\" to stop.");
      }
    }
  }


  void NetWorker::Respond(JSONFile& resp) {
    RStr line = resp.Line();
    hndl->event_log.Log(line);
    
    if (con->write(line.c_str(), qint64(line.size())) != qint64(line.size())) {
      Close_Handler();
    }
#ifdef NETWORKER_TIMING
    cout << (RC::RStr("Response time: ") +
             RC::RStr(timer.SinceStart()) + "\n");
#endif // NETWORKER_TIMING
  }

  void NetWorker::ProcessCommand(RC::RStr cmd) {
#ifdef NETWORKER_TIMING
    timer.Start();
#endif // NETWORKER_TIMING

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
      JSONFile resp = MakeResp("CONNECTED_OK");
      Respond(resp);
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
        JSONFile resp = MakeResp("START");
        Respond(resp);
      }
      else if (type == "HEARTBEAT") {
        JSONFile resp = MakeResp("HEARTBEAT_OK");
        try {
          uint64_t count;
          inp.Get(count, "data", "count");
          resp.Set(count, "data", "count");
        }
        catch (...) { }
        Respond(resp);
      }
      else if (type == "WORD") {
        ProtWord(inp);
        status_panel->SetEvent(type);
      }
      else if (type == "STIM") {
        hndl->stim_worker.Stimulate();
      }
      else if (type == "CLSTIM") {
        uint64_t classifyms;
        inp.Get(classifyms, "data", "classifyms");
        ClassificationType cl_type = ClassificationType::STIM;
        hndl->task_classifier_manager->ProcessClassifierEvent(cl_type, classifyms);
      }
      else if (type == "CLSHAM") {
        uint64_t classifyms;
        inp.Get(classifyms, "data", "classifyms");
        ClassificationType cl_type = ClassificationType::SHAM;
        hndl->task_classifier_manager->ProcessClassifierEvent(cl_type, classifyms);
      }
      else if (type == "CLNORMALIZE") {
        uint64_t duration;
        inp.Get(duration, "data", "duration");
        ClassificationType cl_type = ClassificationType::NORMALIZE;
        hndl->task_classifier_manager->ProcessClassifierEvent(cl_type, duration);
      }
      else if (type == "STIMSELECT") {
        RC::RStr stimtag;
        inp.Get(stimtag, "data", "stimtag");
        hndl->SelectStim(stimtag);
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
            RC::OneOf("ORIENT", "COUNTDOWN", "MATH", "RECALL", "REST",
                      "TRIALEND")) {
          status_panel->SetEvent(type);
        }
      }
    }
  }


  void NetWorker::ProtConfigure(const JSONFile& inp) {
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
      JSONFile resp = MakeResp("CONFIGURE_ERROR");
      RStr json_msg = RC::RStr::Join(errors, "; ");
      resp.Set(json_msg.c_str(), "data", "error");
      Respond(resp);

      hndl->StopExperiment();
      RStr human_msg = RC::RStr::Join(errors, "\n");
      ErrorWin("Experiment halted, configuration error:\n" + human_msg);
    }
    else {
      JSONFile resp = MakeResp("CONFIGURE_OK");
      configured = true;
      Respond(resp);
    }
  }

  void NetWorker::ProtWord(const JSONFile& inp) {
    bool do_stim;

    if (inp.TryGet(do_stim, "data", "stim")) {
      if (do_stim) {
        hndl->stim_worker.Stimulate();
      }
    }
  }

  void NetWorker::Compare(Data1D<RStr>& errors, const RC::RStr& label,
      const std::string& a, const std::string& b) {
    if (RStr(a).ToLower() != RStr(b).ToLower()) {
      errors += RStr(label + " does not match, task: " + a + ", host: " + b);
    }
  }
}

