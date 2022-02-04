#include "EEGPowers.h"
#include "RC/RStr.h"

namespace CML {
  void PrintEEGPowers(const EEGPowers& powers) {
    auto& datar = powers.data;
    size_t freqlen = datar.size3();
    size_t chanlen = datar.size2();
    //size_t eventlen = datar.size1();
    PrintEEGPowers(powers, freqlen, chanlen);
  }

  void PrintEEGPowers(const EEGPowers& powers, size_t num_freqs) {
    auto& datar = powers.data;
    size_t freqlen = num_freqs;
    size_t chanlen = datar.size2();
    //size_t eventlen = datar.size1();
    PrintEEGPowers(powers, freqlen, chanlen);
  }

  void PrintEEGPowers(const EEGPowers& powers, size_t num_freqs, size_t num_chans) {
    auto& datar = powers.data;
    size_t freqlen = num_freqs;
    size_t chanlen = num_chans;
    //size_t eventlen = num_events;

    if (freqlen > datar.size3()) {
      Throw_RC_Error((RC::RStr("The freqlen (") + freqlen +
            ") is longer than then number of freqs in powers (" + datar.size3() + ")").c_str());
    } else if (chanlen > datar.size2()) {
      Throw_RC_Error((RC::RStr("The chanlen (") + chanlen +
            ") is longer than then number of freqs in powers (" + datar.size2() + ")").c_str());
    }

    RC::RStr deb_msg = RC::RStr("sampling_rate: ") + powers.sampling_rate + "\n";
    deb_msg += "data:\n\n";
    RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
      deb_msg += "frequency " + RC::RStr(i) + "\n";
      RC_ForRange(j, 0, chanlen) { // Iterate over channels
        deb_msg += "channel " + RC::RStr(j) + ": " + RC::RStr::Join(datar[i][j], ", ") + "\n";
      }
      deb_msg += "\n";
    }
    deb_msg += "––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––\n";
    RC_DEBOUT(deb_msg);
  }
}
