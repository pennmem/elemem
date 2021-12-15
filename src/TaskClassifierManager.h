#ifndef CLASSIFICATIONDATA_H
#define CLASSIFICATIONDATA_H

#include "EEGData.h"
#include "TaskClassifierSettings.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  using ClassifierCallback = RCqt::TaskCaller<const double, const TaskClassifierSettings>;
  using TaskClassifierCallback = RCqt::TaskCaller<RC::APtr<const EEGData>, const TaskClassifierSettings>;

  // TODO: JPB: (refactor) Make this a base class
  class TaskClassifierManager : public RCqt::WorkerThread {
    public:
    TaskClassifierManager(RC::Ptr<Handler> hndl, size_t sampling_rate); 

    RCqt::TaskCaller<const ClassificationType, const uint64_t> ProcessClassifierEvent =
      TaskHandler(TaskClassifierManager::ProcessClassifierEvent_Handler);

    ClassifierCallback ClassifierDecision =
      TaskHandler(TaskClassifierManager::ClassifierDecision_Handler);

    RCqt::TaskCaller<const TaskClassifierCallback> SetCallback =
      TaskHandler(TaskClassifierManager::SetCallback_Handler);

    protected:
    RCqt::TaskCaller<RC::APtr<const EEGData>> ClassifyData = 
      TaskHandler(TaskClassifierManager::ClassifyData_Handler);

    void ClassifyData_Handler(RC::APtr<const EEGData>& data);

    void ProcessClassifierEvent_Handler(const ClassificationType& cl_type, const uint64_t& duration_ms);
    void ClassifierDecision_Handler(const double& result, const TaskClassifierSettings& task_classifier_settings);
    
    void SetCallback_Handler(const TaskClassifierCallback& new_callback);

    // TODO: JPB: (refactor) Make this into it's own CiruclarBuffer class or Binning class or something?
    RC::APtr<EEGData> GetCircularBufferData();
    void PrintCircularBuffer();
    void UpdateCircularBuffer(RC::APtr<const EEGData>& new_data);
    void UpdateCircularBuffer(RC::APtr<const EEGData>& new_data, size_t start);
    void UpdateCircularBuffer(RC::APtr<const EEGData>& new_data, size_t start, size_t amnt);
    RC::APtr<EEGData> BinData(RC::APtr<const EEGData> in_data, size_t new_sampling_rate);

    void StartClassification();

    RC::Ptr<Handler> hndl;
    RC::RStr callback_ID;

    EEGData circular_data;
    size_t circular_data_start = 0;

    TaskClassifierSettings task_classifier_settings;

    bool stim_event_waiting = false;
    size_t num_eeg_events_before_stim = 0;

    TaskClassifierCallback callback;
  };
}

#endif // CLASSIFICATIONDATA_H
