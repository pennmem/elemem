#ifndef FEATUREWEIGHTS_H
#define FEATUREWEIGHTS_H

#include "RC/Data1D.h"
#include "RC/RStr.h"
#include "ChannelConf.h"
#include "ConfigFile.h"

namespace CML {
  /// This class stores feature weights, the intercept, bipolar channel
  /// numbers, and frequencies.
  class FeatureWeights {
    public:
    double intercept;
    RC::Data2D<double> coef; // frequencies outer, channels inner

    RC::Data1D<BipolarPair> chans;
    RC::Data1D<double> freqs;
  };
}

#endif // FEATUREWEIGHTS_H

