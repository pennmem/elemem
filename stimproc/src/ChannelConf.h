#ifndef CHANNELCONF_H
#define CHANNELCONF_H

#include <iostream>

namespace CML {
  class CSStimChannel {
    public:
    uint8_t electrode_pos = 0;  // Goes positive first / anodic
    uint8_t electrode_neg = 0;  // Goes negative first / cathodic
    uint16_t amplitude = 0; // Unit 1uA, granularity 100uA for macro.
    uint32_t frequency = 0; // Unit Hz.
    uint32_t duration = 0;  // Unit us.
    float area = 0; // Unit mm^2.
    float burst_frac = 1; // Fraction of 1/burst_slow_freq to stimulate for.
    uint32_t burst_slow_freq = 0; // Unit Hz.  Slower envelope freq of bursts.
  };

  class BipolarPair {
    public:
    uint8_t pos = 0;
    uint8_t neg = 0;
  };

  inline std::ostream& operator<< (std::ostream &out, const BipolarPair& bp) {
    out << "(" << (unsigned int)(bp.pos) << ", "
               << (unsigned int)(bp.neg) << ")";
    return out;
  }
}

#endif // CHANNELCONF_H

