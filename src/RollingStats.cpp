#include "RollingStats.h"

namespace CML {
  RollingStats::RollingStats(int num_feats) {
    count = 0;
    means.Resize(num_feats);
    m2s.Resize(num_feats);
    Reset();
  }

  void RollingStats::Reset() {
    for (double &mean : means) 
      mean = 0;
    for (double &m2 : m2s)
      m2 = 0;
  }

  void RollingStats::Update(RC::Data1D<double> new_feats) {
    // TODO: JPB: Force new_feats to be same size as means & m2s
    count += 1;
    for (size_t i=0; i<new_feats.size(); ++i) {
      double delta = new_feats[i] - means[i];
      means[i] += delta / count;
      double delta2 = new_feats[i] - means[i];
      m2s[i] += delta * delta2;
    }
  }
  
  StatsData RollingStats::GetStats() {
    RC::Data1D<double> std_dev(m2s.size());
    RC::Data1D<double> sample_std_dev(m2s.size());
    for (size_t i=0; i<m2s.size(); ++i) {
      std_dev[i] = m2s[i] / count;
      sample_std_dev[i] = m2s[i] / (count - 1);
    }
    
    StatsData ret {means, std_dev, sample_std_dev};
    return ret;
  }
}
