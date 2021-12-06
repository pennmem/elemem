#ifndef ROLLINGSTATS_H
#define ROLLINGSTATS_H

#include "RC/Data1D.h"

namespace CML {
  class StatsData {
    public:
    RC::Data1D<double> means = {};
    RC::Data1D<double> std_devs = {};
    RC::Data1D<double> sample_std_devs = {};
    StatsData(size_t num_feats) : means(num_feats), std_devs(num_feats), sample_std_devs(num_feats) {}
    StatsData(RC::Data1D<double> means, RC::Data1D<double> std_devs, RC::Data1D<double> sample_std_devs) : means(means), std_devs(std_devs), sample_std_devs(sample_std_devs) {}
  };

  class RollingStats {
    public:
    RollingStats(int num_feats);

    void Update(RC::Data1D<double> new_feats);
    void Reset();
    StatsData GetStats();

    protected:
    int count;
    RC::Data1D<double> means;
    RC::Data1D<double> m2s;
  };
}

#endif // ROLLINGSTATS_H
