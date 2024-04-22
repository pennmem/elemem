#ifndef BUTTERWORTHTRANSFORMER_H
#define BUTTERWORTHTRANSFORMER_H

#include <cstdint>
#include <complex>
#include "ChannelConf.h"
#include "EEGData.h"
#include "RC/Data1D.h"
#include "RC/Data2D.h"
#include "RC/APtr.h"


namespace CML {
  class ButterworthSettings {
    public:
    RC::Data1D<BipolarPair> channels;
    size_t sampling_rate = 1000;
    RC::Data2D<double> frequency_bands = {{58, 62}};
    uint32_t cpus = 2;
  };

  class ButterworthTransformer {
    public:
    ButterworthTransformer();

    void Setup(const ButterworthSettings& butterworth_settings);
    RC::APtr<EEGDataDouble> Filter(RC::APtr<const EEGDataDouble>& data);

    protected:
    ButterworthSettings but_set;
  };
}

#endif // BUTTERWORTHRANSFORMER_H

