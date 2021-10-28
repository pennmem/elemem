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
   *  @return The classifier result
   */
  bool ClassifierEvenOdd::Classification(RC::APtr<const RC::Data1D<double>>& data) {
    RC_DEBOUT(RC::RStr("Classification\n\n"));

    // TODO: Split the wavelet data into 2 threads for classification
    return F2I((*data)[0]) % 2;
  }
}
