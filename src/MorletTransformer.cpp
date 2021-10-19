#include "MorletTransformer.h"
#include "MorletWaveletTransformMP.h"


namespace CML {
  MorletTransformer::MorletTransformer(MorletSettings morlet_settings) {
    mor_set = std::move(morlet_settings);

    if (mor_set.channels.size() < 1 || mor_set.frequencies.size() < 1) {
      Throw_RC_Error("Must configure at least one channel and one frequency "
          "for classification.");
    }
    return; //JPB: TODO: Remove (this causes an exception)
    mt = new MorletWaveletTransformMP(mor_set.cpus);

    mt->set_output_type(OutputType::POWER);

    mt->initialize_signal_props(mor_set.sampling_rate);
    mt->initialize_wavelet_props(mor_set.cycle_count,
        mor_set.frequencies.Raw(), mor_set.frequencies.size(),
        mor_set.complete);

    mt->prepare_run();
  }


  void MorletTransformer::Process_Handler(RC::APtr<const EEGData>& data) {
    RC_DEBOUT(RC::RStr("\n\nAHHHHHHHHHHHH\n\n"));
    if ( ! callback.IsSet() ) {
      return;
    }

    auto& datar = data->data;
    size_t datalen = datar[mor_set.channels[0].pos].size();
    size_t chanlen = mor_set.channels.size();

    size_t flat_size = chanlen * datalen;
    pow_arr.Resize(flat_size);
    phase_arr.Resize(flat_size);
    complex_arr.Resize(flat_size);
    mt->set_wavelet_pow_array(pow_arr.Raw(), chanlen, datalen);
    mt->set_wavelet_phase_array(phase_arr.Raw(), chanlen, datalen);
    mt->set_wavelet_complex_array(complex_arr.Raw(), chanlen, datalen);

    // Calculate bipolar pair data as flat 1D array.
    RC::Data1D<double> flatdata(flat_size);
    for (size_t ci=0; ci < chanlen; ci++) {
      uint8_t pos = mor_set.channels[ci].pos;
      uint8_t neg = mor_set.channels[ci].neg;
      for (size_t ti=0; ti < datalen; ti++) {
        size_t fi = ci*chanlen + ti;
        flatdata[fi] = datar[pos][ti] - datar[neg][ti];
      }
    }

    mt->set_signal_array(flatdata.Raw(), chanlen, datalen);
    mt->compute_wavelets_threads();

    callback(RC::MakeAPtr<const RC::Data1D<double>>(pow_arr));
  }


  void MorletTransformer::SetCallback_Handler(MorletCallback& new_callback) {
    callback = new_callback;
  }
}

