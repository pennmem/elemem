#ifndef FEATUREFILTERS_H
#define FEATUREFILTERS_H

#include <complex>
#include "EEGData.h"
#include "TaskClassifierSettings.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"
#include "ChannelConf.h"

class MorletWaveletTransformMP;

namespace CML {
  using TaskClassifierCallback = RCqt::TaskCaller<RC::APtr<const EEGData>, const TaskClassifierSettings>;
  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const RC::Data1D<double>>, const TaskClassifierSettings>;

  class MorletSettings {
    public:
    size_t cycle_count = 3;
    RC::Data1D<double> frequencies;
    RC::Data1D<BipolarPair> channels;
    size_t sampling_rate = 1000;
    uint32_t cpus = 2;
    bool complete = true;
  };

  class MorletTransformer {
    public:
    MorletTransformer() = default;

    void Setup(const MorletSettings& morlet_settings);
    RC::APtr<const EEGData> Filter(RC::APtr<const EEGData>&);

    protected:
    MorletSettings mor_set;
    RC::APtr<MorletWaveletTransformMP> mt;

    // Sizes chans*freqs, chans outer, freqs inner.
    RC::Data1D<double> pow_arr;
    RC::Data1D<double> phase_arr;
    RC::Data1D<std::complex<double>> complex_arr;
  };

  class ButterworthSettings {
    public:
    RC::Data1D<double> frequencies;
    RC::Data1D<BipolarPair> channels;
    size_t sampling_rate = 1000;
    uint32_t cpus = 2;
    bool complete = true;
  };

  class ButterworthTransformer {
    public:
    ButterworthTransformer() = default;

    void Setup(const ButterworthSettings& butterworth_settings);
    RC::APtr<const EEGData> Filter(RC::APtr<const EEGData>& data, double freq);
    RC::APtr<const EEGData> Filter(RC::APtr<const EEGData>& data, double high_freq, double low_freq);

    protected:
    ButterworthSettings but_set;
    //RC::APtr<ButerworthTransformMP> bt;
  };

  class FeatureFilters : public RCqt::WorkerThread {
    public:
    FeatureFilters(MorletSettings morlet_settings);

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

