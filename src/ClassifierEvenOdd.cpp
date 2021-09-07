#ifndef CLASSIFIER_EVEN_ODD_H
#define CLASSIFIER_EVEN_ODD_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"

    ClassifierEvenOdd::ClassifierEvenOdd(int sampling_rate) {
      callback_ID = RC::RStr("ClassifierEvenOdd_") + RC::RStr(sampling_rate);
    }

    // NOT WORKING (just returns 1)
    int ClassifierEvenOdd::classify(RC::APtr<const EEGData> eegData) {
      // TODO: Split the wavelet data into 2 threads for classification
      // TODO: Change the return to be whether the first EEG value is even or odd 
      return 1;
    }
    
    protected:
    RC::RStr callback_ID;
  };
}

#endif // CLASSIFIER_EVEN_ODD_H
