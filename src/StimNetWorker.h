#ifndef STIMNETWORKER_H
#define STIMNETWORKER_H

#include "NetWorker.h"

namespace CML {
  class Handler;
  class StatusPanel;

  class StimNetWorker : public NetWorker {
    public:

    StimNetWorker(RC::Ptr<Handler> hndl);
    ~StimNetWorker() = default;

    // Rule of 3.
    StimNetWorker(const StimNetWorker&) = delete;
    StimNetWorker& operator=(const StimNetWorker&) = delete;

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(StimNetWorker::SetStatusPanel_Handler);

    protected slots:
    void DisconnectedBefore() override;

    protected:
    void ProcessCommand(RC::RStr cmd) override;

    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel);

    void ProtConfigure(const JSONFile& inp);
    void ProtWord(const JSONFile& inp);

    void Compare(RC::Data1D<RC::RStr>& errors, const RC::RStr& label,
        const std::string& a, const std::string& b);

    RC::Ptr<StatusPanel> status_panel;
  };
}

#endif // STIMNETWORKER_H

