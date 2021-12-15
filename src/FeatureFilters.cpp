#include "FeatureFilters.h"


namespace CML {
  FeatureFilters::FeatureFilters(MorletSettings morlet_settings, ButterworthSettings butterworth_settings) {
    butterworth_transformer.Setup(butterworth_settings);
    morlet_transformer.Setup(morlet_settings);
  }

  RC::APtr<const EEGData> FeatureFilters::BipolarReference(RC::APtr<const EEGData>& data) {
    // TODO: JPB: Impl BipolarReference
    return data;
  }

  RC::APtr<const EEGData> FeatureFilters::MirrorEnds(RC::APtr<const EEGData>& data, size_t duration_ms) {
    // TODO: JPB: Impl MirrorEnds
    return data;
  }
  RC::APtr<const EEGData> FeatureFilters::Log10Transform(RC::APtr<const EEGData>& data) {
    // TODO: JPB: Impl Log10Transform
    return data;
  }
  RC::APtr<const EEGData> FeatureFilters::AvgOverTime(RC::APtr<const EEGData>& data) {
    // TODO: JPB: Impl AvgOverTime
    return data;
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
