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

    void ConfigureStimulation(const StimProfile& profile) override { RCqt::TaskCaller<const StimProfile&> configure = TaskHandler(StimNetWorker::ConfigureStimulation_Handler); configure(profile); }
    void OpenInterface() override { RCqt::TaskCaller<> open = TaskHandler(StimNetWorker::Open_Handler); open(); }
    void CloseInterface() override { RCqt::TaskCaller<> close = TaskHandler(StimNetWorker::Close_Handler); close(); }
    void Stimulate() override { RCqt::TaskCaller<> stim = TaskHandler(StimNetWorker::Stimulate_Handler); stim(); }

    uint32_t GetBurstSlowFreq() override { RCqt::TaskGetter<uint32_t> getFreq = TaskHandler(StimNetWorker::GetBurstSlowFreq_Handler); return getFreq(); }
    uint32_t GetBurstDuration_us() override { RCqt::TaskGetter<uint32_t> getDur = TaskHandler(StimNetWorker::GetBurstDuration_us_Handler); return getDur(); }
    
    //typedef uint32_t (StimNetWorker::*FooType)() const;
    //uint32_t GetBurstDuration_us() const override { auto getDur = &StimNetWorker::Foo_Handler; return (this->*getDur)(); }

    protected slots:
    void DisconnectedBefore() override;


    protected:
    void ConfigureStimulation_Handler(const StimProfile& profile) override;
    void Stimulate_Handler() override;
    void Open_Handler() override;
    void Close_Handler() override;

    uint32_t GetBurstSlowFreq_Handler() override;
    uint32_t GetBurstDuration_us_Handler() override;

    void ProcessCommand(RC::RStr cmd) override;

    void LogAndSend(const RC::RStr& msg);
    void SetStatusPanel_Handler(const RC::Ptr<StatusPanel>& set_panel);

    RC::Ptr<StatusPanel> status_panel;
    RC::Ptr<Handler> hndl;

    StimNetWorkerSettings settings;
  };
}

#endif // STIMNETWORKER_H

