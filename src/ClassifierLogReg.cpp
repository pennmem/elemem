#include "ClassifierLogReg.h"
#include "FeatureWeights.h"
#include "Handler.h"

#define F2I(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

namespace CML {
  /// Constuctor which sets the sampling rate
  /** @param sampling_rate The sampling rate of the incoming data
   */
  ClassifierLogReg::ClassifierLogReg(RC::Ptr<Handler> hndl,
      ClassifierLogRegSettings /* classifier_settings*/,  // TODO: JPB: Do we need this?
      RC::APtr<const FeatureWeights> weights)
    : Classifier(hndl, weights){
    //callback_ID = RC::RStr("ClassifierLogReg_") + RC::RStr(classifier_settings);
  }

  /// Handler that actually does the classification and reports the result with a callback
  /** @param data The input data to the classifier
   *  @return The classifier result
   */
  double ClassifierLogReg::Classification(RC::APtr<const EEGPowers>& data) {
    auto& intercept = weights->intercept;
    auto& coef = weights->coef;
    auto& datar = data->data;

    size_t freqlen = datar.size3();
    size_t chanlen = datar.size2();
    size_t eventlen = datar.size1();

    if ( (eventlen != 1) ||
         (chanlen != coef.size1()) ||
         (freqlen != coef.size2()) ) {
      Throw_RC_Error((RC::RStr("Classification data len (") + freqlen + ", " + chanlen + ", " + eventlen + ") " + 
                               "and coefficient dimensions (" + coef.size2() + ", " + coef.size1() + ", " + 1 + ") do not match.").c_str());
    }

    double logodds = intercept;
    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        logodds += datar[i][j][0] * coef[i][j];
      }
    }

    double prob = 1 / (1 + std::exp(-logodds));

    return prob;
  }
}
