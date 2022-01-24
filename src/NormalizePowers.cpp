#include "NormalizePowers.h"
#include "RC/RStr.h"

namespace CML {
  /// Default constructor that initializes and resets the internal lists
  /** @param The number of events
   *  @param The number of channels
   *  @param The number of freqs
   */
  NormalizePowers::NormalizePowers(size_t eventlen, size_t chanlen, size_t freqlen) 
      : rolling_powers(chanlen, freqlen) {
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j] = new RollingStats(eventlen);
      }
    } 
  }

  NormalizePowers::~NormalizePowers() {
    size_t freqlen = rolling_powers.size2();
    size_t chanlen = rolling_powers.size1();
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        if (rolling_powers[i][j])
          delete rolling_powers[i][j];
      }
    }
  }

  /// Reset all of the values back to 0 
  void NormalizePowers::Reset() {
    size_t freqlen = rolling_powers.size2();
    size_t chanlen = rolling_powers.size1();
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j]->Reset(); 
      }
    }
  }

  /// Update the rolling statistics with a new set of values
  /** @param The new values to be added to the rolling statics
   */
  void NormalizePowers::Update(RC::APtr<const EEGPowers>& new_data) {
    size_t freqlen = rolling_powers.size2();
    size_t chanlen = rolling_powers.size1();
    auto& new_datar = new_data->data;
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j]->Update(new_datar[i][j]);
      }
    }
  }

  /// Z-score the powers with the current statistics
  /** @param The powers to be z-scored
   */
  RC::APtr<EEGPowers> NormalizePowers::ZScore(RC::APtr<const EEGPowers>& in_data) {
    size_t freqlen = rolling_powers.size2();
    size_t chanlen = rolling_powers.size1();
    RC::APtr<EEGPowers> out_data = new EEGPowers(in_data->sampling_rate, 0, chanlen, freqlen);
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        out_datar[i][j] = rolling_powers[i][j]->ZScore(in_datar[i][j]);
      }
    }
    return out_data;
  }

  // TODO: JPB: (feature) Implement NormalizePowers::GetStats()
  //            You can make the PrintStats better once you implement this
  /// Returns the current statistics from the collected data
  /** @return The current statistics
   */
  //RC::Data2D<StatsData> NormalizePowers::GetStats() {
  //  size_t freqlen = rolling_powers.size2();
  //  size_t chanlen = rolling_powers.size1();
  //  RC::Data2D<StatsData> out_data(chanlen, freqlen);
  //  RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
  //    RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
  //      out_data[i][j] = rolling_powers[i][j]->GetStats();
  //    }
  //  }
  //  return out_data;
  //}

  void NormalizePowers::PrintStats() {
    size_t freqlen = rolling_powers.size2();
    size_t chanlen = rolling_powers.size1();
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j]->PrintStats();
      }
    }
  }
}
