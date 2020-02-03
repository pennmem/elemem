#ifndef NETWORKER_H
#define NETWORKER_H

#include "RC/APtr.h"
#include "RC/RStr.h"
#include "RC/Ptr.h"
#include <QTcpServer>
#include <QTcpSocket>

namespace CML {
  class NetWorker : public RCqt::WorkerThread, QObject {
    public:

    NetWorker();
    ~NetWorker();

    // Rule of 3.
    NetWorker(const NetWorker&) = delete;
    NetWorker& operator=(const NetWorker&) = delete;

    RCqt::TaskCaller<const RC::RStr, const uint16_t> Listen =
      TaskHandler(NetWorker::Listen_Handler);

    RCqt::TaskCaller<> Close =
      TaskHandler(NetWorker::Close_Handler);
    RCqt::TaskCaller<> StopListening =
      TaskHandler(NetWorker::Stop_Handler);

    RCqt::TaskBlocker<bool> IsConnected =
      TaskHandler(NetWorker::IsConnected_Handler);

    RCqt::TaskCaller<bool> WarnOnDisconnect =
      TaskHandler(NetWorker::WarnOnDisconnect_Handler);

    static JSONFile MakeResp(RC::RStr type, uint64_t id=uint64_t(-1));

    protected slot:
    void NewConnection();
    void DataReady();
    void Disconnected();

    protected:
    void Listen_Handler(const RC::RStr& address, const uint16_t& port);
    void Close_Handler();
    void StopListening_Handler();

    bool IsConnected_Handler();
    void WarnOnDisconnect_Handler();

    void ProcessCommand(RC::RStr cmd);
    void Respond(JSONFile& resp);

    void ProtConfigure(const JSONFile& inp);
    void ProtWord(const JSONFile& inp);

    void Compare(Data1D<RStr>& errors, const RC::RStr& label,
        const std::string& a, const std::string& b);

    RC::Ptr<Handler> hndl;
    QTcpServer server;
    RC::APtr<QTcpSocket> con;
    RC::RStr buffer;
    bool stop_on_disconnect = false;
  };
}

#endif // NETWORKER_H

