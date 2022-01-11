#include "FeatureFilters.h"
#include <cmath>


namespace CML {
  // TODO: JPB: (refactor) Make this take const refs
  FeatureFilters::FeatureFilters(ButterworthSettings butterworth_settings, MorletSettings morlet_settings,
      RC::Data1D<BipolarPair> bipolar_reference_channels) : bipolar_reference_channels(bipolar_reference_channels){
    butterworth_transformer.Setup(butterworth_settings);
    morlet_transformer.Setup(morlet_settings);
  }

  RC::APtr<const EEGData> FeatureFilters::BipolarReference(RC::APtr<const EEGData>& in_data) {
    // TODO: JPB: (need) Test BipolarReference
    RC::APtr<EEGData> out_data = new EEGData(in_data->sampling_rate);
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    size_t chanlen = bipolar_reference_channels.size();
    out_datar.Resize(chanlen);

    RC_ForIndex(i, out_datar) { // Iterate over channels
      auto& in_events = in_datar[i];
      auto& out_events = out_datar[i];
      size_t eventlen = in_events.size();
      out_events.Resize(eventlen);
      uint8_t pos = bipolar_reference_channels[i].pos;
      uint8_t neg = bipolar_reference_channels[i].neg;

      RC_ForIndex(j, out_events) {
        out_events[j] = in_datar[pos][j] - in_datar[neg][j];
      }
    }

    return out_data.ExtractConst();
  }

  RC::APtr<const EEGData> FeatureFilters::MirrorEnds(RC::APtr<const EEGData>& in_data, size_t mirrored_duration_ms) {
    // TODO: JPB: (need) Test MirrorEnds
    RC::APtr<EEGData> out_data = new EEGData(in_data->sampling_rate);
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    size_t chanlen = in_datar.size();
    size_t num_mirrored_samples = mirrored_duration_ms * in_data->sampling_rate / 1000;
    out_datar.Resize(chanlen);

    RC_ForIndex(i, in_datar) { // Iterate over channels
      auto& in_events = in_datar[i];
      auto& out_events = out_datar[i];
      size_t in_eventlen = in_events.size();
      size_t out_eventlen = in_eventlen + num_mirrored_samples * 2;
      out_events.Resize(out_eventlen);

      RC_ForRange(i, 0, num_mirrored_samples) {
        out_events[i] = in_events[num_mirrored_samples-1-i];
      }
      out_events.CopyAt(num_mirrored_samples, in_events);
      out_events.CopyAt(num_mirrored_samples + in_eventlen, in_events, 
                        in_eventlen - num_mirrored_samples, num_mirrored_samples);
    }

    return out_data.ExtractConst();
  }

  RC::APtr<const EEGPowers> FeatureFilters::Log10Transform(RC::APtr<const EEGPowers>& in_data) {
    // TODO: JPB: (need) Test Log10Transform
    auto& in_datar = in_data->data;
    size_t freqlen = in_datar.size1();
    size_t chanlen = in_datar.size2();
    size_t eventlen = in_datar.size3();

    RC::APtr<EEGPowers> out_data = new EEGPowers(in_data->sampling_rate, freqlen, chanlen, eventlen);
    auto& out_datar = out_data->data;

    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        RC_ForRange(k, 0, eventlen) { // Iterate over events
          out_datar[i][j][k] = log10(in_datar[i][j][k]);
        }
      }   
    } 

    return out_data.ExtractConst();
  }

  RC::APtr<const EEGPowers> FeatureFilters::AvgOverTime(RC::APtr<const EEGPowers>& in_data) {
    // TODO: JPB: (need) Test AvgOverTime
    auto& in_datar = in_data->data;
    size_t freqlen = in_datar.size1();
    size_t chanlen = in_datar.size2();
    size_t in_eventlen = in_datar.size3();
    size_t out_eventlen = 1;

    RC::APtr<EEGPowers> out_data = new EEGPowers(in_data->sampling_rate, freqlen, chanlen, out_eventlen);
    auto& out_datar = out_data->data;

    auto accum_event = [](u32 sum, size_t val) { return std::move(sum) + val; };
    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        auto& in_events = in_datar[i][j];
        auto& out_events = out_datar[i][j];
        out_events[0] = std::accumulate(in_events.begin(), in_events.end(),
                          0, accum_event) / in_eventlen;
      }   
    }

    return out_data.ExtractConst();
  }

  void FeatureFilters::Process_Handler(RC::APtr<const EEGData>& data, const TaskClassifierSettings& task_classifier_settings) {
    //RC_DEBOUT(RC::RStr("FeatureFilters_Handler\n\n"));
    if ( ! callback.IsSet() ) {
      return;
    }

    auto bipolar_ref_data = BipolarReference(data);
    auto mirrored_data = MirrorEnds(bipolar_ref_data, 1000);
    auto no_line_noise_data = butterworth_transformer.Filter(mirrored_data, 58, 62);
    auto no_drift_data = butterworth_transformer.Filter(no_line_noise_data, 0.5);
    auto morlet_data = morlet_transformer.Filter(no_drift_data);
    auto log_data = Log10Transform(morlet_data);
    auto avg_data = AvgOverTime(log_data);

    RC::Data1D<double> temp1{0};
    auto temp2 = RC::MakeAPtr<const RC::Data1D<double>>(temp1);
    callback(temp2, task_classifier_settings);
  }

  /// Handler that sets the callback on the feature generator results
  /** @param The callback on the classifier results
   */
  void FeatureFilters::SetCallback_Handler(const FeatureCallback &new_callback) {
    callback = new_callback;  
  }
}
