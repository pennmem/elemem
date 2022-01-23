#ifndef BUTTERWORTHTRANSFORMER_H
#define BUTTERWORTHTRANSFORMER_H

#include <cstdint>
#include <complex>
#include "ChannelConf.h"
#include "EEGData.h"
#include "RC/Data1D.h"
#include "RC/APtr.h"

//#include "ButterworthTransformMP.h"
// TODO: JPB: (refactor) Move include to cpp file and use
//class ButterworthTransformMP;

namespace CML {
  class ButterworthSettings {
    public:
    RC::Data1D<BipolarPair> channels;
    size_t sampling_rate = 1000;
    uint32_t cpus = 2;
  };

  class ButterworthTransformer {
    public:
    ButterworthTransformer();

    void Setup(const ButterworthSettings& butterworth_settings);
    RC::APtr<EEGData> Filter(RC::APtr<const EEGData>& data, double freq);
    RC::APtr<EEGData> Filter(RC::APtr<const EEGData>& data, double high_freq, double low_freq);

    protected:
    ButterworthSettings but_set;
    //RC::APtr<ButerworthTransformMP> bt;
  };
}

#endif // BUTTERWORTHRANSFORMER_H

