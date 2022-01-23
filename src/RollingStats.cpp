#include "RollingStats.h"
#include <cmath>
#include "RC/RStr.h"

namespace CML {
  /// Default constructor that initializes and resets the internal lists
  /** @param the number of values in each list
   */
  RollingStats::RollingStats(size_t num_values) {
    means.Resize(num_values);
    m2s.Resize(num_values);
    Reset();
  }

  /// Reset all of the values back to 0 
  void RollingStats::Reset() {
    count = 0;
    RC_ForEach(mean, means) { mean = 0; }
    RC_ForEach(m2, m2s) { m2 = 0; }
  }

  /// Update the rolling statistics with a new set of values
  /** @param The new values to be added to the rolling statics
   */
  void RollingStats::Update(const RC::Data1D<double>& new_values) {
    if (new_values.size() != means.size())
      Throw_RC_Type(Bounds, "Data1D size mismatch between new_values and means");
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
  RC::Data1D<double> RollingStats::ZScore(const RC::Data1D<double>& data) {
    if (data.size() != means.size())
      Throw_RC_Type(Bounds, "Data1D size mismatch between data and means");
    RC::Data1D<double> zscored_data(data.size());
    StatsData stats = GetStats();
    RC_ForIndex(i, data) {
      zscored_data[i] = (data[i] - stats.means[i]) / stats.sample_std_devs[i];
    }
    return zscored_data;
  }

  /// Returns the current statistics from the collected data
  /** @return The current statistics
   */
  StatsData RollingStats::GetStats() {
    RC::Data1D<double> std_dev(m2s.size());
    RC::Data1D<double> sample_std_dev(m2s.size());
    RC_ForIndex(i, m2s) {
      std_dev[i] = std::sqrt(m2s[i] / count);
      sample_std_dev[i] = std::sqrt(m2s[i] / (count - 1));
    }
    return StatsData {means, std_dev, sample_std_dev};
  }

  void RollingStats::PrintStats() {
	StatsData stats_data = GetStats();
	auto rstr = "means: " + RC::RStr::Join(stats_data.means, ", ") + "\n";
	rstr += "std_devs: " + RC::RStr::Join(stats_data.std_devs, ", ") + "\n";
	rstr += "sample_std_devs: " + RC::RStr::Join(stats_data.sample_std_devs, ", ") + "\n";
	RC_DEBOUT(rstr);
  }
}
