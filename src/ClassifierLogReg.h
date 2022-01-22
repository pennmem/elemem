#ifndef CLASSIFIERLOGREG_H
#define CLASSIFIERLOGREG_H

#include "Classifier.h"

namespace CML {
  class ClassifierLogRegSettings {
    int temp_variable = 0;
  };

  class ClassifierLogReg : public Classifier {
    public:
    ClassifierLogReg(RC::Ptr<Handler> hndl,
        ClassifierLogRegSettings classifierSettings,
        RC::APtr<const FeatureWeights> weights);


    protected:
    double Classification(RC::APtr<const EEGPowers>& data);

  };
}

#endif // CLASSIFIERLOGREG_H
