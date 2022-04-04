// 2022, James Bruska
// Computational Memory Lab, University of Pennsylvania
//
// This file provides the interface for Stim
//
/////////////////////////////////////////////////////////////////////////////

#ifndef STIMINTERFACE_H
#define STIMINTERFACE_H

#include <vector>
#include <RC/Macros.h>
#include "ChannelConf.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>

namespace CML {
  class StimMaxValues {
    public:
    uint8_t voltage; // [7,15], (0.5 + 0.6*voltage) = value in volts.
    uint16_t amplitude; // Unit 1uA, granularity 100uA for macro.
    uint32_t phase_charge; // Unit pC.  amplitude_us * width_us.  (Not 100us)
    uint32_t frequency; // Unit Hz.
  };

  class StimProfile {
    public:
    StimProfile& operator+=(const StimChannel& chan) {
      stim_profile.push_back(chan);
      return *this;
    }
    size_t size() const {
      return stim_profile.size();
    }
    const StimChannel& operator[](size_t i) const {
      return stim_profile[i];
    }

    protected:
    // TODO: JPB: (refactor) Change vector to Data1D
    std::vector<StimChannel> stim_profile;
    friend class StimInterface;
  };

  // All channel numbers are zero-based.  For user-interfacing use one-based.
  class StimInterface {
    public:
    StimInterface() = default;
    virtual ~StimInterface() = default;

    // Shannon, R. V. (1992). A model of safe levels for electrical
    // stimulation. IEEE Transactions on biomedical engineering, 39(4),
    // 424-426.  eqn. 2
    uint16_t ShannonCriteria(float area_mmsq) {
      // I = sqrt(A*10^k)/T
      // I : uA
      // T : s
      // A : cm^2
      float current_f_uA = std::sqrt(area_mmsq*1e-2*31.622776601683793) /
                           (stim_width_us*1e-6);

      uint16_t current_uA = uint16_t(current_f_uA);
      if (current_f_uA > uint16_t(-1)) {
        current_uA = uint16_t(-1);
      }

      return current_uA;
    }

    uint16_t ShannonCriteria(const StimChannel& chan) {
      return ShannonCriteria(chan.area);
    }

    bool ShannonSafe(float area_mmsq, uint16_t amplitude_uA) {
      return amplitude_uA <= ShannonCriteria(area_mmsq);
    }

    bool ShannonSafe(const StimChannel& chan) {
      return ShannonSafe(chan.area, chan.amplitude);
    }

    void ShannonAssert(float area_mmsq, uint16_t amplitude_uA) {
      if ( ! ShannonSafe(area_mmsq, amplitude_uA)) {
        throw std::runtime_error(std::string("Requested ") +
            std::to_string(amplitude_uA*1e-3) +
            "mA current exceeded Shannon safety criteria for " +
            std::to_string(area_mmsq) + "mm^2 electrode.  "
            "Stimulation not done.");
      }
    }

    void ShannonAssert(const StimChannel& chan) {
      ShannonAssert(chan.area, chan.amplitude);
    }

