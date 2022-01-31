#include "ClassifierLogReg.h"
#include "FeatureWeights.h"
#include "Handler.h"

#define F2I(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

namespace CML {
  /// Constuctor which sets the sampling rate
  /** @param sampling_rate The sampling rate of the incoming data
   */
  ClassifierLogReg::ClassifierLogReg(RC::Ptr<Handler> hndl,
      ClassifierLogRegSettings classifier_settings,
      RC::APtr<const FeatureWeights> weights)
    : Classifier(hndl, weights){
    //callback_ID = RC::RStr("ClassifierLogReg_") + RC::RStr(classifier_settings);
  }

  /// Handler that actually does the classification and reports the result with a callback
  /** @param data The input data to the classifier
   *  @return The classifier result
   */
  double ClassifierLogReg::Classification(RC::APtr<const EEGPowers>& data) {
    RC_DEBOUT(RC::RStr("Classification\n\n"));
    auto& intercept = weights->intercept;
    auto& coef = weights->coef;
    auto& datar = data->data;

    if ( (datar.size1() != coef.size1()) ||
         (datar.size2() != coef.size2()) ||
         (datar.size3() != 1) ) {
      Throw_RC_Error("Classification data and coefficient dimensions do not "
          "match.");
    }

    double logodds = intercept;
    for (size_t f=0; f<coef.size1(); f++) {
      auto& dataf = datar[f];
      auto& coeff = coef[f];
      for (size_t c=0; c<coef.size2(); c++) {
        logodds += dataf[c][0]*coeff[c];
      }
    }


    double prob = 1 / (1 + std::exp(-logodds));

    return prob;
  }
}
