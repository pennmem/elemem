#ifndef CLASSIFICATION_DATA_H
#define CLASSIFICATION_DATA_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"


namespace CML {
  class Handler;
  class Classifier;

  using EEGCallback = RCqt::TaskCaller<RC::APtr<const EEGData>>;

  class ClassificationData : public RCqt::WorkerThread {
    public:
    ClassificationData(RC::Ptr<Handler> hndl, size_t sampling_rate); 

    RCqt::TaskCaller<RC::APtr<const EEGData>> ClassifyData = 
      TaskHandler(ClassificationData::ClassifyData_Handler);
    
    RCqt::TaskCaller<const EEGCallback> SetCallback =
      TaskHandler(ClassificationData::SetCallback_Handler);


    protected:
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

    RC::Ptr<Handler> hndl;
    EEGData buffer;
    size_t sampling_rate;
    RC::RStr callback_ID;

    EEGCallback callback;

    u32 dataSinceLastClassification;
  };
}

#endif // CLASSIFICATION_DATA_H
