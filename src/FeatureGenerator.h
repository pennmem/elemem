#ifndef FEATUREGENERATOR_H
#define FEATUREGENERATOR_H

#include "EEGData.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"

namespace CML {
  using EEGCallback = RCqt::TaskCaller<RC::APtr<const EEGData>>;
  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const RC::Data1D<double>>>;

  class FeatureGenerator : public RCqt::WorkerThread {
    public:
    virtual ~FeatureGenerator() {}

    EEGCallback Process =
      TaskHandler(FeatureGenerator::Process_Handler);
      
    RCqt::TaskCaller<const FeatureCallback> SetCallback =
      TaskHandler(FeatureGenerator::SetCallback_Handler);


    protected:
    virtual void Process_Handler(RC::APtr<const EEGData>&) = 0;
    void SetCallback_Handler(const FeatureCallback &new_callback);

    FeatureCallback callback;
  };
}

#endif // FEATUREGENERATOR_H

