#include "NetWorker.h"
#include "Handler.h"
#include "Popup.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  NetWorker::NetWorker(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    connect(&server, &QTcpServer::newConnection, this,
        &NetWorker::NewConnection);
  }

  NetWorker::~NetWorker() {
    StopListening_Handler();
  }

  void NetWorker::Listen_Handler(const RC::RStr& address,
                                 const uint16_t& port) {
    server.listen(QHostAddress(address.ToQString()), port);
  }

  void NetWorker::Close_Handler() {
    if (con.IsSet()) {
      con->close();
    }
  }

  void NetWorker::StopListening_Handler() {
    server.close();
  }

  bool NetWorker::IsConnected_Handler() {
    return (con.IsSet() && con->isOpen());
  }

  void NetWorker::WarnOnDisconnect_Handler(const bool& warn) {
    stop_on_disconnect = warn;
  }

  void NetWorker::NewConnection() {
    con = server.nextPendingConnection();
    if (con.IsSet()) {
      connect(con, &QTcpSocket::readyRead, this, &NetWorker::DataReady);
      connect(con, &QTcpSocket::disconnected, this, &NetWorker::Disconnected);
    }
  }

  void NetWorker::DataReady() {
    auto new_data = con->readAll();
    buffer += RStr(new_data.data(), size_t(new_data.size()));
    Data1D<RStr> split;
    while (buffer.Contains("\n")) {
      split = buffer.SplitFirst('\n');
      ProcessCommand(split[0]);
      buffer = split[1];
    }
  }

  void NetWorker::Disconnected() {
    if (stop_on_disconnect) {
      hndl->StopExperiment();
      ErrorWin("Task laptop disconnected.  Experiment stopped.");
    }
  }

  JSONFile NetWorker::MakeResp(RC::RStr type, uint64_t id) {
    JSONFile resp;
    resp.SetFilename("HostResponse");
    resp.Set(type.Raw(), "type");
    resp.Set(R"({})"_json, "data");
    if (id != uint64_t(-1)) {
      resp.Set(id, "id");
    }
    resp.Set(Time::Get()*1e3, "time");  // ms from 1970-01-01 00:00:00 UTC
    return resp;
  }

  void NetWorker::Respond(JSONFile& resp) {
    RStr line = resp.Line();
    hndl->event_log.Log(line);
    
    if (con->write(line.c_str(), qint64(line.size())) != qint64(line.size())) {
      Close_Handler();
    }
  }

  void NetWorker::ProcessCommand(RC::RStr cmd) {
    JSONFile inp;
    inp.SetFilename("TaskLaptopCommand");
    inp.Parse(cmd);

    std::string type;
    uint64_t id = uint64_t(-1);
    try {
      inp.Get(type, "type");
    }
    catch (ErrorMsg&) {
      // Not a command.  Ignore.
      return;
    }
    try {
      inp.Get(id, "id");
    }
    catch (ErrorMsg&) {
      // Leave as uint64_t(-1) to disable.
    }

    inp.Set(Time::Get()*1e3, "time");
    hndl->event_log.Log(inp.Line());

    if (type == "CONNECTED") {
      JSONFile resp = MakeResp("CONNECTED_OK");
      Respond(resp);
    }
    else if (type == "CONFIGURE") {
      ProtConfigure(inp);
    }
    else if (type == "READY") {
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
    }
    else {
      // Ignore unknowns.
    }
  }

  void NetWorker::ProtConfigure(const JSONFile& inp) {
    Data1D<RStr> errors;

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
      Respond(resp);
    }
  }

  void NetWorker::ProtWord(const JSONFile& inp) {
    bool do_stim;
    try {
      inp.Get(do_stim, "data", "stim");
      hndl->stim_worker.Stimulate();
    }
    catch (ErrorMsg&) {
      hndl->StopExperiment();
      ErrorWin("Experiment halted, stim setting missing in word event.");
    }
  }

  void NetWorker::Compare(Data1D<RStr>& errors, const RC::RStr& label,
      const std::string& a, const std::string& b) {
    if (RStr(a).ToLower() != RStr(b).ToLower()) {
      errors += RStr(label + " does not match, task: " + a + ", host: " + b);
    }
  }
}

