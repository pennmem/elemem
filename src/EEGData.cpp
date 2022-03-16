#include "EEGData.h"
#include "RC/RStr.h"

namespace CML {
  template<typename T>
  std::ostream& operator<<(std::ostream& os, const EEGDataT<T>& data) {
    auto& datar = data.data;
    size_t chanlen = datar.size();

    if (chanlen > datar.size()) {
      Throw_RC_Error((RC::RStr("The chanlen (") + chanlen +
            ") is longer than then number of freqs in powers (" + datar.size() + ")").c_str());
    }

    os << "sampling_rate: " << data.sampling_rate << "\n";
    os << "data: \n";
    RC_ForRange(c, 0, chanlen) { // Iterate over channels
      os << "channel " << c << ": " << RC::RStr::Join(datar[c], ", ") << "\n";
    }
    os << "\n––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––\n";
  }
}
