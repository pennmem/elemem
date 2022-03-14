#include "MorletTransformer.h"
#include "MorletWaveletTransformMP.h"
#include "RC/RStr.h"
#include <algorithm>

namespace CML {
  MorletTransformer::MorletTransformer() = default;

  void MorletTransformer::Setup(const MorletSettings& morlet_settings) {
    mor_set = morlet_settings;

    if (mor_set.channels.size() < 1 || mor_set.frequencies.size() < 1) {
      Throw_RC_Error("Must configure at least one channel and one frequency "
          "for classification.");
    }

    mt = RC::MakeAPtr<MorletWaveletTransformMP>(mor_set.cpus);

    mt->set_output_type(OutputType::POWER);

    mt->initialize_signal_props(mor_set.sampling_rate);
    mt->initialize_wavelet_props(mor_set.cycle_count,
        mor_set.frequencies.Raw(), mor_set.frequencies.size(),
        mor_set.complete);

    // TODO: JPB: (need) Make this a setting in mor_set that only changes when this setup is called
    //                   This will also require a new network packet for classifier setup (duration)
    //                   This will also require a new check in filter to make sure it is the right length
    // This is currently an optimization to make future prepare_run() calls take less time
    size_t temp_eventlen = 1750; // This magic number was chosen becuase the expected duration is 1000ms + 750ms of mirroring
    mt->set_signal_array(nullptr, mor_set.channels.size(), temp_eventlen);
    mt->prepare_run();
  }

  // This calculates the minimum statistical buffer duration for the MorletTransform,
  // based on the input duration
  double MorletTransformer::CalcAvgMirroringDurationMs() {
    if (mt == NULL) {
      Throw_RC_Error("MorletTransformer Setup() was not called before CalcBufferDurationMs() was called.");
    }

    double min_freq = *std::min_element(mor_set.frequencies.begin(), mor_set.frequencies.end());
    return 1.5 * 1000 * mor_set.cycle_count / 2 / min_freq;
  }

  RC::APtr<EEGPowers> MorletTransformer::Filter(RC::APtr<const EEGDataDouble>& data) {
    if (mt == NULL) {
      Throw_RC_Error("MorletTransformer Setup() was not called before Filter() was called.");
    }

    auto& datar = data->data;
    size_t freqlen = mor_set.frequencies.size();
    size_t chanlen = mor_set.channels.size();
    size_t eventlen = data->sample_len;

    if (chanlen != datar.size()) {
      Throw_RC_Error((RC::RStr("MorletSettings dimensions (") + chanlen + ", _" + ") " +
                     "and data dimensions (" + datar.size() + ", _" + ") " +
                     "do not match.").c_str());
    }

    // The in data dimensions from outer to inner are: channel->time/event
    // The out data dimensions from outer to inner are: frequency->channel->time/event
    size_t in_flat_size = chanlen * eventlen;
    size_t out_flat_size = freqlen * chanlen * eventlen;
    pow_arr.Resize(out_flat_size);
    phase_arr.Resize(out_flat_size);
    complex_arr.Resize(out_flat_size); // TODO: (feature)(optimization) This can likely be removed to reduce overhead

    mt->set_wavelet_pow_array(pow_arr.Raw(), chanlen, eventlen);
    mt->set_wavelet_phase_array(phase_arr.Raw(), chanlen, eventlen);
    mt->set_wavelet_complex_array(complex_arr.Raw(), chanlen, eventlen); // TODO: (feature)(optimization) This can likely be removed to reduce overhead

    // Flatten Data (and convert to double)
    RC::Data1D<double> flat_data(in_flat_size);
    flat_data.Zero();
    RC_ForIndex(i, datar) { // Iterate over channels
      size_t flat_pos = i * eventlen;
      flat_data.CopyAt(flat_pos, datar[i]);
    }

    // TODO: JPB: (feature)(optimization) Only prepare_run when the eventlen has changed
    mt->set_signal_array(flat_data.Raw(), chanlen, eventlen);
    mt->prepare_run(); // This must be run every time because the duration can change
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

    return powers;
  }
}

