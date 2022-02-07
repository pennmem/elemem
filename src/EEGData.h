#ifndef EEGDATA_H
#define EEGDATA_H

#include "RC/Data1D.h"

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
  class EEGData {
    public:
    EEGData(size_t sampling_rate) : sampling_rate(sampling_rate) {}
    size_t sampling_rate;
    RC::Data1D<RC::Data1D<int16_t>> data;
  };

  void PrintEEGData(const EEGData& data);
  void PrintEEGData(const EEGData& data, size_t num_chans);
}

#endif // EEGDATA_H

