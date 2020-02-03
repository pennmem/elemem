#ifndef NETWORKER_H
#define NETWORKER_H

#include "RC/APtr.h"
#include "RC/RStr.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"
#include "ConfigFile.h"
#include <QTcpServer>
#include <QTcpSocket>

namespace CML {
  class Handler;

  class NetWorker : public RCqt::WorkerThread, public QObject {
    public:

    NetWorker(RC::Ptr<Handler> hndl);
    ~NetWorker();

    // Rule of 3.
    NetWorker(const NetWorker&) = delete;
    NetWorker& operator=(const NetWorker&) = delete;

    RCqt::TaskCaller<const RC::RStr, const uint16_t> Listen =
      TaskHandler(NetWorker::Listen_Handler);

    RCqt::TaskCaller<> Close =
      TaskHandler(NetWorker::Close_Handler);
    RCqt::TaskCaller<> StopListening =
      TaskHandler(NetWorker::StopListening_Handler);

    RCqt::TaskGetter<bool> IsConnected =
      TaskHandler(NetWorker::IsConnected_Handler);

    RCqt::TaskCaller<const bool> WarnOnDisconnect =
      TaskHandler(NetWorker::WarnOnDisconnect_Handler);

    static JSONFile MakeResp(RC::RStr type, uint64_t id=uint64_t(-1));

    protected slots:
    void NewConnection();
    void DataReady();
    void Disconnected();

    protected:
    void Listen_Handler(const RC::RStr& address, const uint16_t& port);
    void Close_Handler();
    void StopListening_Handler();

    bool IsConnected_Handler();
    void WarnOnDisconnect_Handler(const bool& warn);

    void ProcessCommand(RC::RStr cmd);
    void Respond(JSONFile& resp);

    void ProtConfigure(const JSONFile& inp);
    void ProtWord(const JSONFile& inp);

    void Compare(RC::Data1D<RC::RStr>& errors, const RC::RStr& label,
        const std::string& a, const std::string& b);

    RC::Ptr<Handler> hndl;
    QTcpServer server;
    RC::APtr<QTcpSocket> con;
    RC::RStr buffer;
    bool stop_on_disconnect = false;
  };
}

#endif // NETWORKER_H