    void ConfigureStimulation(StimProfile profile) {
      is_configured = false;

      // Reset defaults. 
      burst_slow_freq = 0; 
      burst_frac = 1; 
      burst_duration_us = 0; 

      struct FreqDurAmp {
      uint32_t frequency;
      uint32_t duration;
      uint16_t amplitude;
      bool operator==(const FreqDurAmp& other) const {
          return frequency == other.frequency &&
                 duration == other.duration &&
                 amplitude == other.amplitude;
        }
      };
      std::vector<FreqDurAmp> fda_vec;

      size_t prof_size = profile.stim_profile.size();

      // TODO: JPB: (refactor) change this to const define
      size_t max_bipolar_pairs = (128-2)/2;  // From stim script max length.
      if (prof_size > (128-2)/2) {
        throw std::runtime_error("Only " + std::to_string(max_bipolar_pairs) +
            " bipolar pairs can be stimulated.");
      }

      // Verify electrodes are all unique.
      std::vector<uint8_t> uniqueness_check(256);
      for (size_t i=0; i<profile.stim_profile.size(); i++) {
        auto& prof =  profile.stim_profile[i];

        if (uniqueness_check.at(prof.electrode_pos) ||
            uniqueness_check.at(prof.electrode_neg) ||
            prof.electrode_pos == prof.electrode_neg) {
          throw std::runtime_error("Stimulation channels must be unique");
        }
        uniqueness_check.at(prof.electrode_pos) = 1;
        uniqueness_check.at(prof.electrode_neg) = 1;

        if (i==0) {  // Save first burst setting.
          burst_slow_freq = prof.burst_slow_freq;
          burst_frac = prof.burst_frac;
          burst_duration_us = prof.duration;
        }  // All burst settings must be identical.
        else if ((burst_slow_freq != prof.burst_slow_freq) ||
                 (std::abs(burst_frac - prof.burst_frac) > 0.001) ||
                 (burst_duration_us != prof.duration)) {
          throw std::runtime_error("Simultaneous stim channels must be all "
              "identical duration and burst stim settings, or all not burst "
              "stim.");
        }
      }

      // Sensible burst settings only.
      if (burst_frac > 1) {
        throw std::runtime_error("Attempted to configure stim burst fraction "
            "greater than 1.");
      }
      if (burst_frac < 1) {
        if (burst_slow_freq == 0) {
          throw std::runtime_error("Attempted burst fraction less than 1 at "
              "0 Hz.");
        }
      }
      if (burst_frac == 1) {
        burst_slow_freq = 0;  // Triggers no burst in stim worker.
      }

      std::vector<size_t> pattern_index(prof_size);

      for (size_t i=0; i<prof_size; i++) {
        // Extract and index the unique ones.
        auto& chan = profile.stim_profile[i];

        // Enforce Shannon criteria for safety.
        ShannonAssert(chan);

        FreqDurAmp fda{chan.frequency, chan.duration, chan.amplitude};
        auto res = std::find(fda_vec.begin(), fda_vec.end(), fda);
        if (res == fda_vec.end()) {
          fda_vec.push_back(fda);
          res = std::prev(fda_vec.end());
        }
        auto pat_index = res - fda_vec.begin();
        pattern_index[i] = size_t(pat_index);
      }

      for (size_t i=0; i<fda_vec.size(); i++) {
        auto& afd = fda_vec[i];
        uint64_t pulses_64 = (uint64_t(afd.duration) * afd.frequency) / 1000000;
        if (burst_frac < 1) {
          // Configure stim profile for burst-on period only.
          pulses_64 = uint64_t(round(afd.frequency * burst_frac /
                                     burst_slow_freq));
        }

        if (pulses_64 < 1) {
          pulses_64 = 1;
        }
        if (pulses_64 > 255) {
          throw std::runtime_error(std::to_string(afd.duration) + "us pulse "
              "duration too long for " + std::to_string(afd.frequency) + "Hz "
              "stimulus.");
        }
      }

      ConfigureStimulationHelper(profile);

      is_configured = true;
    }

    void Open() {
      is_configured = false;

      OpenHelper();
    }

    void Close() {
      CloseHelper();

      is_configured = false;
    }

    void Stimulate() {
      if ( ! is_configured ) {
        throw std::runtime_error("Stimulation attempted when no stim pattern "
            "was internally configured.");
      }
      StimulateHelper();
    }

    uint32_t GetBurstSlowFreq() const { return burst_slow_freq; }
    uint32_t GetBurstDuration_us() const { return burst_duration_us; }

    protected:
    virtual void OpenHelper() = 0;
    virtual void CloseHelper() = 0;
    virtual void StimulateHelper() = 0;
    virtual void ConfigureStimulationHelper(StimProfile profile) = 0;

    bool is_configured = false;

    uint32_t burst_slow_freq = 0; // Unit Hz.  Slower envelope freq of bursts.
    float burst_frac = 1; // Fraction of 1/burst_slow_freq to stimulate for.
    uint32_t burst_duration_us = 0;

    uint16_t stim_width_us = 300;

    const StimMaxValues stim_max_vals = {
      .voltage=15, // (0.5 + 0.6*15)V = 9.5V
      .amplitude=3500, // 35*100uA.
      .phase_charge=20000000, // Won't go above 1.05e6 with 300us
      .frequency=1000}; // Hz
  };
}

#endif

