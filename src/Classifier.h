#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"


namespace CML {
  class Handler;

  class Classifier : public RCqt::WorkerThread {
    public:

    Classifier(RC::Ptr<Handler> hndl, size_t sampling_rate); 

    //Classifier(RC::Ptr<Handler> hndl, size_t sampling_rate) 
    //  : hndl(hndl), buffer(sampling_rate), 
    //    sampling_rate(sampling_rate) {
    //  callback_ID = RC::RStr("Classifier_") + RC::RStr(sampling_rate);
    //  buffer.sampling_rate = sampling_rate;

    //  hndl->eeg_acq.RegisterCallback(callback_ID, ClassifyData);
    //}
    
    RCqt::TaskCaller<RC::APtr<const EEGData>> ClassifyData = 
      TaskHandler(Classifier::ClassifyData_Handler);

    protected:
    //void StartClassifier_Handler(const RC::RStr& filename,
    //                             const FullConf& conf) override;
    // Thread ordering constraint:
    // Must call Stop after Start, before this Destructor, and before
    // hndl->eeg_acq is deleted.
    //void StopClassifier_Handler() override;
    void ClassifyData_Handler(RC::APtr<const EEGData>& data);

    RC::Ptr<Handler> hndl;
    EEGData buffer;
    size_t sampling_rate;
    RC::RStr callback_ID;
  };
}

#endif // CLASSIFIER_H
