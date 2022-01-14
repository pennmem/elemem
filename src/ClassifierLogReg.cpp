#include "ClassifierLogReg.h"

#define F2I(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

namespace CML {
  /// Constuctor which sets the sampling rate
  /** @param sampling_rate The sampling rate of the incoming data
   */
  ClassifierLogReg::ClassifierLogReg(RC::Ptr<Handler> hndl, ClassifierLogRegSettings classifier_settings)
    : Classifier(hndl){
    //callback_ID = RC::RStr("ClassifierLogReg_") + RC::RStr(classifier_settings);
  }

  /// Handler that actually does the classification and reports the result with a callback
  /** @param data The input data to the classifier
   *  @return The classifier result
   */
  double ClassifierLogReg::Classification(RC::APtr<const EEGPowers>& data) {
    //RC_DEBOUT(RC::RStr("Classification\n\n"));

    // TODO: JPB: (need) Impl Classification for FR5
    return 0;
  }
}
