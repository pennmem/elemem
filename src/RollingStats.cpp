#include "RollingStats.h"
#include <cmath>
#include "RC/RStr.h"

namespace CML {
  /// Constructor that initializes and resets the internal lists
  /** @param the number of values in each list
   */
  RollingStats::RollingStats(size_t num_values) {
    SetSize(num_values);
  }

  /// Get the size of the internal Data1Ds
  /** @return The size of the internal Data1Ds
   */
  size_t RollingStats::size() {
    return means.size();
  }

  /// Set the count of means and stddevs to track and reset.
  void RollingStats::SetSize(size_t num_values) {
    means.Resize(num_values);
    m2s.Resize(num_values);
    Reset();
  }

  /// Reset all of the values back to 0
  void RollingStats::Reset() {
    count = 0;
    means.Zero();
    m2s.Zero();
  }

  /// Update the rolling statistics with a new set of values
  /** @param The new values to be added to the rolling statics
   */
  void RollingStats::Update(const RC::Data1D<double>& new_values) {
    if (new_values.size() != means.size())
      Throw_RC_Type(Bounds, (RC::RStr("Data1D size mismatch between new_values (") + new_values.size() + ") and means (" + means.size() + ")").c_str());
    count += 1;
    RC_ForIndex(i, new_values) {
      double delta = new_values[i] - means[i];
      means[i] += delta / count;
      double delta2 = new_values[i] - means[i];
      m2s[i] += delta * delta2;
    }
  }

  /// Z-score the data with the current statistics
  /** @param The data to be z-scored
   */
  RC::Data1D<double> RollingStats::ZScore(const RC::Data1D<double>& data, bool div_by_zero_eq_zero) {
    if (data.size() != means.size())
      Throw_RC_Type(Bounds, (RC::RStr("Data1D size mismatch between data (") + data.size() + ") and means (" + means.size() + ")").c_str());
    RC::Data1D<double> zscored_data(data.size());
    StatsData stats = GetStats();
    RC_ForIndex(i, data) {
      if (div_by_zero_eq_zero && stats.sample_std_devs[i] == 0)
        zscored_data[i] = 0;
      else
        zscored_data[i] = (data[i] - stats.means[i]) / stats.sample_std_devs[i];
    }
    return zscored_data;
  }

  /// Returns the current statistics from the collected data
  /** @return The current statistics
   */
  StatsData RollingStats::GetStats() {
    if (count <= 1) {
      Throw_RC_Type(Bounds, "Cannot calculate statistics on fewer than 2 "
          "elements");
    }
    RC::Data1D<double> sample_std_dev(m2s.size());
    RC_ForIndex(i, m2s) {
      sample_std_dev[i] = std::sqrt(m2s[i] / (count - 1));
    }
    return StatsData {means, sample_std_dev};
  }

  void RollingStats::PrintStats() {
    StatsData stats_data = GetStats();
    auto rstr = "\nmeans: " + RC::RStr::Join(stats_data.means, ", ") + "\n";
    rstr += "sample_std_devs: " + RC::RStr::Join(stats_data.sample_std_devs, ", ") + "\n";
    std::cerr << rstr << std::endl;
  }
}
