#include "StimLoop.h"
#include "SPUtils.h"
#include "CereStim.h"
#include <cmath>
#include <string>

namespace SP {

template<class T>
void ConvertRange(const std::string& s, T& t,
    uint64_t min, uint64_t max, bool onetozero=false) {
  T val;
  ConvertString(s, val, onetozero);
  if ( ! (val >= min && val <= max) ) {
    throw std::runtime_error(CleanError("Stim parameter ", val, " outside of "
          "range ", min, " to ", max, "."));
  }
  t = val;
  return;
}

template<class T1, class T2>
T1 UnitScale(T1 val, T2 mult) {
  if (val > std::numeric_limits<T1>::max() / mult) {
    throw std::runtime_error(CleanError("Unit conversion of ", val, " by ",
          mult, " would overflow its integer type."));
  }
  return val * mult;
}



void StimLoop::Run() {
  std::string line;

  stim_configured = false;

  if ( ! StimInitialize() ) {
    return;
  }

  try {
    while (true) { // Loop until closed by network exception.
      soc.Recv(line);
      const std::vector<std::string> cmd = SplitCSV(line);

      if (cmd.size() < 1) {
        continue;
      }

      if (cmd.at(0) == "SPSTIMSTART") {
        StimStart();
      }
      else if (cmd.at(0) == "SPSTIMCONFIG") {
        StimConfig(cmd, false);
      }
      else if (cmd.at(0) == "SPSTIMTHETACONFIG") {
        StimConfig(cmd, true);
      }
      else {
        soc.Send(std::string("SPERROR,") + CleanError("StimProc command not "
              "recognized:  \"", cmd.at(0), "\""));
      }
    }
  }
  catch (std::runtime_error& ex) {
    // Pass.
    // Connection closed, so proceed to terminate.
  }
}


bool StimLoop::StimInitialize() {
  std::string line;

  soc.Recv(line);

  if (line != conf.subject) {
    soc.Send(std::string("SPERROR,") +
        CleanError("Subject code ", line, " does not match configuration "
          "code ", conf.subject));
    return false;
  }

  soc.Send(std::string("SPREADY,StimProc,") + CleanStr(version));
  return true;
}


void StimLoop::StimStart() {
  if ( ! stim_configured ) {
    soc.Send("SPSTIMSTARTERROR,Attempted to stimulate with no stimulation "
        "configured.");
    return;
  }

  try {
    cerestim.Stimulate();
  }
  catch (std::exception& ex) {
    soc.Send(std::string("SPSTIMSTARTERROR,") +
        CleanError(ex.what()));
    return;
  }

  soc.Send("SPSTIMSTARTDONE");
}


void StimLoop::StimConfig(const std::vector<std::string>& cmd,
    bool thetaburst) {
  stim_configured = false;
  if (cmd.size() < 2 || (cmd.size()==2 && cmd.at(1)=="")) {
    // No channels specified.  Configure for no-stim.
    try {
      cerestim.ConfigureStimulation(CML::CSStimProfile{});
      soc.Send("SPSTIMCONFIGDONE");
    }
    catch (std::exception& ex) {
      soc.Send(std::string("SPSTIMCONFIGERROR,") + CleanStr(ex.what()));
    }
    return;
  }

  const uint64_t elem_cnt = thetaburst ? 6 : 5;
  uint64_t pair_cnt = 0;
  try {
    pair_cnt = To_uint64(cmd.at(1));

    if (pair_cnt > 6) {
      soc.Send(std::string("SPSTIMCONFIGERROR,") +
          CleanError("Stim pair count ", pair_cnt, " exceeded maximum 6."));
    }

    uint64_t expected = 2 + pair_cnt*elem_cnt;
    if (cmd.size() != expected) {
      soc.Send(std::string("SPSTIMCONFIGERROR,") + CleanError("Expected ",
            expected, " stim config line elements and got ", cmd.size()));
      return;
    }

    CML::CSStimProfile stim_profile;
    for (uint64_t p=0; p<pair_cnt; p++) {
      uint64_t i = 2 + p*elem_cnt;

      CML::CSStimChannel csc;
      ConvertRange(cmd.at(i), csc.electrode_pos, 0, 255);
      ConvertRange(cmd.at(i+1), csc.electrode_neg, 0, 255);

      ChannelLimits limits = FindLimit(csc.electrode_pos, csc.electrode_neg);

      ConvertRange(cmd.at(i+2), csc.amplitude, 0, limits.max_amp_uA);
      ConvertRange(cmd.at(i+3), csc.frequency, limits.min_freq_Hz,
          limits.max_freq_Hz);

      if (thetaburst) {
        csc.burst_frac = To_float(cmd.at(i+5));
        if (csc.burst_frac <= 0 || csc.burst_frac >= 1) {
          soc.Send(std::string("SPSTIMCONFIGERROR,") + CleanError("Burst "
            "fraction ", csc.burst_frac, " must be greater than 0 and less "
            "than 1."));
          return;
        }
        uint64_t min_slow_freq = uint64_t(
            std::ceil(1000 * csc.burst_frac / limits.max_dur_ms));
        ConvertRange(cmd.at(i+4), csc.burst_slow_freq, min_slow_freq,
            csc.frequency);
      }

      else { // Conventional stim pattern, not theta-burst.
        ConvertRange(cmd.at(i+4), csc.duration, 0, limits.max_dur_ms);
        csc.duration = UnitScale(csc.duration, 1000ul);  // To us.
      }

      csc.area = limits.area_mm_sq;

      stim_profile += csc;
    }

    cerestim.ConfigureStimulation(stim_profile);
    stim_configured = true;
    soc.Send("SPSTIMCONFIGDONE");
  }
  catch(std::exception& ex) {
    soc.Send(std::string("SPSTIMCONFIGERROR,") + CleanStr(ex.what()));
    return;
  }
}

ChannelLimits StimLoop::FindLimit(uint8_t pos, uint8_t neg) {
  for (size_t c=0; c<conf.chan_limits.size(); c++) {
    if (pos == conf.chan_limits[c].chan_pos &&
        neg == conf.chan_limits[c].chan_neg) {
      return conf.chan_limits[c];
    }
  }
  throw std::runtime_error(CleanError("StimProc not configured for stim on "
        "pair ", uint32_t(pos), "_", uint32_t(neg), "."));
}

}

