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

    void ConfigureStimulation(StimProfile profile) override { RCqt::TaskCaller<const StimProfile> configure = TaskHandler(StimNetWorker::ConfigureStimulation_Handler); configure(profile); }
    void OpenInterface() override { RCqt::TaskCaller<> open = TaskHandler(StimNetWorker::OpenInterface_Handler); open(); }
    void CloseInterface() override { RCqt::TaskCaller<> close = TaskHandler(StimNetWorker::CloseInterface_Handler); close(); }
    void Stimulate() override { RCqt::TaskCaller<> stim = TaskHandler(StimNetWorker::Stimulate_Handler); stim(); }

    uint32_t GetBurstSlowFreq() override { RCqt::TaskGetter<uint32_t> getFreq = TaskHandler(StimNetWorker::GetBurstSlowFreq_Handler); return getFreq(); }
    uint32_t GetBurstDuration_us() override { RCqt::TaskGetter<uint32_t> getDur = TaskHandler(StimNetWorker::GetBurstDuration_us_Handler); return getDur(); }
    
    //typedef uint32_t (StimNetWorker::*FooType)() const;
    //uint32_t GetBurstDuration_us() const override { auto getDur = &StimNetWorker::Foo_Handler; return (this->*getDur)(); }


    protected:
    void ConfigureStimulation_Helper(const StimProfile& profile) override;
    void Stimulate_Helper() override;
    void OpenInterface_Helper() override;
    void CloseInterface_Helper() override;

    // TODO: JPB: (feature) Fix MakeCaller to handle Base Class types
    // This is a temporary redeclaration
    void ConfigureStimulation_Handler(const StimProfile& profile) { StimInterface::ConfigureStimulation_Handler(profile); }
    void Stimulate_Handler() { StimInterface::Stimulate_Handler(); }
    void OpenInterface_Handler() { StimInterface::OpenInterface_Handler(); }
    void CloseInterface_Handler() { StimInterface::CloseInterface_Handler(); }
    uint32_t GetBurstSlowFreq_Handler() { return StimInterface::GetBurstSlowFreq_Handler(); }
    uint32_t GetBurstDuration_us_Handler() { return StimInterface::GetBurstDuration_us_Handler(); }

    void ProcessCommand(RC::RStr cmd) override;

    void LogAndSend(const RC::RStr& msg);

    RC::Ptr<Handler> hndl;

    StimNetWorkerSettings settings;
  };
}

#endif // STIMNETWORKER_H

