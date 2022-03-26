#ifndef STIMNETWORKER_H
#define STIMNETWORKER_H

#include "NetWorker.h"
#include "StimInterface.h"

namespace CML {
  class Handler;
  class StatusPanel;

  class StimNetWorkerSettings {
    public:
      RC::RStr ip;
      uint16_t port;
  };

  class StimNetWorker : public StimInterface, public NetWorker {
    public:
    StimNetWorker(RC::Ptr<Handler> hndl, const StimNetWorkerSettings& settings);
    ~StimNetWorker() = default;

    // Rule of 3.
    StimNetWorker(const StimNetWorker&) = delete;
    StimNetWorker& operator=(const StimNetWorker&) = delete;

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(StimNetWorker::SetStatusPanel_Handler);


    protected slots:
    void DisconnectedBefore() override;


    protected:
    void ConfigureStimulationHelper(StimProfile profile) override;
    void StimulateHelper() override;
    void OpenHelper() override;
    void CloseHelper() override;

    void ProcessCommand(RC::RStr cmd) override;

    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel);

    RC::Ptr<StatusPanel> status_panel;

    StimNetWorkerSettings settings;
  };
}

#endif // STIMNETWORKER_H

