#include "Testing.h"
#include "FeatureFilters.h"
#include "ChannelConf.h"
#include "TaskClassifierManager.h"
#include "EEGCircularData.h"
#include "RollingStats.h"
#include "NormalizePowers.h"

namespace CML {
  // Helper Functions
  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw() {
    return CreateTestingEEGDataRaw(42);
  }

  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw(size_t sampling_rate) {
    return CreateTestingEEGDataRaw(sampling_rate, 4, 3);
  }

  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw(size_t sampling_rate, size_t eventlen, size_t chanlen) {
    return CreateTestingEEGDataRaw(sampling_rate, eventlen, chanlen, 0); 
  }
  
  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw(size_t sampling_rate, size_t eventlen, size_t chanlen, int16_t offset) {
    RC::APtr<EEGDataRaw> data = new EEGDataRaw(sampling_rate, eventlen);
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

  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble() {
    return CreateTestingEEGDataDouble(42);
  }

  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble(size_t sampling_rate) {
    return CreateTestingEEGDataDouble(sampling_rate, 4, 3);
  }

  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble(size_t sampling_rate, size_t eventlen, size_t chanlen) {
    return CreateTestingEEGDataDouble(sampling_rate, eventlen, chanlen, 0); 
  }
  
  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble(size_t sampling_rate, size_t eventlen, size_t chanlen, int16_t offset) {
    RC::APtr<EEGDataDouble> data = new EEGDataDouble(sampling_rate, eventlen);
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
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 4, 5);
    in_data->Print();

    EEGCircularData circular_data(sampling_rate, 10);
    circular_data.Append(in_data);
    circular_data.PrintData();
    circular_data.GetData(5)->Print();
    circular_data.GetDataAllAsTimeline()->Print();
    //circular_data.PrintRawData();
    circular_data.Append(in_data);
    circular_data.PrintData();
    circular_data.GetData(5)->Print();
    //circular_data.PrintRawData();
    circular_data.Append(in_data);
    circular_data.PrintData();
    circular_data.GetData(5)->Print();
    //circular_data.PrintRawData();

    //circular_data.GetData()->Print();
    //circular_data.GetData(5)->Print();
  }

  //void TestEEGBinning() {
  //  size_t sampling_rate = 10;
  //  RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 11, 3);

  //  RC::APtr<EEGDataRaw> out_data = FeatureFilters::BinData(in_data, 3);

  //  in_data->Print();
  //  out_data->Print();
  //}

