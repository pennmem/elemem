#include "EEGPowers.h"
#include "RC/RStr.h"

namespace CML {
  void PrintEEGPowers(const EEGPowers& powers) {
    auto& datar = powers.data;
    RC::RStr deb_msg = RC::RStr("sampling_rate: ") + data.sampling_rate + "\n";
    deb_msg += "data:\n";
    RC_ForIndex(c, datar)
      deb_msg += "channel " + RC::RStr(c) + ": " + RC::RStr::Join(datar[c], ", ") + "\n";
    deb_msg += "\n\n";
    RC_DEBOUT(deb_msg);
  }
}
