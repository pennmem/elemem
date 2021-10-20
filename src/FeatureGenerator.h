#ifndef FEATUREGENERATOR_H
#define FEATUREGENERATOR_H

#include "EEGData.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"

namespace CML {
  class FeatureGenerator : public RCqt::WorkerThread {
    public:
    virtual ~FeatureGenerator() {}

    RCqt::TaskCaller<RC::APtr<const EEGData>> Process =
      TaskHandler(FeatureGenerator::Process_Handler);

    using EEGCallback = RC::Caller<void, RC::APtr<const EEGData>>;
    EEGCallback Temp;

    protected:
    virtual void Process_Handler(RC::APtr<const EEGData>&) = 0;
  };
}

#endif // FEATUREGENERATOR_H

