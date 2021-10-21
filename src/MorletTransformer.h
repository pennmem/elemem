#ifndef MORLETTRANSFORMER_H
#define MORLETTRANSFORMER_H

#include <cstdint>
#include <complex>
#include "ChannelConf.h"
#include "FeatureGenerator.h"
#include "RC/Data1D.h"
#include "RC/Data1D.h"

class MorletWaveletTransformMP;

namespace CML {

  class MorletSettings {
    public:
    size_t cycle_count = 3;
    RC::Data1D<double> frequencies;
    RC::Data1D<BipolarPair> channels;
    size_t sampling_rate = 1000;
    uint32_t cpus = 2;
    bool complete = true;
  };

  using MorletCallback = RCqt::TaskCaller<RC::APtr<const RC::Data1D<double>>>;

  class MorletTransformer : public FeatureGenerator {
    public:
    MorletTransformer(MorletSettings morlet_settings);

    RCqt::TaskCaller<MorletCallback> SetCallback =
      TaskHandler(MorletTransformer::SetCallback_Handler);


    protected:
    void Process_Handler(RC::APtr<const EEGData>& data);
    void SetCallback_Handler(MorletCallback& new_callback);

    MorletSettings mor_set;
    RC::APtr<MorletWaveletTransformMP> mt;

    MorletCallback callback;

    // Sizes chans*freqs, chans outer, freqs inner.
    RC::Data1D<double> pow_arr;
    RC::Data1D<double> phase_arr;
    RC::Data1D<std::complex<double>> complex_arr;
  };
}

#endif // MORLETTRANSFORMER_H

