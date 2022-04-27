#include "NetWorker.h"
#include "Handler.h"
#include "JSONLines.h"
#include "Popup.h"
#include "RC/Data1D.h"

using namespace RC;

namespace CML {
  NetWorker::NetWorker(RC::Ptr<Handler> hndl, const RC::RStr& netWorkerType)
    : hndl(hndl), netWorkerType(netWorkerType) {
    AddToThread(this);
  }

  NetWorker::~NetWorker() {
    Close_Handler();
  }

  void NetWorker::Listen_Handler(const RC::RStr& address,
                                 const uint16_t& port) {
    connected = false;
    server = new QTcpServer();
    connect(server, &QTcpServer::newConnection, this,
        &NetWorker::NewConnection);

    auto qt_address = QHostAddress(address.ToQString());
    auto qt_addstr = qt_address.toString();
    // Start listening if the conversion checks pass.
    if ( qt_addstr != address.ToQString() || qt_address.isNull() ||
        (! server->listen(QHostAddress(address.ToQString()), port)) ) {
      hndl->StopExperiment();  // Always stop.  Cannot start.
      Throw_RC_Type(Net, (RC::RStr("Could not setup ") + netWorkerType +
            " server on address " + address + " port " +
            RC::RStr(port)).c_str());
    }
    // Resolves initialization threading issue, pushing onto the right thread.
    // Note, Qt requires this to come after the "listen" call.
    AddToThread(server);
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
    return (con.IsSet() && con->isOpen() && connected);
  }

  void NetWorker::StopOnDisconnect_Handler(const bool& stop) {
    stop_on_disconnect = stop;
  }

  void NetWorker::NewConnection() {
    NewConnectionBefore();
    configured = false;
    if (server.IsNull()) {
      return;
    }
    con = server->nextPendingConnection();
    if (con.IsSet()) {
      connect(con, &QTcpSocket::readyRead, this, &NetWorker::DataReady);
      connect(con, &QTcpSocket::disconnected, this, &NetWorker::Disconnected);
      connected = true;
      NewConnectionMade();
    }
    NewConnectionAfter();
  }

  void NetWorker::DataReady() {
    DataReadyBefore();
    auto new_data = con->readAll();
    buffer += RStr(new_data.data(), size_t(new_data.size()));
    Data1D<RStr> split;
    while (buffer.Contains("\n")) {
      split = buffer.SplitFirst('\n');
      buffer = split[1];
      ProcessCommand(split[0]);
    }
    DataReadyAfter();
  }

  void NetWorker::Disconnected() {
    DisconnectedBefore();
    buffer.clear();
    // Message required, unplanned disconnect.
    if (connected) {
      connected = false;
      if (stop_on_disconnect) {
        hndl->StopExperiment();
        ErrorWin(netWorkerType + " disconnected.  Experiment stopped.");
      }
      else {
        ErrorWin(netWorkerType + " disconnected.  Waiting for reconnection.  "
                 "Click \"Stop Experiment\" to stop.");
      }
    }
    DisconnectedAfter();
  }

  void NetWorker::Send(const RC::RStr& msg) {
    if ( ! IsConnected_Handler() ) {
      hndl->StopExperiment();
      Throw_RC_Type(Net, "Tried to send something via NetWorker before "
          "connection was made");
    }

    if (con->write(msg.c_str(), qint64(msg.size())) != qint64(msg.size())) {
      Close_Handler();
    }

#ifdef NETWORKER_TIMING
    cout << (RC::RStr("Response time: ") +
             RC::RStr(timer.SinceStart()) + "\n");
#endif // NETWORKER_TIMING
  }
}

