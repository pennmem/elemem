#include "RollingStats.h"

namespace CML {
  RollingStats::RollingStats(int num_feats) {
    
  }

  void RollingStats::Update(RC::Data1D<double> new_feats) {

  }
  
  StatsData RollingStats::GetStats() {
    return StatsData();
  }
}
