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
    ClassificationData(RC::Ptr<Handler> hndl, size_t sampling_rate); 

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

    void SetCallback_Handler(const EEGCallback& new_callback);

    void ClassifierDecision_Handler(const double& result);

    RC::Ptr<Handler> hndl;
    EEGData buffer;
    size_t sampling_rate;
    RC::RStr callback_ID;
    TaskClassifierSettings task_classifier_settings;

    EEGCallback callback;

    u32 dataSinceLastClassification;
  };
}

#endif // CLASSIFICATIONDATA_H
