#ifndef CLASSIFIERFR5_H
#define CLASSIFIERFR5_H

#include "Classifier.h"

namespace CML {
  class ClassifierFR5Settings {
    int temp_variable = 0;
  };

  class ClassifierFR5 : public Classifier {
    public:
    ClassifierFR5(RC::Ptr<Handler> hndl, ClassifierFR5Settings classifierSettings);
    

    protected:
    double Classification(RC::APtr<const EEGPowers>& data);

  };
}

#endif // CLASSIFIERFR5_H
