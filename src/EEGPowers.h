#ifndef EEGPOWERS_H
#define EEGPOWERS_H

#include "RC/Data3D.h"
#include "RC/RStr.h"

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
   *  EEGPowers eegPowers(4, 3, 2);
   *  size_t val = 0;
   *  RC_ForRange(i, 0, eegPowers.size3()) { // Iterate over frequencies
   *    RC_ForRange(j, 0, eegPowers.size2()) { // Iterate over channels
   *      RC_ForRange(k, 0, eegPowers.size1()) { // Iterate over samples
   *        eegPowers[i][j][k] = val++;
   *      }
   *    }
   *  }
   *
   *  // Print EEGPowers
   *  PrintEEGPowers(eegPowers);
   *  \endcode
   *  \nosubgrouping
   */
  class EEGPowers {
    public:
    EEGPowers(size_t sampling_rate) : sampling_rate(sampling_rate) {}
    EEGPowers(size_t sampling_rate, size_t event_len, size_t chan_len, size_t freq_len)
      : sampling_rate(sampling_rate), data(event_len, chan_len, freq_len) {}
    
    size_t sampling_rate;
    RC::Data3D<double> data;
    
    void Print(size_t num_freqs, size_t num_chans) const {
      size_t freqlen = num_freqs;
      size_t chanlen = num_chans;
      //size_t eventlen = num_events;

      if (freqlen > data.size3()) {
        Throw_RC_Error((RC::RStr("The freqlen (") + freqlen +
              ") is longer than then number of freqs in powers (" + data.size3() + ")").c_str());
      } else if (chanlen > data.size2()) {
        Throw_RC_Error((RC::RStr("The chanlen (") + chanlen +
              ") is longer than then number of freqs in powers (" + data.size2() + ")").c_str());
      }

      RC::RStr deb_msg = RC::RStr("sampling_rate: ") + sampling_rate + "\n";
      deb_msg += "data:\n\n";
      RC_ForRange(i, 0, freqlen) { // Iterate over frequencies
        deb_msg += "frequency " + RC::RStr(i) + "\n";
        RC_ForRange(j, 0, chanlen) { // Iterate over channels
          deb_msg += "channel " + RC::RStr(j) + ": " + RC::RStr::Join(data[i][j], ", ") + "\n";
        }
        deb_msg += "\n";
      }
      deb_msg += "––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––\n";
      RC_DEBOUT(deb_msg);
    }

    void Print(size_t num_freqs) const {
      size_t freqlen = num_freqs;
      size_t chanlen = data.size2();
      //size_t eventlen = data.size1();
      Print(freqlen, chanlen);
    } 
    
    void Print() const {
      size_t freqlen = data.size3();
      size_t chanlen = data.size2();
      //size_t eventlen = data.size1();
      Print(freqlen, chanlen);
    }
  };

  //void PrintEEGPowers(const EEGPowers& powers);
  //void PrintEEGPowers(const EEGPowers& powers, size_t num_freqs);
  //void PrintEEGPowers(const EEGPowers& powers, size_t num_freqs, size_t num_chans);
}

#endif // EEGPOWERS_H

