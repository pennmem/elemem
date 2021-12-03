#ifndef ROLLINGSTATS_H
#define ROLLINGSTATS_H

#include "RC/Data1D.h"

namespace CML {
  class StatsData {
    RC::Data1D<double> mean;
    RC::Data1D<double> std_dev;
    RC::Data1D<double> sample_std_dev;
  };

  class RollingStats {
    public:
    RollingStats(int num_feats);

    void Update(RC::Data1D<double> new_feats);
    StatsData GetStats();

    protected:
    int count;
    RC::Data1D<double> mean;
    RC::Data1D<double> M2;
  };
}

#endif // ROLLINGSTATS_H
