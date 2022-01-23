#include "ButterworthTransformer.h"
#include "MorletWaveletTransformMP.h"

namespace CML {
  ButterworthTransformer::ButterworthTransformer() {}

  void ButterworthTransformer::Setup(const ButterworthSettings& butterworth_settings) {
    // TODO: JPB: (feature) Impl Butterworth Setup
    but_set = butterworth_settings;
  }

  RC::APtr<EEGData> ButterworthTransformer::Filter(RC::APtr<const EEGData>& data, double freq) {
    // TODO: JPB: (feature) Impl Butterworth Filter
    RC::APtr<EEGData> out_data = new EEGData(data->sampling_rate);
    return out_data;
  } 
  
  RC::APtr<EEGData> ButterworthTransformer::Filter(RC::APtr<const EEGData>& data, double high_freq, double low_freq) {
    // TODO: JPB: (feature) Impl Butterworth Filter
    RC::APtr<EEGData> out_data = new EEGData(data->sampling_rate);
    return out_data;
  } 
}

