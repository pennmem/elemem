#include "ClassifierEvenOdd.h"

#define F2I(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

namespace CML {
  /// Constuctor which sets the sampling rate
  /** @param sampling_rate The sampling rate of the incoming data
   */
  ClassifierEvenOdd::ClassifierEvenOdd(RC::Ptr<Handler> hndl, ClassifierEvenOddSettings classifier_settings)
    : Classifier(hndl){
    //callback_ID = RC::RStr("ClassifierEvenOdd_") + RC::RStr(classifier_settings);
  }

  /// Handler that actually does the classification and reports the result with a callback
  /** @param data The input data to the classifier
   *  @return The classifier result
   */
  double ClassifierEvenOdd::Classification(RC::APtr<const RC::Data1D<double>>& data) {
    //RC_DEBOUT(RC::RStr("Classification\n\n"));

    return F2I((*data)[0]) % 2;
  }
}
