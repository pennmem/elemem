#ifndef TASKCLASSIFIERMANAGER_H
#define TASKCLASSIFIERMANAGER_H

#include "EEGData.h"
#include "EEGCircularData.h"
#include "TaskClassifierSettings.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  using ClassifierEvent = RCqt::TaskCaller<const ClassificationType, const uint64_t, const uint64_t>;
  using ClassifierCallback = RCqt::TaskCaller<const double, const TaskClassifierSettings>;
  using TaskClassifierCallback = RCqt::TaskCaller<RC::APtr<const EEGData>, const TaskClassifierSettings>;

  // TODO: JPB: (refactor) Make this a base class
  class TaskClassifierManager : public RCqt::WorkerThread {
    public:
    TaskClassifierManager(RC::Ptr<Handler> hndl, size_t sampling_rate,
      size_t circ_buf_duration_ms);

    ~TaskClassifierManager();
    // Rule of 3.
    TaskClassifierManager(const TaskClassifierManager&) = delete;
    TaskClassifierManager& operator=(const TaskClassifierManager&) = delete;

    ClassifierEvent ProcessClassifierEvent =
      TaskHandler(TaskClassifierManager::ProcessClassifierEvent_Handler);

    RCqt::TaskCaller<const TaskClassifierCallback> SetCallback =
      TaskHandler(TaskClassifierManager::SetCallback_Handler);

    RCqt::TaskBlocker<> Shutdown =
      TaskHandler(TaskClassifierManager::Shutdown_Handler);

    protected:
    RCqt::TaskCaller<RC::APtr<const EEGData>> ClassifyData =
      TaskHandler(TaskClassifierManager::ClassifyData_Handler);

    void ClassifyData_Handler(RC::APtr<const EEGData>& data);

    void ProcessClassifierEvent_Handler(const ClassificationType& cl_type,
        const uint64_t& duration_ms, const uint64_t& classif_id);

    void SetCallback_Handler(const TaskClassifierCallback& new_callback);

    void Shutdown_Handler();

    void StartClassification();

    RC::Ptr<Handler> hndl;
    RC::RStr callback_ID;

    EEGCircularData circular_data;

    size_t sampling_rate = 0;
    TaskClassifierSettings task_classifier_settings;

    bool stim_event_waiting = false;
    size_t num_eeg_events_before_stim = 0;

    TaskClassifierCallback callback;
  };
}

#endif // TASKCLASSIFIERMANAGER_H
