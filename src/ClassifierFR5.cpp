#include "ClassifierFR5.h"

#define F2I(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

namespace CML {
  /// Constuctor which sets the sampling rate
  /** @param sampling_rate The sampling rate of the incoming data
   */
  ClassifierFR5::ClassifierFR5(RC::Ptr<Handler> hndl, ClassifierFR5Settings classifier_settings)
    : Classifier(hndl){
    //callback_ID = RC::RStr("ClassifierFR5_") + RC::RStr(classifier_settings);
  }

  /// Handler that actually does the classification and reports the result with a callback
  /** @param data The input data to the classifier
   *  @return The classifier result
   */
  double ClassifierFR5::Classification(RC::APtr<const EEGPowers>& data) {
    //RC_DEBOUT(RC::RStr("Classification\n\n"));

    // TODO: JPB: (need) Impl Classification for FR5
    return 0;
  }
}
