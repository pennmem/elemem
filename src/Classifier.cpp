#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"


namespace CML {
  class Classifier : public RCqt::WorkerThread {
    public:

    Classifier::Classifier() {}

    int Classifier::classify(RC::APtr<const EEGData> eegData) {
      // TODO: Ryan, this feels wrong to me (I don't think there is a valid
      //       default classifier). Are you okay with this?
      // Always return true;
      return true;
    }
    
    protected:
    RC::RStr callback_ID;
  };
}

#endif // CLASSIFIER_EVEN_ODD_H
