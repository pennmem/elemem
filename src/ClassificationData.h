#ifndef CLASSIFICATIONDATA_H
#define CLASSIFICATIONDATA_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  using ClassifierCallback = RCqt::TaskCaller<const double>;
  using EEGCallback = RCqt::TaskCaller<RC::APtr<const EEGData>>;

  class TaskClassifierSettings {
    public:
    bool sham;
  };

  class ClassificationData : public RCqt::WorkerThread {
    public:
    ClassificationData(RC::Ptr<Handler> hndl, int sampling_rate); 

    RCqt::TaskCaller<const RC::RStr> ProcessTaskClassifierEvent =
      TaskHandler(ClassificationData::ProcessTaskClassifierEvent_Handler);

    ClassifierCallback ClassifierDecision =
      TaskHandler(ClassificationData::ClassifierDecision_Handler);

    RCqt::TaskCaller<const EEGCallback> SetCallback =
      TaskHandler(ClassificationData::SetCallback_Handler);

    protected:
    RCqt::TaskCaller<RC::APtr<const EEGData>> ClassifyData = 
      TaskHandler(ClassificationData::ClassifyData_Handler);

    // TODO: Decide whether to have json configurable variables for the
    //       classifier data, such as binning sizes

    //void StartClassifier_Handler(const RC::RStr& filename,
    //                             const FullConf& conf) override;
    // Thread ordering constraint:
    // Must call Stop after Start, before this Destructor, and before
    // hndl->eeg_acq is deleted.
    //void StopClassifier_Handler() override;
    void ClassifyData_Handler(RC::APtr<const EEGData>& data);

    void ProcessTaskClassifierEvent_Handler(const RC::RStr& event);
    void ClassifierDecision_Handler(const double& result);
    
    void SetCallback_Handler(const EEGCallback& new_callback);

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

    EEGCallback callback;
  };
}

#endif // CLASSIFICATIONDATA_H
