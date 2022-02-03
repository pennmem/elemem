#ifndef TASKSTIMMANAGER_H
#define TASKSTIMMANAGER_H

#include "TaskClassifierSettings.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  using ClassifierCallback = RCqt::TaskCaller<const double, const TaskClassifierSettings>;
  using TaskStimCallback = RCqt::TaskCaller<const bool, const TaskClassifierSettings>;

  // TODO: JPB: (refactor) Make this a base class
  class TaskStimManager : public RCqt::WorkerThread {
    public:
    TaskStimManager(RC::Ptr<Handler> hndl);

    ClassifierCallback StimDecision =
      TaskHandler(TaskStimManager::StimDecision_Handler);

    RCqt::TaskCaller<const TaskStimCallback> SetCallback =
      TaskHandler(TaskStimManager::SetCallback_Handler);

    protected:
    void StimDecision_Handler(const double& result, const TaskClassifierSettings& task_classifier_settings);

    void SetCallback_Handler(const TaskStimCallback& new_callback);

    RC::Ptr<Handler> hndl;
	RC::RStr callback_ID;

    TaskStimCallback callback;
  };
}

#endif // TASKSTIMMANAGER_H
