#ifndef EEGDATA_H
#define EEGDATA_H

#include "RC/Data1D.h"
#include "RC/RStr.h"

namespace CML {
  /// This is a simple class that acts as a container for EEG data.
  /** This class is an array of arrays containing EEG data and its sampling
   *  rate. The data is arranged with the outer array as the EEG channels and the
   *  inner arrays as the EEG values over time (for that channel).
   *
   *  Ex: An EEG acquisition with 2 channels that had collected 3 samples for
   *  each channel would look like this:
   *  \n[ [ i16, i16, i16 ], [ i16, i16, i16 ] ]
   *
   *  Common sampling rates (freqs):
   *  - 1kHz = no micros
   *  - 30kHz = micros
   *
   *  Note that channels can have NO data (check data size for each channel)
   *
   *  Usage example (print the data):
   *  \code{.cpp}
   *  #include <iostrean>
   *  #include "RC/Ptr.h"
   *  #include "RC/RStr.h"
   *
   *  #include "EEGData.h"
   *
   *  // Setup EEGData for example
   *  EEGData eegData(1000);
   *  eegData.data.Resize(2);
   *  eegData.data[0] = RC::Data1D<short> {5,10,1};
   *  eegData.data[1] = RC::Data1D<short> {6,11,2};
   *
   *  // Print EEGData
   *  RC::RStr msg = "sampling_rate: " + RC::RStr(eegData.sampling_rate) + "\n";
   *  msg += "data:\n"
   *  for (size_t c=0; c<eegData.data.size(); c++) {
   *    msg += "Channel " + RC::RStr(c) + ": " + RC::RStr::Join(eegData.data[c], ", ") + "\n";
   *  }
   *  cout << msg << endl;
   *  \endcode
   *  \nosubgrouping
   */
  template<typename T>
  class EEGDataT {
    public:
    EEGDataT(size_t sampling_rate, size_t sample_len)
      : sampling_rate(sampling_rate), sample_len(sample_len) {}

    size_t sampling_rate;
    // TODO - Encapsulate sample_len and data to preserve this invariant.
    size_t sample_len; // Internal Data1D size is either 0 or sample_len
    RC::Data1D<RC::Data1D<T>> data;

    void EnableChan(size_t chan) {
      data[chan].Resize(sample_len);
    }

    void Print(size_t num_chans) const {
      size_t chanlen = num_chans;

      if (chanlen > data.size()) {
        Throw_RC_Error((RC::RStr("The chanlen (") + chanlen +
              ") is longer than then number of freqs in powers (" + data.size() + ")").c_str());
      }

      RC::RStr deb_msg = RC::RStr("sampling_rate: ") + sampling_rate + "\n";
      deb_msg += RC::RStr("sample_len: ") + sample_len + "\n";
      deb_msg += "data: \n";
      RC_ForRange(c, 0, chanlen) { // Iterate over channels
        deb_msg += "channel " + RC::RStr(c) + ": " + RC::RStr::Join(data[c], ", ") + "\n";
      }
      deb_msg += "\n––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––\n";
      RC_DEBOUT(deb_msg);
    }

    void Print() const {
      size_t chanlen = data.size();
      Print(chanlen);
    }
  };

  using EEGData = EEGDataT<int16_t>;
  using EEGDataRaw = EEGDataT<int16_t>;
  using EEGDataDouble = EEGDataT<double>;
}

#endif // EEGDATA_H

