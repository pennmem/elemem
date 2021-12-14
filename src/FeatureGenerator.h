#ifndef FEATUREGENERATOR_H
#define FEATUREGENERATOR_H

#include "EEGData.h"
#include "TaskClassifierSettings.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"

namespace CML {
  using TaskClassifierCallback = RCqt::TaskCaller<RC::APtr<const EEGData>, const TaskClassifierSettings>;
  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const RC::Data1D<double>>, const TaskClassifierSettings>;

  class FeatureGenerator : public RCqt::WorkerThread {
    public:
    virtual ~FeatureGenerator() {}

    TaskClassifierCallback Process =
      TaskHandler(FeatureGenerator::Process_Handler);
      
    RCqt::TaskCaller<const FeatureCallback> SetCallback =
      TaskHandler(FeatureGenerator::SetCallback_Handler);


    protected:
    virtual void Process_Handler(RC::APtr<const EEGData>&, const TaskClassifierSettings&) = 0;
    void SetCallback_Handler(const FeatureCallback &new_callback);

    FeatureCallback callback;
  };
}

#endif // FEATUREGENERATOR_H

