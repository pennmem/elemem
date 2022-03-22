#ifndef TASKNETWORKER_H
#define TASKNETWORKER_H

#include "NetWorker.h"

namespace CML {
  class Handler;
  class StatusPanel;

  class TaskNetWorker : public NetWorker {
    public:

    TaskNetWorker(RC::Ptr<Handler> hndl);
    ~TaskNetWorker() = default;

    // Rule of 3.
    TaskNetWorker(const TaskNetWorker&) = delete;
    TaskNetWorker& operator=(const TaskNetWorker&) = delete;

    RCqt::TaskCaller<const RC::Ptr<StatusPanel>> SetStatusPanel =
      TaskHandler(TaskNetWorker::SetStatusPanel_Handler);

    protected slots:
    void Disconnected() override;

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

#endif // TASKNETWORKER_H

