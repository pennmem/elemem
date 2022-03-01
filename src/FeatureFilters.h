#ifndef FEATUREFILTERS_H
#define FEATUREFILTERS_H

#include <complex>
#include "EEGData.h"
#include "EEGPowers.h"
#include "TaskClassifierSettings.h"
#include "MorletTransformer.h"
#include "ButterworthTransformer.h"
#include "NormalizePowers.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"
#include "ChannelConf.h"


namespace CML {
  using TaskClassifierCallback = RCqt::TaskCaller<RC::APtr<const EEGDataRaw>, const TaskClassifierSettings>;
  using FeatureCallback = RCqt::TaskCaller<RC::APtr<const EEGPowers>, const TaskClassifierSettings>;

  struct BinnedData {
    BinnedData(size_t binned_sampling_rate, size_t binned_sample_len, size_t leftover_sampling_rate, size_t leftover_sample_len);
    RC::APtr<EEGDataRaw> out_data;
    RC::APtr<EEGDataRaw> leftover_data;
  };

  class FeatureFilters : public RCqt::WorkerThread {
    public:
    FeatureFilters(RC::Data1D<BipolarPair> bipolar_reference_channels,
	  ButterworthSettings butterworth_settings, MorletSettings morlet_settings,
      NormalizePowersSettings np_set);

    TaskClassifierCallback Process =
      TaskHandler(FeatureFilters::Process_Handler);
      
    RCqt::TaskCaller<const FeatureCallback> SetCallback =
      TaskHandler(FeatureFilters::SetCallback_Handler);

    static RC::APtr<BinnedData> BinData(RC::APtr<const EEGDataRaw> in_data, size_t new_sampling_rate);
    static RC::APtr<BinnedData> BinData(RC::APtr<const EEGDataRaw> rollover_data, RC::APtr<const EEGDataRaw> in_data, size_t new_sampling_rate);
    static RC::APtr<EEGDataRaw> BinDataAvgRollover(RC::APtr<const EEGDataRaw> in_data, size_t new_sampling_rate);
    static RC::APtr<EEGDataDouble> BipolarReference(RC::APtr<const EEGDataRaw>& in_data, RC::Data1D<BipolarPair> bipolar_reference_channels);
    static RC::APtr<RC::Data1D<bool>> FindArtifactChannels(RC::APtr<const EEGDataDouble>& in_data, size_t threshold, size_t order);
    static RC::APtr<EEGDataDouble> MirrorEnds(RC::APtr<const EEGDataDouble>& in_data, size_t duration_ms);
    static RC::APtr<EEGPowers> RemoveMirrorEnds(RC::APtr<const EEGPowers>& in_data, size_t mirrored_duration_ms);
    static RC::APtr<EEGPowers> Log10Transform(RC::APtr<const EEGPowers>& in_data, double epsilon);
    static RC::APtr<EEGPowers> AvgOverTime(RC::APtr<const EEGPowers>& in_data, bool ignore_inf_and_nan);

    // This is only public for testing purposes
    template<typename T>
    static RC::Data1D<T> Differentiate(const RC::Data1D<T>& in_data, size_t order);


    protected:
    void Process_Handler(RC::APtr<const EEGData>&, const TaskClassifierSettings&);
    void SetCallback_Handler(const FeatureCallback &new_callback);
    
    MorletTransformer morlet_transformer;
    ButterworthTransformer butterworth_transformer;
    RC::Data1D<BipolarPair> bipolar_reference_channels;
    NormalizePowers normalize_powers;

    FeatureCallback callback;

    // Minimum power clamp (just before taking log) to avoid log singularity in case we get zero power
    // A power could be zero due to constant signal across two electrodes that are part of bipolar pair
    const double log_min_power_clamp = 1e-16;
  };
}

#endif // FEATUREFILTERS_H

