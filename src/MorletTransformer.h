#ifndef MORLETTRANSFORMER_H
#define MORLETTRANSFORMER_H

#include <cstdint>
#include <complex>
#include "ChannelConf.h"
#include "EEGData.h"
#include "EEGPowers.h"
#include "RC/Data1D.h"
#include "RC/APtr.h"

#include "MorletWaveletTransformMP.h"
// TODO: JPB: (refactor) Move include to cpp file and use
//class MorletWaveletTransformMP;

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

  class MorletTransformer {
    public:
    MorletTransformer();

    void Setup(const MorletSettings& morlet_settings);
    double CalcAvgMirroringDurationMs();
    RC::APtr<EEGPowers> Filter(RC::APtr<const EEGData>& data);

    protected:
    MorletSettings mor_set;
    RC::APtr<MorletWaveletTransformMP> mt;

    // Sizes freqs*chans*events, freqs outer, events inner.
    RC::Data1D<double> pow_arr;
    RC::Data1D<double> phase_arr;
    RC::Data1D<std::complex<double>> complex_arr;

    double min_freq;
  };
}

#endif // MORLETTRANSFORMER_H

