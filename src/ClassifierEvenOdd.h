#ifndef CLASSIFIER_EVEN_ODD_H
#define CLASSIFIER_EVEN_ODD_H

#include "Classifier.h"

namespace CML {
  class ClassifierEvenOddSettings {
    int sampling_rate = 0;
  };

  class ClassifierEvenOdd : public Classifier {
    public:
    ClassifierEvenOdd(ClassifierEvenOddSettings classifierSettings); 
    

    protected:
    double Classification(RC::APtr<const RC::Data1D<double>>& data);
  };
}

#endif // CLASSIFIER_EVEN_ODD_H
