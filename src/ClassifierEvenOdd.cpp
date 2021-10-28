#include "ClassifierEvenOdd.h"

#define F2I(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

namespace CML {
    /// Constuctor which sets the sampling rate
    /** @param sampling_rate The sampling rate of the incoming data
     */
    ClassifierEvenOdd::ClassifierEvenOdd(ClassifierEvenOddSettings classifier_settings) {
      //callback_ID = RC::RStr("ClassifierEvenOdd_") + RC::RStr(classifier_settings);
    }

    /// Handler that actually does the classification and reports the result with a callback
    /** @param data The input data to the classifier
     */
    void ClassifierEvenOdd::Classifier_Handler(RC::APtr<const RC::Data1D<double>>& data) {
      RC_DEBOUT(RC::RStr("Classifier_Handler\n\n"));
      if ( ! callback.IsSet() ) {
        return;
      }

      // TODO: Split the wavelet data into 2 threads for classification
      bool result = F2I((*data)[0]) % 2;
      callback(result);

      for (size_t i=0; i<data_callbacks.size(); i++) {
        data_callbacks[i].callback(result);
      }
    }
}
