#include "Testing.h"
#include "FeatureFilters.h"
#include "ChannelConf.h"
#include "TaskClassifierManager.h"
#include "EEGCircularData.h"
#include "RollingStats.h"
#include "NormalizePowers.h"

namespace CML {
  // Helper Functions
  RC::APtr<const EEGData> CreateTestingEEGData() {
    return CreateTestingEEGData(42);
  }

  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate) {
    return CreateTestingEEGData(sampling_rate, 4, 3);
  }

  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate, size_t eventlen, size_t chanlen) {
    return CreateTestingEEGData(sampling_rate, eventlen, chanlen, 0); 
  }
  
  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate, size_t eventlen, size_t chanlen, int16_t offset) {
    RC::APtr<EEGData> data = new EEGData(sampling_rate, eventlen);
    auto& datar = data->data;
  
    datar.Resize(chanlen);
    RC_ForIndex(i, datar) {
      data->EnableChan(i);
      RC_ForIndex(j, datar[i]) {
        datar[i][j] = i*eventlen + j + offset;
      }
    }
  
    return data.ExtractConst();
  }
  
  RC::APtr<const EEGPowers> CreateTestingEEGPowers() {
    return CreateTestingEEGPowers(42);
  }

  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate) {
    return CreateTestingEEGPowers(sampling_rate, 4, 3, 2);
  }

  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate, size_t eventlen, size_t chanlen, size_t freqlen) {
    return CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen, 0);
  }

  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate, size_t eventlen, size_t chanlen, size_t freqlen, int16_t offset) {
    RC::APtr<EEGPowers> powers = new EEGPowers(sampling_rate, eventlen, chanlen, freqlen);
    auto& datar = powers->data;
  
    RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
      RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
        RC_ForRange(k, 0, eventlen) { // Iterate over eventlen
          datar[i][j][k] = i*chanlen*eventlen + j*eventlen + k + offset;
        }
      }
    }
  
    return powers.ExtractConst();
  }

  // Data Storing and Binning
  void TestEEGCircularData() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGData> in_data = CreateTestingEEGData(sampling_rate, 4, 5);
    PrintEEGData(*in_data);

    EEGCircularData circular_data(sampling_rate, 10);
    circular_data.Append(in_data);
    circular_data.PrintData();
    //circular_data.PrintRawData();
    circular_data.Append(in_data);
    circular_data.PrintData();
    //circular_data.PrintRawData();
    circular_data.Append(in_data);
    circular_data.PrintData();
    //circular_data.PrintRawData();
  }

  void TestEEGBinning() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGData> in_data = CreateTestingEEGData(sampling_rate, 11, 3);

    RC::APtr<EEGData> out_data = EEGCircularData::BinData(in_data, 3);

    PrintEEGData(*in_data);
    PrintEEGData(*out_data);
  }

  // Feature Filters
  void TestBipolarReference() {
    RC::APtr<const EEGData> in_data = CreateTestingEEGData();
  
    RC::Data1D<BipolarPair> bipolar_reference_channels = {BipolarPair{0,1}, BipolarPair{1,0}, BipolarPair{0,2}, BipolarPair{2,1}}; 
    RC::RStr deb_msg = "";
    RC_ForEach(pair, bipolar_reference_channels) {
      deb_msg += ("(" + RC::RStr(pair.pos) + ", " + RC::RStr(pair.neg) + "), ");
    }
    deb_msg += "\n";
    RC_DEBOUT(deb_msg);

    RC::APtr<EEGData> out_data = FeatureFilters::BipolarReference(in_data, bipolar_reference_channels);
  
    PrintEEGData(*in_data);
    PrintEEGData(*out_data);
  }
  
  void TestMirrorEnds() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGData> in_data = CreateTestingEEGData(sampling_rate);
    
    RC::APtr<EEGData> out_data = FeatureFilters::MirrorEnds(in_data, 2);

    RC_DEBOUT(in_data->data[0].size());
    RC_DEBOUT(out_data->data[0].size());
    PrintEEGData(*in_data);
    PrintEEGData(*out_data);
  }
  
  void TestAvgOverTime() {
    // TODO: JPB: Add test for inf, -inf, and nan
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();
  
    RC::APtr<EEGPowers> out_powers = FeatureFilters::AvgOverTime(in_powers, false);
  
    PrintEEGPowers(*in_powers);
    PrintEEGPowers(*out_powers);
  }

  void TestLog10Transform() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();

    RC::APtr<EEGPowers> out_powers = FeatureFilters::Log10Transform(in_powers, 1e-16);
  
    PrintEEGPowers(*in_powers);
    PrintEEGPowers(*out_powers);
  }

  void TestMorletTransformer() {
    size_t sampling_rate = 1000;
    size_t num_events = 10;
    RC::Data1D<BipolarPair> channels = {BipolarPair{0,1}, BipolarPair{1,0}, BipolarPair{0,2}};
    RC::Data1D<double> freqs = {1000, 500};
    RC::APtr<const EEGData> in_data = CreateTestingEEGData(sampling_rate, num_events, channels.size());
    
    MorletSettings mor_set;
    mor_set.channels = channels;
    mor_set.frequencies = freqs;
    mor_set.sampling_rate = sampling_rate;

    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(mor_set);
    RC::APtr<EEGPowers> out_powers = morlet_transformer.Filter(in_data, num_events);

    PrintEEGData(*in_data);
    PrintEEGPowers(*out_powers);
  }

  void TestMorletTransformerRealData() {
    size_t sampling_rate = 1000;
    RC::Data1D<BipolarPair> channels = {BipolarPair{0,1}}; // This means nothing
    RC::Data1D<double> freqs = {6, 9.75368155833899, 15.8557173235803, 25.7752696088736, 41.900628640881, 68.1142314762286, 110.727420568354, 180};
    RC::Data1D real_data = {1715, 1685, 1652, 1617, 1580, 1540, 1499, 1456, 1411, 1365, 1316, 1266, 1216, 1164, 1110, 1056, 1002, 946, 890, 833, 777, 720, 662, 606, 550, 492, 550, 606, 662, 720, 777, 833, 890, 946, 1002, 1056, 1110, 1164, 1216, 1266, 1316, 1365, 1411, 1456, 1499, 1540, 1580, 1617, 1652, 1685, 1715, 1742, 1768, 1791, 1811, 1828, 1843, 1854, 1862, 1868, 1870, 1870, 1866, 1860, 1850, 1837, 1822, 1803, 1781, 1757, 1729, 1698, 1664, 1628, 1589, 1547, 1502, 1456, 1407, 1355, 1300, 1244, 1186, 1124, 1063, 999, 933, 866, 797, 728, 657, 585, 513, 439, 364, 290, 216, 142, 67, -8, -82, -157, -230, -303, -374, -444, -514, -583, -650, -716, -779, -842, -903, -962, -1018, -1072, -1124, -1174, -1222, -1267, -1311, -1351, -1388, -1422, -1455, -1483, -1509, -1533, -1553, -1571, -1586, -1598, -1607, -1613, -1617, -1617, -1615, -1610, -1602, -1592, -1579, -1563, -1546, -1525, -1503, -1479, -1452, -1423, -1392, -1359, -1325, -1288, -1251, -1213, -1172, -1130, -1088, -1045, -1001, -956, -911, -865, -819, -773, -727, -681, -633, -587, -542, -498, -453, -409, -366, -325, -284, -244, -207, -169, -134, -99, -66, -35, -6, 21, 47, 71, 93, 113, 131, 147, 161, 173, 184, 192, 199, 203, 204, 205, 203, 200, 194, 187, 179, 168, 156, 143, 127, 111, 93, 75, 55, 34, 11, -12, -35, -59, -83, -109, -135, -161, -186, -211, -236, -262, -287, -262, -236, -211, -186, -161, -135, -109, -83, -59, -35, -12, 11, 34, 55, 75, 93, 111, 127, 143, 156, 168, 179, 187, 194, 200};
    size_t num_events = real_data.size();
    EEGData eeg_data(sampling_rate, num_events);
    eeg_data.data.Resize(1);
    eeg_data.data[0].CopyFrom(real_data);
    RC::APtr<const EEGData> in_data = RC::MakeAPtr<const EEGData>(eeg_data);
    
    MorletSettings mor_set;
    mor_set.channels = channels;
    mor_set.frequencies = freqs;
    mor_set.sampling_rate = sampling_rate;

    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(mor_set);
    RC::APtr<EEGPowers> out_powers = morlet_transformer.Filter(in_data, num_events);

    PrintEEGData(*in_data);
    PrintEEGPowers(*out_powers);
  }

  void TestRollingStats() {
    size_t sampling_rate = 1000;
    size_t eventlen = 10;
    size_t chanlen = 5;
    size_t freqlen = 1;
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen);
    PrintEEGPowers(*in_powers);

    RollingStats rolling_stats(eventlen);
    rolling_stats.Update(in_powers->data[0][0]);
    rolling_stats.PrintStats();
    rolling_stats.Update(in_powers->data[0][2]);
    rolling_stats.PrintStats();

    auto out_data = rolling_stats.ZScore(in_powers->data[0][4], true);
    RC_DEBOUT(RC::RStr::Join(out_data, ", ") + "\n");

    // means should be 10 through 19
    // std_devs should be 10
    // sample_std_devs should be 14.1421...
    // zscores should be 2.1213...

    // TODO: JPB: (test) Add test for std_dev = 0, inf, -inf, nan
  }

  void TestNormalizePowers() {
    size_t sampling_rate = 1000;
    size_t eventlen = 1;
    size_t chanlen = 2;
    size_t freqlen = 2;
    RC::APtr<const EEGPowers> in_powers1 = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen, 0);
    PrintEEGPowers(*in_powers1);
    RC::APtr<const EEGPowers> in_powers2 = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen, 20);
    PrintEEGPowers(*in_powers2);
    RC::APtr<const EEGPowers> in_powers3 = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen, 40);
    PrintEEGPowers(*in_powers3);

    NormalizePowersSettings np_set = { eventlen, chanlen, freqlen };
    NormalizePowers normalize_powers(np_set);
    normalize_powers.Update(in_powers1);
    normalize_powers.PrintStats();
    normalize_powers.Update(in_powers2);
    normalize_powers.PrintStats();

    RC::APtr<EEGPowers> out_powers = normalize_powers.ZScore(in_powers3, true);
    PrintEEGPowers(*out_powers);
  }

  void TestAllCode() {
    //TestLog10Transform();
    //TestAvgOverTime();
    //TestMirrorEnds();
    //TestBipolarReference();
    TestMorletTransformer();
    //TestMorletTransformerRealData();
    //TestEEGCircularData();
    //TestEEGBinning();
    //TestRollingStats();
    //TestNormalizePowers();
  }
}

