#include "MorletTransformer.h"
#include "MorletWaveletTransformMP.h"
#include "RC/RStr.h"

namespace CML {
  MorletTransformer::MorletTransformer() = default;

  void MorletTransformer::Setup(const MorletSettings& morlet_settings) {
    mor_set = morlet_settings;

    if (mor_set.channels.size() < 1 || mor_set.frequencies.size() < 1) {
      Throw_RC_Error("Must configure at least one channel and one frequency "
          "for classification.");
    } else if (mor_set.num_events < 1) {
      Throw_RC_Error("Must set the number of events per channel to be "
          "at least 1");
    }

    mt = new MorletWaveletTransformMP(mor_set.cpus);

    mt->set_output_type(OutputType::POWER);

    mt->initialize_signal_props(mor_set.sampling_rate);
    mt->initialize_wavelet_props(mor_set.cycle_count,
        mor_set.frequencies.Raw(), mor_set.frequencies.size(),
        mor_set.complete);

    // TODO: JPB: (refactor) Change ptsa to include num_events in initialize_signal_props
    mt->set_signal_array(nullptr, mor_set.channels.size(), mor_set.num_events);

    mt->prepare_run();
  }

  RC::APtr<const EEGPowers> MorletTransformer::Filter(RC::APtr<const EEGData>& data) {
    auto& datar = data->data;

    size_t freqlen = mor_set.frequencies.size();
    size_t chanlen = mor_set.channels.size();
    size_t eventlen = mor_set.num_events;

    // The in data dimensions from outer to inner are: channel->time/event
    // The out data dimensions from outer to inner are: frequency->channel->time/event
    size_t in_flat_size = chanlen * eventlen;
    size_t out_flat_size = freqlen * chanlen * eventlen;
    pow_arr.Resize(out_flat_size);
    phase_arr.Resize(out_flat_size);
    complex_arr.Resize(out_flat_size); // TODO: (feature) This can likely be removed to reduce overhead

    mt->set_wavelet_pow_array(pow_arr.Raw(), chanlen, eventlen);
    mt->set_wavelet_phase_array(phase_arr.Raw(), chanlen, eventlen);
    mt->set_wavelet_complex_array(complex_arr.Raw(), chanlen, eventlen); // TODO: (feature) This can likely be removed to reduce overhead

    // Flatten Data (and convert to double)
    RC::Data1D<double> flat_data(in_flat_size);
    RC_ForIndex(i, datar) { // Iterate over channels
      size_t flat_pos = i * eventlen; 
      flat_data.CopyAt(flat_pos, datar[i]);
    }

    mt->set_signal_array(flat_data.Raw(), chanlen, eventlen);
    mt->compute_wavelets_threads();

    // UnflattenData
    // The implicit pow_arr dimensions from outer to inner are: channel->frequency->time/event
    // This is just a part of the MorletWaveletTransformMP API in PTSA...
    // It is converted back to the standard frequency->channel->time/event when unflattened 
    RC::APtr<EEGPowers> powers = new EEGPowers(data->sampling_rate, eventlen, chanlen, freqlen);
    RC_ForRange(i, 0, chanlen) { // Iterate over channels
      RC_ForRange(j, 0, freqlen) { // Iterate over frequencies
        size_t flat_pos = (i * freqlen * eventlen) + (j * eventlen); 
        powers->data[j][i].CopyFrom(pow_arr, flat_pos, eventlen);
      }
    }

    return powers.ExtractConst();
  }
}

