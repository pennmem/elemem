#ifndef WEIGHTMANAGER_H
#define WEIGHTMANAGER_H

#include "FeatureWeights.h"
#include "ChannelConf.h"
#include "ConfigFile.h"

namespace CML {
  /// This class loads classification result json files and provides feature
  /// weights, bipolar channel numbers, and frequencies.
  class WeightManager {
    public:
    WeightManager(RC::RStr classif_json, RC::APtr<const CSVFile> elec_config);

    // This is an APtr to const so future experiments can atomically update
    // the values.
    RC::APtr<const FeatureWeights> weights;
  };
}

#endif // WEIGHTMANAGER_H

