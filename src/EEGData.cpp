#include "EEGData.h"
#include "RC/RStr.h"

namespace CML {
  void PrintEEGData(const EEGData& data) {
    auto& datar = data.data;
    size_t chanlen = datar.size();
	PrintEEGData(data, chanlen);
  }

  void PrintEEGData(const EEGData& data, size_t num_chans) {
    auto& datar = data.data;
    size_t chanlen = num_chans;

    if (chanlen > datar.size()) {
      Throw_RC_Error((RC::RStr("The chanlen (") + chanlen +
            ") is longer than then number of freqs in powers (" + datar.size() + ")").c_str());
    }

    RC::RStr deb_msg = RC::RStr("sampling_rate: ") + data.sampling_rate + "\n";
    deb_msg += "data: \n";
    RC_ForRange(c, 0, chanlen) { // Iterate over channels
      deb_msg += "channel " + RC::RStr(c) + ": " + RC::RStr::Join(datar[c], ", ") + "\n";
    }
    deb_msg += "\n––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––\n";
    RC_DEBOUT(deb_msg);
  }
}
