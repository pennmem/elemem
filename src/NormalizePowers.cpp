#include "NormalizePowers.h"
#include "RC/RStr.h"
#include "EventLog.h"
#include "JSONLines.h"

namespace CML {
  /// Default constructor that initializes and resets the internal lists
  /** @param The settings needed to set up consistent normalization
   */
  NormalizePowers::NormalizePowers(const NormalizePowersSettings& np_set)
      : np_set(np_set), rolling_powers(np_set.chanlen, np_set.freqlen) {
    size_t freqlen = np_set.freqlen;
    size_t chanlen = np_set.chanlen;
    size_t eventlen = np_set.eventlen;

    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j].SetSize(eventlen);
      }
    }
  }

  /// Reset all of the values back to 0
  void NormalizePowers::Reset() {
    size_t freqlen = np_set.freqlen;
    size_t chanlen = np_set.chanlen;

    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j].Reset();
      }
    }
  }

  /// Update the rolling statistics with a new set of values
  /** @param The new values to be added to the rolling statics
   */
  void NormalizePowers::Update(RC::APtr<const EEGPowers>& new_data, RC::Ptr<EventLog> event_log) {
    auto& new_datar = new_data->data;
    size_t freqlen = np_set.freqlen;
    size_t chanlen = np_set.chanlen;

    RC::Data1D<RC::Data1D<RC::Data1D<double>>> means(freqlen);
    RC::Data1D<RC::Data1D<RC::Data1D<double>>> sample_std_devs(freqlen);

    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      means[i].Resize(chanlen);
      sample_std_devs[i].Resize(chanlen);
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j].Update(new_datar[i][j]);

        try { // GetStats errors on first update (can't take sample std dev of 1 value)
          if (event_log.IsSet()) {
            auto stats = rolling_powers[i][j].GetStats();
            means[i][j] += stats.means;
            sample_std_devs[i][j] += stats.sample_std_devs;
          }
        } catch (RC::ErrorMsg& err) {}
      }
    }

//    if (event_log.IsSet()) {
//      JSONFile normalization_stats;
//      normalization_stats.Set(means, "means");
//      normalization_stats.Set(sample_std_devs, "sample_std_devs");
//      event_log->Log(MakeResp("NORMALIZATION_STATS", 0, normalization_stats).Line());
//    }
  }

  /// Z-score the powers with the current statistics
  /** @param The powers to be z-scored
   */
  RC::APtr<EEGPowers> NormalizePowers::ZScore(RC::APtr<const EEGPowers>& in_data, bool div_by_zero_eq_zero) {
    auto& in_datar = in_data->data;
    size_t freqlen = np_set.freqlen;
    size_t chanlen = np_set.chanlen;
    size_t eventlen = np_set.eventlen;

    if ( (eventlen != in_datar.size1()) ||
         (chanlen != in_datar.size2()) ||
         (freqlen != in_datar.size3()) ) {
      Throw_RC_Error((RC::RStr("NormalizePowersSettings dimensions (") + freqlen + ", " + chanlen + ", " +
                               eventlen + ") " + "and in_data dimensions (" + in_datar.size3() +
                               ", " + in_datar.size2() + ", " + in_datar.size1() + ") do not match.").c_str());
    }

    RC::APtr<EEGPowers> out_data = new EEGPowers(in_data->sampling_rate, 1, chanlen, freqlen);
    auto& out_datar = out_data->data;

    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        out_datar[i][j] = rolling_powers[i][j].ZScore(in_datar[i][j], div_by_zero_eq_zero);
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
  //      out_data[i][j] = rolling_powers[i][j].GetStats();
  //    }
  //  }
  //  return out_data;
  //}

  void NormalizePowers::PrintStats() {
    size_t freqlen = np_set.freqlen;
    size_t chanlen = np_set.chanlen;
    PrintStats(freqlen, chanlen);
  }

  void NormalizePowers::PrintStats(size_t num_freqs) {
    size_t freqlen = num_freqs;
    size_t chanlen = np_set.chanlen;
    PrintStats(freqlen, chanlen);
  }

  void NormalizePowers::PrintStats(size_t num_freqs, size_t num_chans) {
    size_t freqlen = num_freqs;
    size_t chanlen = num_chans;
    if (freqlen > rolling_powers.size2()) {
      Throw_RC_Error((RC::RStr("The num_freqs (") + freqlen +
            ") is longer than then number of freqs in powers (" + rolling_powers.size2() + ")").c_str());
    } else if (chanlen > rolling_powers.size1()) {
      Throw_RC_Error((RC::RStr("The num_chans (") + chanlen +
            ") is longer than then number of freqs in powers (" + rolling_powers.size1() + ")").c_str());
    }

    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        rolling_powers[i][j].PrintStats();
      }
    }
  }
}
