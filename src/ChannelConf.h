#ifndef CHANNELCONF_H
#define CHANNELCONF_H

namespace CML {
  class CSStimChannel {
    public:
    uint8_t electrode_pos = 0;  // Goes positive first / anodic
    uint8_t electrode_neg = 0;  // Goes negative first / cathodic
    uint16_t amplitude = 0; // Unit 1uA, granularity 100uA for macro.
    uint32_t frequency = 0; // Unit Hz.
    uint32_t duration = 0;  // Unit us.
    float area = 0; // Unit mm^2.
  };
}

#endif // CHANNELCONF_G

