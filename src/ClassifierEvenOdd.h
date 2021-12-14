#ifndef CLASSIFIEREVENODD_H
#define CLASSIFIEREVENODD_H

#include "Classifier.h"

namespace CML {
  class ClassifierEvenOddSettings {
    int temp_variable = 0;
  };

  class ClassifierEvenOdd : public Classifier {
    public:
    ClassifierEvenOdd(RC::Ptr<Handler> hndl, ClassifierEvenOddSettings classifierSettings);
    

    protected:
    double Classification(RC::APtr<const RC::Data1D<double>>& data);

  };
}

#endif // CLASSIFIEREVENODD_H
