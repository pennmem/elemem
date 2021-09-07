#ifndef CLASSIFIER_EVEN_ODD_H
#define CLASSIFIER_EVEN_ODD_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"


namespace CML {
  class ClassifierEvenOdd : public RCqt::WorkerThread {
    public:
    ClassifierEvenOdd(int sampling_rate); 

    int classify(RC::APtr<const EEGData> eegData) override;
    
    protected:
    RC::RStr callback_ID;
  };
}

#endif // CLASSIFIER_EVEN_ODD_H
