#include "FeatureFilters.h"
#include <cmath>


namespace CML {
  // TODO: JPB: (refactor) Make this take const refs
  FeatureFilters::FeatureFilters(RC::Data1D<BipolarPair> bipolar_reference_channels,
      ButterworthSettings butterworth_settings, MorletSettings morlet_settings,
      NormalizePowersSettings np_set) 
	  : bipolar_reference_channels(bipolar_reference_channels),
	  normalize_powers(np_set.num_events, np_set.num_chans, np_set.num_freqs) {
    butterworth_transformer.Setup(butterworth_settings);
    morlet_transformer.Setup(morlet_settings);
  }

  /// Converts an EEGData of electrode channels into EEGData of bipolar pair channels
  /** @param EEGData of electrode channels
    * @param List of bipolar pairs
    * @return EEGData of bipolar pair channels
    */
  RC::APtr<EEGData> FeatureFilters::BipolarReference(RC::APtr<const EEGData>& in_data, RC::Data1D<BipolarPair> bipolar_reference_channels) {
    RC::APtr<EEGData> out_data = new EEGData(in_data->sampling_rate);
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    size_t chanlen = bipolar_reference_channels.size();
    out_datar.Resize(chanlen);

    RC_ForIndex(i, out_datar) { // Iterate over channels
      uint8_t pos = bipolar_reference_channels[i].pos;
      uint8_t neg = bipolar_reference_channels[i].neg;

      if (pos >= in_datar.size()) {
        Throw_RC_Error(("Positive channel " + RC::RStr(pos) +
              " is not a valid channel. The number of channels available is " +
              RC::RStr(in_datar.size())).c_str());
      } else if (neg >= in_datar.size()) {
        Throw_RC_Error(("Negative channel " + RC::RStr(neg) +
              " is not a valid channel. The number of channels available is " +
              RC::RStr(in_datar.size())).c_str());
      } else if (in_datar[pos].size() != in_datar[neg].size()) {
        Throw_RC_Error(("Size of positive channel " + RC::RStr(pos) +
              " (" + RC::RStr(in_datar[pos].size()) + ") " +
              "and size of negitive channel " + RC::RStr(neg) +
              " (" + RC::RStr(in_datar[neg].size()) + ") " +
              "are different").c_str());
      }

      auto& out_events = out_datar[i];
      size_t eventlen = in_datar[pos].size();
      out_events.Resize(eventlen);

      RC_ForIndex(j, out_events) {
        out_events[j] = in_datar[pos][j] - in_datar[neg][j];
      }
    }

    return out_data;
  }

  /// Mirrors both ends of the EEGData for the provided number of seconds
  /** Note that when mirroring, you do not include the items bein mirror over
    * Ex: 0, 1, 2, 3 with a mirroring of 2 samples becomes 2, 1, 0, 1, 2, 3, 2, 1
    * @param The data to be mirrored
    * @param Duration to mirror each side for
    * @return The mirrored EEGData
    */
  RC::APtr<EEGData> FeatureFilters::MirrorEnds(RC::APtr<const EEGData>& in_data, size_t mirrored_duration_ms) {
    size_t num_mirrored_samples = mirrored_duration_ms * in_data->sampling_rate / 1000;
    
    RC::APtr<EEGData> out_data = new EEGData(in_data->sampling_rate);
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    size_t chanlen = in_datar.size();
    out_datar.Resize(chanlen);

    RC_ForIndex(i, in_datar) { // Iterate over channels
      auto& in_events = in_datar[i];
      auto& out_events = out_datar[i];
      size_t in_eventlen = in_events.size();
      size_t out_eventlen = in_eventlen + num_mirrored_samples * 2;
      out_events.Resize(out_eventlen);

      if (num_mirrored_samples >= in_eventlen) {
        Throw_RC_Error(("Number of events to mirror "
              "(" + RC::RStr(num_mirrored_samples) + ") " +
              "is greater than number of samples in channel " + RC::RStr(i) +
              "(" + RC::RStr(in_eventlen) + ")").c_str());
      }

      // Copy starting samples in reverse, skipping the first item
      RC_ForRange(i, 0, num_mirrored_samples) {
        out_events[i] = in_events[num_mirrored_samples-i];
      }

      // Copy middle samples verbatim
      out_events.CopyAt(num_mirrored_samples, in_events);

      // Copy ending samples in reverse, skipping the last item
      size_t start_pos = num_mirrored_samples + in_eventlen;
      RC_ForRange(i, 0, num_mirrored_samples) {
        out_events[start_pos+i] = in_events[in_eventlen-i-2];
      }
    }

    return out_data;
  }

