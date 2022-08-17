#ifndef ROLLINGSTATS_H
#define ROLLINGSTATS_H

#include "RC/Data1D.h"

namespace CML {
  class StatsData {
    public:
    RC::Data1D<double> means = {};
    RC::Data1D<double> sample_std_devs = {};
    StatsData(size_t num_values)
      : means(num_values), sample_std_devs(num_values) {}
    StatsData(RC::Data1D<double> means, RC::Data1D<double> sample_std_devs)
      : means(means), sample_std_devs(sample_std_devs) {}
  };

  class RollingStats {
    public:
    RollingStats() {}
    RollingStats(size_t num_values);

    size_t size();
    void SetSize(size_t num_values);
    int GetCount();
    void Reset();
    void Update(const RC::Data1D<double>& new_values);
    RC::Data1D<double> ZScore(const RC::Data1D<double>& data, bool div_by_zero_eq_zero);
    StatsData GetStats();
    void PrintStats();


    protected:
    int count;
    RC::Data1D<double> means;
    RC::Data1D<double> m2s;
  };
}

#endif // ROLLINGSTATS_H
