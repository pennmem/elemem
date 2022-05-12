#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

namespace SP {
  class ChannelLimits {
    public:
    uint8_t chan_pos = 0;
    uint8_t chan_neg = 0;
    uint16_t max_amp_uA = 0;
    uint32_t min_freq_Hz = 0;
    uint32_t max_freq_Hz = 0;
    uint32_t max_dur_ms = 0;
    float area_mm_sq = 0;
  };

  class Config {
    public:
    Config(const std::string& filename);

    std::string filename;

    std::string subject;
    std::vector<ChannelLimits> chan_limits;
  };
}

#endif