  /// A log (base 10) transform of all powers in EEGPowers
  /** Note: A power could be zero due to constant signal across two electrodes that are part of bipolar pair
    * @param The data to be log transformed
    * @param Minimum power clamp (just before taking log) to avoid log singularity in case we get zero power
    * @return The log transformed data
    */
  RC::APtr<EEGPowers> FeatureFilters::Log10Transform(RC::APtr<const EEGPowers>& in_data, double min_power_clamp) {
    auto& in_datar = in_data->data;
    size_t freqlen = in_datar.size3();
    size_t chanlen = in_datar.size2();
    size_t eventlen = in_datar.size1();

    RC::APtr<EEGPowers> out_data = new EEGPowers(in_data->sampling_rate, eventlen, chanlen, freqlen);
    auto& out_datar = out_data->data;

    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        RC_ForRange(k, 0, eventlen) { // Iterate over events
          double power = std::max(min_power_clamp, in_datar[i][j][k]);
          out_datar[i][j][k] = log10(power);
        }
      }   
    } 

    return out_data;
  }

  /// Average the EEGPowers over the time dimension (most inner dimension)
  /** Note: The time dimension will have a length of 1 in the returned data
    * @param The data to be averaged
    * @return The time averaged EEGPowers
    */
  RC::APtr<EEGPowers> FeatureFilters::AvgOverTime(RC::APtr<const EEGPowers>& in_data) {
    auto& in_datar = in_data->data;
    size_t freqlen = in_datar.size3();
    size_t chanlen = in_datar.size2();
    size_t in_eventlen = in_datar.size1();
    size_t out_eventlen = 1;

    RC::APtr<EEGPowers> out_data = new EEGPowers(in_data->sampling_rate, out_eventlen, chanlen, freqlen);
    auto& out_datar = out_data->data;

    auto accum_event = [](double sum, double val) { return std::move(sum) + val; };
    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        auto& in_events = in_datar[i][j];
        auto& out_events = out_datar[i][j];
        out_events[0] = std::accumulate(in_events.begin(), in_events.end(),
                          0, accum_event) / static_cast<double>(in_eventlen);
      }
    }

    return out_data;
  }

  void FeatureFilters::Process_Handler(RC::APtr<const EEGData>& data, const TaskClassifierSettings& task_classifier_settings) {
    //RC_DEBOUT(RC::RStr("FeatureFilters_Handler\n\n"));
    if (!callback.IsSet())
      return;

    auto bipolar_ref_data = BipolarReference(data, bipolar_reference_channels).ExtractConst();
    auto mirrored_data = MirrorEnds(bipolar_ref_data, 1000).ExtractConst();
    auto morlet_data = morlet_transformer.Filter(mirrored_data).ExtractConst();
    auto log_data = Log10Transform(morlet_data, log_min_power_clamp).ExtractConst();
    auto avg_data = AvgOverTime(log_data).ExtractConst();

    // Normalize Powers
    switch (task_classifier_settings.cl_type) {
      case ClassificationType::NORMALIZE:
        normalize_powers.Update(avg_data);
        break;
      case ClassificationType::STIM:
      case ClassificationType::SHAM:
      {
        auto norm_data = normalize_powers.ZScore(avg_data).ExtractConst();
        callback(norm_data, task_classifier_settings);
        break;
      }
      default: Throw_RC_Error("Invalid classification type received.");
    } 
  }

  /// Handler that sets the callback on the feature generator results
  /** @param The callback on the classifier results
   */
  void FeatureFilters::SetCallback_Handler(const FeatureCallback &new_callback) {
    callback = new_callback;  
  }
}
