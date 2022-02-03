#include "EEGPowers.h"
#include "RC/RStr.h"

namespace CML {
  void PrintEEGPowers(const EEGPowers& powers) {
    auto& datar = powers.data;
    size_t freqlen = datar.size3();
    size_t chanlen = datar.size2();
    //size_t eventlen = datar.size1();

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
