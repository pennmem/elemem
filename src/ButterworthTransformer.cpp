#include "ButterworthTransformer.h"
#include "MorletWaveletTransformMP.h"
#include "DSPFilters/Dsp.h"

namespace CML {
  ButterworthTransformer::ButterworthTransformer() {}

  void ButterworthTransformer::Setup(const ButterworthSettings& butterworth_settings) {
    but_set = butterworth_settings;
  }

  RC::APtr<EEGDataDouble> ButterworthTransformer::Filter(
      RC::APtr<const EEGDataDouble>& data) {

    Dsp::FilterDesign<Dsp::Butterworth::Design::BandStop<4>, 1, // MUST BE 1!
      Dsp::DirectFormII> f;

    size_t sample_len = data->sample_len;
    auto out_data = RC::MakeAPtr<EEGDataDouble>(*data);
    auto& outr = out_data->data;

    for (auto freq_band : but_set.frequency_bands) {
      double center_freq = (freq_band[0] + freq_band[1])/2.0;
      double bandwidth = freq_band[1] - freq_band[0];
      Dsp::Params params;
      params[0] = but_set.sampling_rate;
      params[1] = 4;    // order
      params[2] = center_freq;
      params[3] = bandwidth;
      f.setParams(params);

      for (size_t c=0; c<outr.size(); c++) {
        auto p = outr[c].Raw();  // Requires 1 for second f template parameter.
        f.process_bidir(sample_len, &p);  // bidir does reset internally.
      }
    }

    return out_data;
  }
}

