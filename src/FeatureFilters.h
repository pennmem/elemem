#ifndef FEATUREFILTERS_H
#define FEATUREFILTERS_H

#include <complex>
#include "EEGData.h"
#include "TaskClassifierSettings.h"
#include "MorletTransformer.h"
#include "ButterworthTransformer.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"
#include "ChannelConf.h"

namespace CML {
  using TaskClassifierCallback = RCqt::TaskCaller<RC::APtr<const EEGData>, const TaskClassifierSettings>;
  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const RC::Data1D<double>>, const TaskClassifierSettings>;

  class FeatureFilters : public RCqt::WorkerThread {
    public:
    FeatureFilters(ButterworthSettings butterworth_settings, MorletSettings morlet_settings);

    TaskClassifierCallback Process =
      TaskHandler(FeatureFilters::Process_Handler);
      
    RCqt::TaskCaller<const FeatureCallback> SetCallback =
      TaskHandler(FeatureFilters::SetCallback_Handler);

    RC::APtr<const EEGData> BipolarReference(RC::APtr<const EEGData>& data);
    RC::APtr<const EEGData> MirrorEnds(RC::APtr<const EEGData>& data, size_t duration_ms);
    RC::APtr<const EEGData> Log10Transform(RC::APtr<const EEGData>& data);
    RC::APtr<const EEGData> AvgOverTime(RC::APtr<const EEGData>& data);


    protected:
    void Process_Handler(RC::APtr<const EEGData>&, const TaskClassifierSettings&);
    void SetCallback_Handler(const FeatureCallback &new_callback);
    
    MorletTransformer morlet_transformer;
    ButterworthTransformer butterworth_transformer;

    FeatureCallback callback;
  };
}

#endif // FEATUREFILTERS_H