  void TestEEGBinning1() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(in_data, 3);

    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }
  
  void TestEEGBinning2() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 10, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(in_data, 3);

    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinning3() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 11, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(in_data, 3);

    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover1() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 0, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover2() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 1, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover3() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 2, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover4() {
    size_t sampling_rate = 10;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 3, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  } 

  // Feature Filters
  void TestBipolarReference() {
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw();
  
    RC::Data1D<BipolarPair> bipolar_reference_channels = {BipolarPair{0,1}, BipolarPair{1,0}, BipolarPair{0,2}, BipolarPair{2,1}}; 
    RC::RStr deb_msg = "";
    RC_ForEach(pair, bipolar_reference_channels) {
      deb_msg += ("(" + RC::RStr(pair.pos) + ", " + RC::RStr(pair.neg) + "), ");
    }
    deb_msg += "\n";
    RC_DEBOUT(deb_msg);

    RC::APtr<EEGDataDouble> out_data = FeatureFilters::BipolarReference(in_data, bipolar_reference_channels);
  
    in_data->Print();
    out_data->Print();
  }
  
  void TestDifferentiate() {
    RC::Data1D<double> in_data = {2.0, 2, 4, 7, 0};
    
    RC::Data1D<double> out_data1 = FeatureFilters::Differentiate(in_data, 1);
    RC::Data1D<double> out_data2 = FeatureFilters::Differentiate(in_data, 2);
    RC::Data1D<double> out_data3 = FeatureFilters::Differentiate(in_data, 3);

    RC_DEBOUT(RC::RStr::Join(in_data, ", ") + "\n");
    RC_DEBOUT(RC::RStr::Join(out_data1, ", ") + "\n");
    RC_DEBOUT(RC::RStr::Join(out_data2, ", ") + "\n");
    RC_DEBOUT(RC::RStr::Join(out_data3, ", ") + "\n");
  }

  void TestFindArtifactChannels() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGDataDouble> in_data = CreateTestingEEGDataDouble(sampling_rate, 50, 2);

    RC::APtr<RC::Data1D<bool>> out_data = FeatureFilters::FindArtifactChannels(in_data, 10, 10);

    in_data->Print();
    RC_DEBOUT(RC::RStr::Join(*out_data, ", ") + "\n");
  }

  void TestMirrorEnds() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGDataDouble> in_data = CreateTestingEEGDataDouble(sampling_rate);
    
    RC::APtr<EEGDataDouble> out_data = FeatureFilters::MirrorEnds(in_data, 2);

    RC_DEBOUT(in_data->data[0].size());
    RC_DEBOUT(out_data->data[0].size());
    in_data->Print();
    out_data->Print();
  }

  void TestRemoveMirrorEnds() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGPowers> in_data = CreateTestingEEGPowers(sampling_rate, 10, 2, 2);
    
    RC::APtr<EEGPowers> out_data = FeatureFilters::RemoveMirrorEnds(in_data, 2);

    RC_DEBOUT(in_data->data.size1());
    RC_DEBOUT(out_data->data.size1());
    in_data->Print();
    out_data->Print();
  }
  
  void TestAvgOverTime() {
    // TODO: JPB: Add test for inf, -inf, and nan
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();
  
    RC::APtr<EEGPowers> out_powers = FeatureFilters::AvgOverTime(in_powers, false);
  
    in_powers->Print();
    out_powers->Print();
  }

  void TestLog10Transform() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();

    RC::APtr<EEGPowers> out_powers = FeatureFilters::Log10Transform(in_powers, 1e-16);
  
    in_powers->Print();
    out_powers->Print();
  }

  void TestLog10TransformWithEpsilon() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();

    RC::APtr<EEGPowers> out_powers = FeatureFilters::Log10Transform(in_powers, 1e-16, true);
  
    in_powers->Print();
    out_powers->Print();
  }

  void TestMorletTransformer() {
    size_t sampling_rate = 1000;
    size_t num_events = 10;
    RC::Data1D<BipolarPair> channels = {BipolarPair{0,1}, BipolarPair{1,0}, BipolarPair{0,2}};
    RC::Data1D<double> freqs = {1000, 500};
    RC::APtr<const EEGDataDouble> in_data = CreateTestingEEGDataDouble(sampling_rate, num_events, channels.size());
    
    MorletSettings mor_set;
    mor_set.channels = channels;
    mor_set.frequencies = freqs;
    mor_set.sampling_rate = sampling_rate;

    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(mor_set);
    RC::APtr<EEGPowers> out_powers = morlet_transformer.Filter(in_data);

    in_data->Print();
    out_powers->Print();
  }

  void TestMorletTransformerRealData() {
    size_t sampling_rate = 1000;
    RC::Data1D<BipolarPair> channels = {BipolarPair{0,1}}; // This means nothing
    RC::Data1D<double> freqs = {6, 9.75368155833899, 15.8557173235803, 25.7752696088736, 41.900628640881, 68.1142314762286, 110.727420568354, 180};
    RC::Data1D real_data = {1715, 1685, 1652, 1617, 1580, 1540, 1499, 1456, 1411, 1365, 1316, 1266, 1216, 1164, 1110, 1056, 1002, 946, 890, 833, 777, 720, 662, 606, 550, 492, 550, 606, 662, 720, 777, 833, 890, 946, 1002, 1056, 1110, 1164, 1216, 1266, 1316, 1365, 1411, 1456, 1499, 1540, 1580, 1617, 1652, 1685, 1715, 1742, 1768, 1791, 1811, 1828, 1843, 1854, 1862, 1868, 1870, 1870, 1866, 1860, 1850, 1837, 1822, 1803, 1781, 1757, 1729, 1698, 1664, 1628, 1589, 1547, 1502, 1456, 1407, 1355, 1300, 1244, 1186, 1124, 1063, 999, 933, 866, 797, 728, 657, 585, 513, 439, 364, 290, 216, 142, 67, -8, -82, -157, -230, -303, -374, -444, -514, -583, -650, -716, -779, -842, -903, -962, -1018, -1072, -1124, -1174, -1222, -1267, -1311, -1351, -1388, -1422, -1455, -1483, -1509, -1533, -1553, -1571, -1586, -1598, -1607, -1613, -1617, -1617, -1615, -1610, -1602, -1592, -1579, -1563, -1546, -1525, -1503, -1479, -1452, -1423, -1392, -1359, -1325, -1288, -1251, -1213, -1172, -1130, -1088, -1045, -1001, -956, -911, -865, -819, -773, -727, -681, -633, -587, -542, -498, -453, -409, -366, -325, -284, -244, -207, -169, -134, -99, -66, -35, -6, 21, 47, 71, 93, 113, 131, 147, 161, 173, 184, 192, 199, 203, 204, 205, 203, 200, 194, 187, 179, 168, 156, 143, 127, 111, 93, 75, 55, 34, 11, -12, -35, -59, -83, -109, -135, -161, -186, -211, -236, -262, -287, -262, -236, -211, -186, -161, -135, -109, -83, -59, -35, -12, 11, 34, 55, 75, 93, 111, 127, 143, 156, 168, 179, 187, 194, 200};
    size_t num_events = real_data.size();
    RC::APtr<EEGDataDouble> in_data = RC::MakeAPtr<EEGDataDouble>(sampling_rate, num_events);
    in_data->data.Resize(1);
    in_data->data[0].CopyFrom(real_data);
    
    MorletSettings mor_set;
    mor_set.channels = channels;
    mor_set.frequencies = freqs;
    mor_set.sampling_rate = sampling_rate;

    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(mor_set);
    auto in_data_captr = in_data.ExtractConst();
    RC::APtr<EEGPowers> out_powers = morlet_transformer.Filter(in_data_captr);

    in_data->Print();
    out_powers->Print();
  }

  void TestRollingStats() {
    size_t sampling_rate = 1000;
    size_t eventlen = 10;
    size_t chanlen = 5;
    size_t freqlen = 1;
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen);
    in_powers->Print();

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
    in_powers1->Print();
    RC::APtr<const EEGPowers> in_powers2 = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen, 20);
    in_powers2->Print();
    RC::APtr<const EEGPowers> in_powers3 = CreateTestingEEGPowers(sampling_rate, eventlen, chanlen, freqlen, 40);
    in_powers3->Print();

    NormalizePowersSettings np_set = { eventlen, chanlen, freqlen };
    NormalizePowers normalize_powers(np_set);
    normalize_powers.Update(in_powers1);
    normalize_powers.PrintStats();
    normalize_powers.Update(in_powers2);
    normalize_powers.PrintStats();

    RC::APtr<EEGPowers> out_powers = normalize_powers.ZScore(in_powers3, true);
    out_powers->Print();
  }

  void TestProcess_Handler() {
    size_t sampling_rate = 1000;
    size_t chanlen = 1;
    size_t eventlen = 50;
    RC::APtr<const EEGDataDouble> in_data = CreateTestingEEGDataDouble(sampling_rate, eventlen, chanlen); 

    MorletSettings morlet_settings = {5, {500}, {{0,0}}, 1000, 2, true};
    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(morlet_settings);
    const double log_min_power_clamp = 1e-16;

    size_t mirroring_duration_ms = 20; 
    //size_t mirroring_duration_ms = morlet_transformer.CalcAvgMirroringDurationMs();

    auto bipolar_ref_data = in_data;
    //auto bipolar_ref_data = FeatureFilters::BipolarReference(data, bipolar_reference_channels).ExtractConst();
    auto mirrored_data = FeatureFilters::MirrorEnds(bipolar_ref_data, mirroring_duration_ms).ExtractConst();
    auto morlet_data = morlet_transformer.Filter(mirrored_data).ExtractConst();
    auto unmirrored_data = FeatureFilters::RemoveMirrorEnds(morlet_data, mirroring_duration_ms).ExtractConst();
    auto log_data = FeatureFilters::Log10Transform(unmirrored_data, log_min_power_clamp).ExtractConst();
    auto avg_data = FeatureFilters::AvgOverTime(log_data, true).ExtractConst();

    in_data->Print();
    bipolar_ref_data->Print();
    mirrored_data->Print();
    morlet_data->Print();
    unmirrored_data->Print();
    log_data->Print();
    avg_data->Print();
  }


  void TestAllCode() {
    //TestLog10Transform();
    TestLog10TransformWithEpsilon();
    //TestAvgOverTime();
    //TestMirrorEnds();
    //TestRemoveMirrorEnds();
    //TestBipolarReference();
    //TestMorletTransformer();
    //TestMorletTransformerRealData();
    //TestEEGCircularData();
    //TestEEGBinning1();
    //TestEEGBinning2();
    //TestEEGBinning3();
    //TestEEGBinningRollover1();
    //TestEEGBinningRollover2();
    //TestEEGBinningRollover3();
    //TestEEGBinningRollover4();
    //TestRollingStats();
    //TestNormalizePowers();
    //TestFindArtifactChannels();
    //TestDifferentiate();
    //TestProcess_Handler();
  }
}

