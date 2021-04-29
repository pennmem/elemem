#ifndef EEGDATA_H
#define EEGDATA_H

#include "RC/Data1D.h"

namespace CML {
  class EEGData {
    public:
    EEGData(size_t sampling_rate) : sampling_rate(sampling_rate) {}
    size_t sampling_rate;
    RC::Data1D<RC::Data1D<int16_t>> data;
  };
}

#endif // EEGDATA_H

