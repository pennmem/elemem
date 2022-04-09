#include "Config.h"
#include "SPUtils.h"
#include <fstream>
#include <iostream>

namespace SP {
  Config::Config(const std::string& filename)
    : filename(filename) {

    std::ifstream conffile(filename, std::ios::binary);
    if (!conffile.is_open()) {
      throw std::runtime_error("Could not open StimProc config file.");
    }

    std::string line;
    std::vector<std::string> split;

    if ( ! std::getline(conffile, line) ) {
      throw std::runtime_error("First line missing in StimProc config file.");
    }
    subject = Trim(line);

#ifdef TESTING
    std::cout << ":" << subject << ":" << std::endl;
#endif

    while (std::getline(conffile, line)) {
      line = Trim(line);
      if (line.size() == 0) {
        continue;
      }

      split = SplitCSV(line);
      if (split.size() != 7) {
        throw std::runtime_error("Channel StimProc limits must all be 7 "
            "comma separated values: chan_pos chan_neg max_amp min_freq "
            "max_freq max_dur area.");
      }

      ChannelLimits limit;

      ConvertString(split.at(0), limit.chan_pos, true);
      ConvertString(split.at(1), limit.chan_neg, true);
      ConvertString(split.at(2), limit.max_amp_uA);
      ConvertString(split.at(3), limit.min_freq_Hz);
      ConvertString(split.at(4), limit.max_freq_Hz);
      ConvertString(split.at(5), limit.max_dur_ms);
      limit.area_mm_sq = To_float(split.at(6));

      if (limit.max_amp_uA == 0 || limit.max_freq_Hz == 0 ||
          limit.max_dur_ms == 0) {
        throw std::runtime_error("Zero maximum found in StimProc config.");
      }

      if ( ! (limit.area_mm_sq > 0 && limit.area_mm_sq < 25) ) {
        throw std::runtime_error("Electrode contact area out of range in "
            "StimProc config.");
      }

      chan_limits.push_back(limit);

#define TESTING
#ifdef TESTING
      std::cout << (unsigned int)(limit.chan_pos) << std::endl;
      std::cout << (unsigned int)(limit.chan_neg) << std::endl;
      std::cout << limit.max_amp_uA << std::endl;
      std::cout << limit.min_freq_Hz << std::endl;
      std::cout << limit.max_freq_Hz << std::endl;
      std::cout << limit.max_dur_ms << std::endl;
      std::cout << limit.area_mm_sq << std::endl;
      std::cout << std::endl;
#endif
    }
  }
}

