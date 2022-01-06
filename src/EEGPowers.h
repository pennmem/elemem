#ifndef EEGPOWERS_H
#define EEGPOWERS_H

#include "RC/Data1D.h"

namespace CML {
  /// This is a simple class that acts as a container for EEG Powers.
  /** This class is an array of arrays containing EEG powers and its sampling
   *  rate. The data is arranged with the outer array as the EEG channels and the
   *  inner arrays as the EEG powers over time (for that channel).
   *
   *  Ex: An EEG acquisition with 2 channels that had collected 3 samples for
   *  each channel would look like this:
   *  \n[ [ i16, i16, i16 ], [ i16, i16, i16 ] ]
   *
   *  Common sampling rates (freqs):
   *  - 1kHz = no micros
   *  - 30kHz = micros
   *
   *  Usage example (print the data):
   *  \code{.cpp}
   *  #include <iostrean>
   *  #include "RC/Ptr.h"
   *  #include "RC/RStr.h"
   *
   *  #include "EEGPowers.h"
   *
   *  // Setup EEGPowers for example
   *  // TODO: JPB: (need) FIX THIS
   *  EEGPowers eegPowers(10, 10);
   *  eegPowers.data.Resize(2);
   *  eegPowers.data[0] = RC::Data1D<short> {5,10,1};
   *  eegPowers.data[1] = RC::Data1D<short> {6,11,2};
   *
   *  // Print EEGPowers
   *  PrintEEGPowers(eegPowers);
   *  \endcode
   *  \nosubgrouping
   */
  class EEGPowers {
    public:
    EEGPowers(size_t sampling_rate) : sampling_rate(sampling_rate) {}
    size_t sampling_rate;
    RC::Data2D<double> data;
  };

  void PrintEEGPowers(const EEGPowers& powers);
}

#endif // EEGPOWERS_H

