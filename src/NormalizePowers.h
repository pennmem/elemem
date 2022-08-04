#ifndef NORMALIZEPOWERS_H
#define NORMALIZEPOWERS_H

#include "EEGPowers.h"
#include "RollingStats.h"
#include "RC/Data2D.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class EventLog;

  class NormalizePowersSettings {
    public: 
    size_t freqlen = 0;
    size_t chanlen = 0;
    size_t eventlen = 0;
  }; 

  // TODO: JPB: (feature) Make NormalizePowers an RCWorker?
  class NormalizePowers {
    public:
    NormalizePowers(const NormalizePowersSettings& np_set);

    void Reset();
    void Update(RC::APtr<const EEGPowers>& new_data, RC::Ptr<EventLog> event_log=NULL);
    RC::APtr<EEGPowers> ZScore(RC::APtr<const EEGPowers>& in_data, bool div_by_zero_eq_zero);
    //RC::Data2D<StatsData> GetStats();
    void PrintStats();
    void PrintStats(size_t num_freqs);
    void PrintStats(size_t num_freqs, size_t num_chans);


    protected:
    NormalizePowersSettings np_set;
    RC::Data2D<RollingStats> rolling_powers;
  };
}

#endif // NORMALIZEPOWERS_H
