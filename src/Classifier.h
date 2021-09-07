#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"


namespace CML {
  class Classifier : public RCqt::WorkerThread {
    public:

    Classifier();

    // TODO: Decide whether this should return an int or a bool
    int classify(RC::APtr<const EEGData> eegData);
    
    protected:
    RC::RStr callback_ID;
  };
}

#endif // CLASSIFIER_EVEN_ODD_H
