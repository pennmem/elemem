#ifndef EEGPOWERS_H
#define EEGPOWERS_H

#include "RC/Data3D.h"

namespace CML {
  /// This is a simple class that acts as a container for EEG Powers.
  /** This class is an array of arrays of arrays containing EEG powers and its sampling
   *  rate. The data is arranged with the outer array as the frequencies, the middle 
   *  arrays as the EEG channels and the inner arrays as the EEG powers over time.
   *
   *  Ex: An EEG acquisition with 2 frequencies and 3 channels that had collected 
   *  4 samples/events for each combination of those would look like this:
   *    [ [ [ i16, i16, i16, i16 ], [ i16, i16, i16, i16 ], [ i16, i16, i16, i16 ] ], 
   *      [ [ i16, i16, i16, i16 ], [ i16, i16, i16, i16 ], [ i16, i16, i16, i16 ] ] ]
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
   *  EEGPowers eegPowers(2, 3, 4);
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
    EEGPowers(size_t sampling_rate, size_t d_size1, size_t d_size2, size_t d_size3)
      : sampling_rate(sampling_rate), data(d_size1, d_size2, d_size3) {}
    size_t sampling_rate;
    RC::Data3D<double> data;
  };

  void PrintEEGPowers(const EEGPowers& powers);
}

#endif // EEGPOWERS_H

