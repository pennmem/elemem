#ifndef NORMALIZEPOWERS_H
#define NORMALIZEPOWERS_H

#include "EEGPowers.h"
#include "RollingStats.h"
#include "RC/Data2D.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class NormalizePowersSettings {
    public: 
    size_t num_freqs = 0;
    size_t num_chans = 0;
    size_t num_events = 0;
  }; 

  // TODO: JPB: (feature) Make NormalizePowers an RCWorker?
  class NormalizePowers {
    public:
    NormalizePowers(size_t eventlen, size_t chanlen, size_t freqlen);
    ~NormalizePowers();
    NormalizePowers(const NormalizePowers& other) = delete;
    NormalizePowers& operator=(const NormalizePowers& other) = delete;

    void Reset();
    void Update(RC::APtr<const EEGPowers>& new_data);
    RC::APtr<EEGPowers> ZScore(RC::APtr<const EEGPowers>& in_data);
    //RC::Data2D<StatsData> GetStats();
    void PrintStats();


	protected:
	RC::Data2D<RollingStats *> rolling_powers;
  };
}

#endif // NORMALIZEPOWERS_H
