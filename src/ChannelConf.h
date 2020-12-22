#ifndef CHANNELCONF_H
#define CHANNELCONF_H

namespace CML {
  class CSStimChannel {
    public:
    uint8_t electrode_pos;  // Goes positive first / anodic
    uint8_t electrode_neg;  // Goes negative first / cathodic
    uint16_t amplitude; // Unit 1uA, granularity 100uA for macro.
    uint32_t frequency; // Unit Hz.
    uint32_t duration;  // Unit us.
  };
}

#endif // CHANNELCONF_G

