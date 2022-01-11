#include "MorletTransformer.h"
#include "MorletWaveletTransformMP.h"

namespace CML {
  MorletTransformer::MorletTransformer() = default;

  void MorletTransformer::Setup(const MorletSettings& morlet_settings) {
    mor_set = morlet_settings;

    if (mor_set.channels.size() < 1 || mor_set.frequencies.size() < 1) {
      Throw_RC_Error("Must configure at least one channel and one frequency "
          "for classification.");
    }   

    // TODO: JPB: (need) Remove this early return to enable Setup() (this causes an exception)
    return;

    mt = new MorletWaveletTransformMP(mor_set.cpus);

    mt->set_output_type(OutputType::POWER);

    mt->initialize_signal_props(mor_set.sampling_rate);
    mt->initialize_wavelet_props(mor_set.cycle_count,
        mor_set.frequencies.Raw(), mor_set.frequencies.size(),
        mor_set.complete);

    mt->prepare_run();
  }

  RC::APtr<const EEGPowers> MorletTransformer::Filter(RC::APtr<const EEGData>& data) {
    // TODO: JPB: (need) Remove this early return to enable Filter()
    RC::APtr<EEGPowers> temp = new EEGPowers(data->sampling_rate);
    return temp.ExtractConst();

    auto& datar = data->data;
    size_t freqlen = mor_set.frequencies.size();
    size_t chanlen = mor_set.channels.size();
    size_t eventlen = datar[0].size();

    // The out data dimensions from outer to inner are: frequency, channel, time/event
    size_t in_flat_size = chanlen * eventlen;
    size_t out_flat_size = freqlen * chanlen * eventlen;
    pow_arr.Resize(out_flat_size);
    phase_arr.Resize(out_flat_size);
    complex_arr.Resize(out_flat_size);

    mt->set_wavelet_complex_array(complex_arr.Raw(), chanlen, eventlen);

    // Flatten Data (and convert to double)
    RC::Data1D<double> flat_data(in_flat_size);
    RC_ForIndex(i, datar) { // Iterate over channels
      size_t flat_pos = i * eventlen; 
      flat_data.CopyAt(flat_pos, datar[i]);
    }

    mt->set_signal_array(flat_data.Raw(), chanlen, eventlen);
    mt->compute_wavelets_threads();

    // TODO: JPB: Add way to pre-allocate data size
    // UnflattenData
    RC::APtr<EEGPowers> powers = new EEGPowers(data->sampling_rate);
    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        size_t flat_pos = (i * chanlen * eventlen) + (j * eventlen); 
        powers->data[i][j].CopyFrom(pow_arr, flat_pos, chanlen);
      }
    }

    return powers.ExtractConst();
  }
}

