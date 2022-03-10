#include "PythonInterface.h"
#include "Testing.h"
#include "FeatureFilters.h"
#include "ChannelConf.h"
#include "TaskClassifierManager.h"
#include "EEGCircularData.h"
#include "RollingStats.h"
#include "NormalizePowers.h"
#include "ClassifierLogReg.h"
#include "WeightManager.h"
#include "Handler.h"


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
    size_t sampling_rate = 9;
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(in_data, 3);

    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }
  
  void TestEEGBinning2() {
    size_t sampling_rate = 9;
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 10, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(in_data, 3);

    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinning3() {
    size_t sampling_rate = 9;
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 11, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(in_data, 3);

    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover1() {
    size_t sampling_rate = 9;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 0, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover2() {
    size_t sampling_rate = 9;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 1, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover3() {
    size_t sampling_rate = 9;
    RC::APtr<const EEGDataRaw> rollover_data = CreateTestingEEGDataRaw(sampling_rate, 2, 3);
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, 9, 3);

    RC::APtr<BinnedData> binned_data = FeatureFilters::BinData(rollover_data, in_data, 3);

    rollover_data->Print();
    in_data->Print();
    binned_data->out_data->Print();
    binned_data->leftover_data->Print();
  }

  void TestEEGBinningRollover4() {
    size_t sampling_rate = 9;
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

  void TestFindArtifactChannelsRandomData() {
    RC::Data1D real_data = {23397, 18883, -2134, 6578, 11249, -19618, -6910, 31186, 9828, -8683, -20091, 1888, 17429, -31006, -20021, 16368, -28820, -15500, -29240, 20544, -30586, 14159, -31452, 27411, -21918, -20790, 22078, 10876, 10224, -3028, -30190, -22670, -8015, -17617, 26520, 27338, -25771, -15178, -9305, -24992, -5502, -26735, 483, 716, 30799, -27442, 18557, -14319, -6563, -8249};
    size_t sampling_rate = 1000;
    size_t eventlen = real_data.size();
    size_t chanlen = 1;
    RC::APtr<EEGDataDouble> in_data = RC::MakeAPtr<EEGDataDouble>(sampling_rate, eventlen);
    in_data->data.Resize(chanlen);
    in_data->data[0].CopyFrom(real_data);
    auto in_data_captr = in_data.ExtractConst();

    RC::APtr<RC::Data1D<bool>> out_data = FeatureFilters::FindArtifactChannels(in_data_captr, 10, 10);

    in_data_captr->Print();
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
    RC::Data1D<BipolarPair> channels = {BipolarPair{0,1}}; // This means nothing
    RC::Data1D<double> freqs = {6, 9.75368155833899, 15.8557173235803, 25.7752696088736, 41.900628640881, 68.1142314762286, 110.727420568354, 180};
    RC::Data1D real_data = {1715, 1685, 1652, 1617, 1580, 1540, 1499, 1456, 1411, 1365, 1316, 1266, 1216, 1164, 1110, 1056, 1002, 946, 890, 833, 777, 720, 662, 606, 550, 492, 550, 606, 662, 720, 777, 833, 890, 946, 1002, 1056, 1110, 1164, 1216, 1266, 1316, 1365, 1411, 1456, 1499, 1540, 1580, 1617, 1652, 1685, 1715, 1742, 1768, 1791, 1811, 1828, 1843, 1854, 1862, 1868, 1870, 1870, 1866, 1860, 1850, 1837, 1822, 1803, 1781, 1757, 1729, 1698, 1664, 1628, 1589, 1547, 1502, 1456, 1407, 1355, 1300, 1244, 1186, 1124, 1063, 999, 933, 866, 797, 728, 657, 585, 513, 439, 364, 290, 216, 142, 67, -8, -82, -157, -230, -303, -374, -444, -514, -583, -650, -716, -779, -842, -903, -962, -1018, -1072, -1124, -1174, -1222, -1267, -1311, -1351, -1388, -1422, -1455, -1483, -1509, -1533, -1553, -1571, -1586, -1598, -1607, -1613, -1617, -1617, -1615, -1610, -1602, -1592, -1579, -1563, -1546, -1525, -1503, -1479, -1452, -1423, -1392, -1359, -1325, -1288, -1251, -1213, -1172, -1130, -1088, -1045, -1001, -956, -911, -865, -819, -773, -727, -681, -633, -587, -542, -498, -453, -409, -366, -325, -284, -244, -207, -169, -134, -99, -66, -35, -6, 21, 47, 71, 93, 113, 131, 147, 161, 173, 184, 192, 199, 203, 204, 205, 203, 200, 194, 187, 179, 168, 156, 143, 127, 111, 93, 75, 55, 34, 11, -12, -35, -59, -83, -109, -135, -161, -186, -211, -236, -262, -287, -262, -236, -211, -186, -161, -135, -109, -83, -59, -35, -12, 11, 34, 55, 75, 93, 111, 127, 143, 156, 168, 179, 187, 194, 200};
    size_t sampling_rate = 1000;
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
    try {
      rolling_stats.PrintStats();
      Throw_RC_Error("Failed to trigger exception on nan stddev.");
    }
    catch (RC::ErrorMsgBounds& ex) {
      // Expected test exception.
    }
    rolling_stats.Update(in_powers->data[0][2]);
    rolling_stats.PrintStats();
    auto out_data1 = rolling_stats.ZScore(in_powers->data[0][4], true);
    RC_DEBOUT(RC::RStr::Join(out_data1, ", ") + "\n");
    std::cout << "means should be 10 through 19, sample_std_devs "
          "should be 14.1421, zscores should be 2.1213\n" << std::endl;

    rolling_stats.Update(in_powers->data[0][3]);
    rolling_stats.PrintStats();
    rolling_stats.Update(in_powers->data[0][3]);
    rolling_stats.PrintStats();
    rolling_stats.Update(in_powers->data[0][3]);
    rolling_stats.PrintStats();

    auto out_data2 = rolling_stats.ZScore(in_powers->data[0][4], true);
    RC_DEBOUT(RC::RStr::Join(out_data2, ", ") + "\n");
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
    RC::APtr<const EEGDataRaw> in_data = CreateTestingEEGDataRaw(sampling_rate, eventlen, chanlen); 

    MorletSettings morlet_settings = {5, {500}, {{0,0}}, 1000, 2, true};
    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(morlet_settings);
    const double log_min_power_clamp = 1e-16;

    size_t mirroring_duration_ms = 20; 
    //size_t mirroring_duration_ms = morlet_transformer.CalcAvgMirroringDurationMs();

    auto bipolar_ref_data = FeatureFilters::BipolarSelector(in_data).ExtractConst();
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

  void TestProcess_HandlerRandomData() {
    RC::Data1D real_data = {23397, 18883, -2134, 6578, 11249, -19618, -6910, 31186, 9828, -8683, -20091, 1888, 17429, -31006, -20021, 16368, -28820, -15500, -29240, 20544, -30586, 14159, -31452, 27411, -21918, -20790, 22078, 10876, 10224, -3028, -30190, -22670, -8015, -17617, 26520, 27338, -25771, -15178, -9305, -24992, -5502, -26735, 483, 716, 30799, -27442, 18557, -14319, -6563, -8249};
    size_t sampling_rate = 1000;
    size_t eventlen = real_data.size();
    size_t chanlen = 1;
    RC::APtr<EEGDataRaw> in_data = RC::MakeAPtr<EEGDataRaw>(sampling_rate, eventlen);
    in_data->data.Resize(chanlen);
    in_data->data[0].CopyFrom(real_data);
    auto in_data_captr = in_data.ExtractConst();

    MorletSettings morlet_settings = {5, {500}, {{0,0}}, 1000, 2, true};
    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(morlet_settings);
    const double log_min_power_clamp = 1e-16;

    size_t mirroring_duration_ms = 20; 
    //size_t mirroring_duration_ms = morlet_transformer.CalcAvgMirroringDurationMs();

    auto bipolar_ref_data = FeatureFilters::BipolarSelector(in_data_captr).ExtractConst();
    //auto bipolar_ref_data = FeatureFilters::BipolarReference(data, bipolar_reference_channels).ExtractConst();
    auto mirrored_data = FeatureFilters::MirrorEnds(bipolar_ref_data, mirroring_duration_ms).ExtractConst();
    auto morlet_data = morlet_transformer.Filter(mirrored_data).ExtractConst();
    auto unmirrored_data = FeatureFilters::RemoveMirrorEnds(morlet_data, mirroring_duration_ms).ExtractConst();
    auto log_data = FeatureFilters::Log10Transform(unmirrored_data, log_min_power_clamp).ExtractConst();
    auto avg_data = FeatureFilters::AvgOverTime(log_data, true).ExtractConst();

    in_data_captr->Print();
    bipolar_ref_data->Print();
    mirrored_data->Print();
    morlet_data->Print();
    unmirrored_data->Print();
    log_data->Print();
    avg_data->Print();
  }

  void TestClassification() {
    // Load classifier weights
    RC::RStr config_path = "/Users/jbruska/Desktop/ElememConfigs/";
    RC::APtr<CSVFile> elecs = new CSVFile();
    elecs->Load(config_path + "R1384J_montage.csv");
    auto elec_config = elecs.ExtractConst();
    auto weight_manager = RC::MakeAPtr<WeightManager>(config_path + "converted_classifier_R1384J_session_0.json", elec_config);
    RC_DEBOUT(weight_manager->weights->intercept, weight_manager->weights->coef);

    // Setup classifier
    ClassifierLogRegSettings classifier_settings;
    auto classifier = ClassifierLogReg(new Handler(), classifier_settings, weight_manager->weights);

    // Create Features
    size_t sampling_rate = 1000;
    size_t freqlen = 8;
    size_t chanlen = 177;
    size_t eventlen = 1;

    //RC::Data3D in_data = {RC::Data2D{RC::Data1D{0.}, RC::Data1D{1.}}};
    RC::Data3D in_data = {RC::Data2D{RC::Data1D{0.4430485776800488}, RC::Data1D{0.27658665099963153}, RC::Data1D{0.03831340400009642}, RC::Data1D{0.8720454589960067}, RC::Data1D{0.703131244996522}, RC::Data1D{0.8658457599763941}, RC::Data1D{0.4625304081105338}, RC::Data1D{0.045224473753534}, RC::Data1D{0.05398763614409274}, RC::Data1D{0.7273316587441371}, RC::Data1D{0.7431849954583684}, RC::Data1D{0.9181616368585482}, RC::Data1D{0.8648674015270731}, RC::Data1D{0.8546446964144758}, RC::Data1D{0.672579661128185}, RC::Data1D{0.19151528456653533}, RC::Data1D{0.3501310564533682}, RC::Data1D{0.03505210954522353}, RC::Data1D{0.18384987895158034}, RC::Data1D{0.24006960847431547}, RC::Data1D{0.4474196334511584}, RC::Data1D{0.7443899549276032}, RC::Data1D{0.5409292102434938}, RC::Data1D{0.1491398750595304}, RC::Data1D{0.4657279881210047}, RC::Data1D{0.38197476757775195}, RC::Data1D{0.7470598491238367}, RC::Data1D{0.529722754358643}, RC::Data1D{0.9762989286759309}, RC::Data1D{0.18283650475594015}, RC::Data1D{0.6169250574500634}, RC::Data1D{0.7099562211469638}, RC::Data1D{0.0739918652926107}, RC::Data1D{0.056129116773352083}, RC::Data1D{0.3462323425763635}, RC::Data1D{0.12235078945703537}, RC::Data1D{0.39100886747699326}, RC::Data1D{0.17153054401570111}, RC::Data1D{0.5926854831383455}, RC::Data1D{0.14628181692049502}, RC::Data1D{0.4859938802065654}, RC::Data1D{0.05728889998855957}, RC::Data1D{0.10899339855826085}, RC::Data1D{0.9120707540854167}, RC::Data1D{0.6306641099856177}, RC::Data1D{0.31541978300499196}, RC::Data1D{0.03251899280210191}, RC::Data1D{0.7207987453423502}, RC::Data1D{0.1255453971585061}, RC::Data1D{0.8299270003285211}, RC::Data1D{0.5418199260837379}, RC::Data1D{0.25141607765365226}, RC::Data1D{0.2366024077693667}, RC::Data1D{0.9713879695276674}, RC::Data1D{0.5029279373378421}, RC::Data1D{0.036315807935264566}, RC::Data1D{0.43321862910334685}, RC::Data1D{0.05741213303405901}, RC::Data1D{0.41816793224794346}, RC::Data1D{0.4555469680729004}, RC::Data1D{0.4292217025792945}, RC::Data1D{0.2670162954936247}, RC::Data1D{0.706243112627657}, RC::Data1D{0.7659522443056428}, RC::Data1D{0.560457267819172}, RC::Data1D{0.18778073307061283}, RC::Data1D{0.3074956278648182}, RC::Data1D{0.17425307963747105}, RC::Data1D{0.08359423090788787}, RC::Data1D{0.12101594667521898}, RC::Data1D{0.36467647337866205}, RC::Data1D{0.8549753354255015}, RC::Data1D{0.09943896454111223}, RC::Data1D{0.7496606611706578}, RC::Data1D{0.09733903827215351}, RC::Data1D{0.852775517152658}, RC::Data1D{0.30127655235583406}, RC::Data1D{0.2411523930819146}, RC::Data1D{0.7755349278175157}, RC::Data1D{0.24156811604982642}, RC::Data1D{0.3447741455855756}, RC::Data1D{0.5658125963607662}, RC::Data1D{0.3511832081160188}, RC::Data1D{0.5012806740368274}, RC::Data1D{0.3809844295768071}, RC::Data1D{0.18454558984364922}, RC::Data1D{0.765842855262369}, RC::Data1D{0.8117368471574271}, RC::Data1D{0.3128845765116872}, RC::Data1D{0.7490822004310921}, RC::Data1D{0.16903408078975524}, RC::Data1D{0.35527830798242566}, RC::Data1D{0.6116469353168391}, RC::Data1D{0.01693432325413302}, RC::Data1D{0.786887989731953}, RC::Data1D{0.5777156754362269}, RC::Data1D{0.43642904170120334}, RC::Data1D{0.18745302838348976}, RC::Data1D{0.030354568646237534}, RC::Data1D{0.7853885379617362}, RC::Data1D{0.5988681650423449}, RC::Data1D{0.3432143309850476}, RC::Data1D{0.03653631772714194}, RC::Data1D{0.788468106071284}, RC::Data1D{0.3496626181560851}, RC::Data1D{0.8185175535759572}, RC::Data1D{0.8624387521297345}, RC::Data1D{0.056240221497662835}, RC::Data1D{0.34688187869882214}, RC::Data1D{0.5796440965944776}, RC::Data1D{0.0817603367100721}, RC::Data1D{0.47983950051146085}, RC::Data1D{0.6732238794497462}, RC::Data1D{0.9561824656529264}, RC::Data1D{0.3220352849539976}, RC::Data1D{0.5285624911987544}, RC::Data1D{0.23912284936504613}, RC::Data1D{0.5610001323328563}, RC::Data1D{0.7984156000383927}, RC::Data1D{0.3035499090027589}, RC::Data1D{0.0469817983523122}, RC::Data1D{0.9840318154103038}, RC::Data1D{0.12960085665670962}, RC::Data1D{0.8444768611204216}, RC::Data1D{0.41188455183119344}, RC::Data1D{0.00782817339116959}, RC::Data1D{0.07049926706827347}, RC::Data1D{0.1609953247685345}, RC::Data1D{0.9204591629810882}, RC::Data1D{0.22363713353254855}, RC::Data1D{0.4838511103557428}, RC::Data1D{0.027448970921733262}, RC::Data1D{0.8451204061597358}, RC::Data1D{0.0682965608126419}, RC::Data1D{0.8876874964073193}, RC::Data1D{0.8458498565960579}, RC::Data1D{0.5707069452283796}, RC::Data1D{0.6534597891125896}, RC::Data1D{0.06588457040629903}, RC::Data1D{0.282693333583648}, RC::Data1D{0.16785174692922344}, RC::Data1D{0.3278586157070237}, RC::Data1D{0.22689602009512722}, RC::Data1D{0.8943297856424556}, RC::Data1D{0.5530926611305833}, RC::Data1D{0.23482403977256427}, RC::Data1D{0.9820713561348747}, RC::Data1D{0.0024618875635942228}, RC::Data1D{0.16925395354944606}, RC::Data1D{0.5034425132106467}, RC::Data1D{0.8807962976905833}, RC::Data1D{0.8500495095806259}, RC::Data1D{0.46954933077987293}, RC::Data1D{0.0009430884031759179}, RC::Data1D{0.8533103341901742}, RC::Data1D{0.20726596673120845}, RC::Data1D{0.14810002360142505}, RC::Data1D{0.3868640163978496}, RC::Data1D{0.987439140337502}, RC::Data1D{0.3569925195622048}, RC::Data1D{0.1581792739007436}, RC::Data1D{0.7059313533029075}, RC::Data1D{0.37255610911097214}, RC::Data1D{0.2393090327796038}, RC::Data1D{0.24970653112343322}, RC::Data1D{0.6504174739353645}, RC::Data1D{0.43145475836131986}, RC::Data1D{0.9997722469872765}, RC::Data1D{0.4678438186418513}, RC::Data1D{0.07087338862255121}, RC::Data1D{0.21192609579616006}, RC::Data1D{0.921739162047001}, RC::Data1D{0.570962735887126}, RC::Data1D{0.4237000375908022}, RC::Data1D{0.40326912456668007}, RC::Data1D{0.4204107540993276}, RC::Data1D{0.9888855439315393}}, RC::Data2D{RC::Data1D{0.5852141735439127}, RC::Data1D{0.13794466079944878}, RC::Data1D{0.054472896288987216}, RC::Data1D{0.6861965111302268}, RC::Data1D{0.9886762176874062}, RC::Data1D{0.5912315166916342}, RC::Data1D{0.4472262116621717}, RC::Data1D{0.4417315482664037}, RC::Data1D{0.36968833476878393}, RC::Data1D{0.9190502712823737}, RC::Data1D{0.04569462032290417}, RC::Data1D{0.7008683214802428}, RC::Data1D{0.9274073045443698}, RC::Data1D{0.9514530234099571}, RC::Data1D{0.936307658337132}, RC::Data1D{0.8696949315345606}, RC::Data1D{0.09715789791387452}, RC::Data1D{0.7490569391631847}, RC::Data1D{0.489477155268789}, RC::Data1D{0.7275593393585834}, RC::Data1D{0.4670380716367074}, RC::Data1D{0.3729526966462283}, RC::Data1D{0.9114981881372349}, RC::Data1D{0.5447359854090218}, RC::Data1D{0.32565711536092357}, RC::Data1D{0.45901585521646404}, RC::Data1D{0.5251436616829338}, RC::Data1D{0.6375100324580115}, RC::Data1D{0.4422956008956871}, RC::Data1D{0.20619385387223255}, RC::Data1D{0.16665082252269203}, RC::Data1D{0.784804392447786}, RC::Data1D{0.235569733278013}, RC::Data1D{0.7045196606156521}, RC::Data1D{0.014186923438467391}, RC::Data1D{0.03595064060330955}, RC::Data1D{0.9313998131142387}, RC::Data1D{0.16912410446659887}, RC::Data1D{0.7780178840641093}, RC::Data1D{0.062000356160826}, RC::Data1D{0.7605909523393087}, RC::Data1D{0.7463728929283401}, RC::Data1D{0.8797930357033052}, RC::Data1D{0.5034090376166557}, RC::Data1D{0.570838060281577}, RC::Data1D{0.16979504080813357}, RC::Data1D{0.7243812736570782}, RC::Data1D{0.7264859895256693}, RC::Data1D{0.6114874151321923}, RC::Data1D{0.35366831561479184}, RC::Data1D{0.6683067874544377}, RC::Data1D{0.7778295184750744}, RC::Data1D{0.4853533165061419}, RC::Data1D{0.33900799384649727}, RC::Data1D{0.8492947642031248}, RC::Data1D{0.0845309528050251}, RC::Data1D{0.12258972387082367}, RC::Data1D{0.8750422539709193}, RC::Data1D{0.8320462373368815}, RC::Data1D{0.8480844330737548}, RC::Data1D{0.007318932083238083}, RC::Data1D{0.3520565096539803}, RC::Data1D{0.6812877602457369}, RC::Data1D{0.7251990261302483}, RC::Data1D{0.9306379674116543}, RC::Data1D{0.4553038507915992}, RC::Data1D{0.5468107045592139}, RC::Data1D{0.7198076197636547}, RC::Data1D{0.25188040645092624}, RC::Data1D{0.18231887179179862}, RC::Data1D{0.6407288109995175}, RC::Data1D{0.19464985077331187}, RC::Data1D{0.34062098797512796}, RC::Data1D{0.08363790323111109}, RC::Data1D{0.24008595183749148}, RC::Data1D{0.49030912954120176}, RC::Data1D{0.1730741049458201}, RC::Data1D{0.9467484132498546}, RC::Data1D{0.45827260232178557}, RC::Data1D{0.209428743416754}, RC::Data1D{0.9451827349577699}, RC::Data1D{0.8066664344595463}, RC::Data1D{0.12522715311098842}, RC::Data1D{0.36087799938487286}, RC::Data1D{0.9905424263974297}, RC::Data1D{0.383046797121681}, RC::Data1D{0.10920022965426723}, RC::Data1D{0.9926044631773133}, RC::Data1D{0.026249844200898043}, RC::Data1D{0.6531414536012793}, RC::Data1D{0.6172699303353288}, RC::Data1D{0.7482701982643942}, RC::Data1D{0.9806764994966521}, RC::Data1D{0.9359355454444065}, RC::Data1D{0.30207465481129026}, RC::Data1D{0.8880020048707715}, RC::Data1D{0.832060670497729}, RC::Data1D{0.3768748197424747}, RC::Data1D{0.22220576628134536}, RC::Data1D{0.5417860435496963}, RC::Data1D{0.18279445371232395}, RC::Data1D{0.5873843523054472}, RC::Data1D{0.13104963737973374}, RC::Data1D{0.2268426885510375}, RC::Data1D{0.8309586776504223}, RC::Data1D{0.7758699132471535}, RC::Data1D{0.7375077546730399}, RC::Data1D{0.5066144624441862}, RC::Data1D{0.4558948676085205}, RC::Data1D{0.2521097132793164}, RC::Data1D{0.6433667787806674}, RC::Data1D{0.40192423615589434}, RC::Data1D{0.26454866848283376}, RC::Data1D{0.2314569652185583}, RC::Data1D{0.6455354043684923}, RC::Data1D{0.9178535924123985}, RC::Data1D{0.839238390120029}, RC::Data1D{0.0783457799594035}, RC::Data1D{0.9527567612767344}, RC::Data1D{0.5354283709259362}, RC::Data1D{0.20609368766482639}, RC::Data1D{0.9197206045942913}, RC::Data1D{0.12087631418790556}, RC::Data1D{0.5320068107975255}, RC::Data1D{0.640326868267971}, RC::Data1D{0.6657119128158571}, RC::Data1D{0.40389715186179165}, RC::Data1D{0.7118073816354586}, RC::Data1D{0.8247516552686868}, RC::Data1D{0.7399635035035288}, RC::Data1D{0.5912595901313163}, RC::Data1D{0.06452056731510292}, RC::Data1D{0.18364004662411326}, RC::Data1D{0.629366169835392}, RC::Data1D{0.8318359013575894}, RC::Data1D{0.17546750562065028}, RC::Data1D{0.49122757274532347}, RC::Data1D{0.5040431153875685}, RC::Data1D{0.4491859377298505}, RC::Data1D{0.2930411294055649}, RC::Data1D{0.4911111384023472}, RC::Data1D{0.4627397071761531}, RC::Data1D{0.6083096099523884}, RC::Data1D{0.1936492732285755}, RC::Data1D{0.09915974317649745}, RC::Data1D{0.7799039209383151}, RC::Data1D{0.9267827322694634}, RC::Data1D{0.24105847763555277}, RC::Data1D{0.40765295889677733}, RC::Data1D{0.8307569406795184}, RC::Data1D{0.026166176812191688}, RC::Data1D{0.40176759105708215}, RC::Data1D{0.9463800801177934}, RC::Data1D{0.6801644368293047}, RC::Data1D{0.1752620408525617}, RC::Data1D{0.9652016669824623}, RC::Data1D{0.1541016857605252}, RC::Data1D{0.4990497936138957}, RC::Data1D{0.7713732693935793}, RC::Data1D{0.025133160569791868}, RC::Data1D{0.3779004928294555}, RC::Data1D{0.10908859720977004}, RC::Data1D{0.6157003752456883}, RC::Data1D{0.989364366766163}, RC::Data1D{0.9286406266251279}, RC::Data1D{0.8284608865449662}, RC::Data1D{0.8112697016519912}, RC::Data1D{0.24953254244451528}, RC::Data1D{0.01360024463571674}, RC::Data1D{0.0006034647756010258}, RC::Data1D{0.6355219018156415}, RC::Data1D{0.4609291985103069}, RC::Data1D{0.027688461238170148}, RC::Data1D{0.05168313585315598}, RC::Data1D{0.9900049186847792}, RC::Data1D{0.18181988947222516}, RC::Data1D{0.8811906455125994}}, RC::Data2D{RC::Data1D{0.28213898031551476}, RC::Data1D{0.7244365158356829}, RC::Data1D{0.738897310184914}, RC::Data1D{0.3440218053629063}, RC::Data1D{0.5758399406315814}, RC::Data1D{0.09765345557577465}, RC::Data1D{0.7931370948528459}, RC::Data1D{0.1107100516641879}, RC::Data1D{0.7337045243843733}, RC::Data1D{0.7165752688658318}, RC::Data1D{0.41905772559910137}, RC::Data1D{0.6117531599668488}, RC::Data1D{0.9462700994575628}, RC::Data1D{0.2476272495454408}, RC::Data1D{0.5684357060806646}, RC::Data1D{0.9553255184673712}, RC::Data1D{0.1759225810451398}, RC::Data1D{0.6516281960204243}, RC::Data1D{0.5760032461311938}, RC::Data1D{0.8628834966088323}, RC::Data1D{0.6703958647645024}, RC::Data1D{0.7690654124710563}, RC::Data1D{0.5138529230554254}, RC::Data1D{0.5072354638426606}, RC::Data1D{0.6928570805369323}, RC::Data1D{0.7728890644966586}, RC::Data1D{0.1367152484039742}, RC::Data1D{0.6584863201378589}, RC::Data1D{0.553886124354928}, RC::Data1D{0.9630632993688834}, RC::Data1D{0.2786807643152115}, RC::Data1D{0.5948942995223038}, RC::Data1D{0.3222159422033305}, RC::Data1D{0.5677829660740116}, RC::Data1D{0.7014943492793787}, RC::Data1D{0.959591913310903}, RC::Data1D{0.843421534705013}, RC::Data1D{0.030105722223685127}, RC::Data1D{0.12122365429649418}, RC::Data1D{0.5214890534079171}, RC::Data1D{0.8296661755429608}, RC::Data1D{0.44596668192431377}, RC::Data1D{0.3470535750156555}, RC::Data1D{0.8735903946265311}, RC::Data1D{0.08907143867704759}, RC::Data1D{0.6630601446816861}, RC::Data1D{0.9143614194236703}, RC::Data1D{0.11913466630628533}, RC::Data1D{0.4257282246469922}, RC::Data1D{0.35746761475808475}, RC::Data1D{0.9576710078750227}, RC::Data1D{0.3552281485081459}, RC::Data1D{0.4007652550612004}, RC::Data1D{0.8737369998159729}, RC::Data1D{0.0713119445487953}, RC::Data1D{0.595528702697891}, RC::Data1D{0.04708299956410644}, RC::Data1D{0.934178847845552}, RC::Data1D{0.20858046709284017}, RC::Data1D{0.505027338949508}, RC::Data1D{0.8814602393880173}, RC::Data1D{0.6631820722303495}, RC::Data1D{0.7841465387828416}, RC::Data1D{0.14343509392094156}, RC::Data1D{0.5737329957296737}, RC::Data1D{0.6239171126813453}, RC::Data1D{0.8166361032862608}, RC::Data1D{0.6444867829951615}, RC::Data1D{0.35333956498841956}, RC::Data1D{0.854783822925092}, RC::Data1D{0.5282732640395049}, RC::Data1D{0.8277187620900899}, RC::Data1D{0.7836300371193275}, RC::Data1D{0.7074018415563297}, RC::Data1D{0.2556318845009359}, RC::Data1D{0.14681180447934294}, RC::Data1D{0.46568727754373307}, RC::Data1D{0.44597866341728853}, RC::Data1D{0.9053110523607338}, RC::Data1D{0.09796037637235488}, RC::Data1D{0.06781610398244509}, RC::Data1D{0.1490726808418199}, RC::Data1D{0.8149413120853385}, RC::Data1D{0.6429174338349493}, RC::Data1D{0.18133519224488093}, RC::Data1D{0.7101825831350917}, RC::Data1D{0.5568208125500642}, RC::Data1D{0.3602645972135996}, RC::Data1D{0.5419825335807612}, RC::Data1D{0.3288215585242835}, RC::Data1D{0.11116489661162388}, RC::Data1D{0.4523527243249109}, RC::Data1D{0.47971366125837533}, RC::Data1D{0.20051356637613293}, RC::Data1D{0.41839545961493263}, RC::Data1D{0.03489828301296627}, RC::Data1D{0.2958190910776233}, RC::Data1D{0.6789446750665838}, RC::Data1D{0.322507890946775}, RC::Data1D{0.15819261720647615}, RC::Data1D{0.6027820869795637}, RC::Data1D{0.6000885327763752}, RC::Data1D{0.5581116629794266}, RC::Data1D{0.8945671240644185}, RC::Data1D{0.343671708581903}, RC::Data1D{0.5350095019955001}, RC::Data1D{0.7383946010970575}, RC::Data1D{0.1352283827441193}, RC::Data1D{0.11865717188460745}, RC::Data1D{0.5461576914328948}, RC::Data1D{0.013322082188525486}, RC::Data1D{0.358962666646495}, RC::Data1D{0.007512177863309155}, RC::Data1D{0.9587415271443175}, RC::Data1D{0.6185353115914987}, RC::Data1D{0.8397667195463134}, RC::Data1D{0.21874175395558204}, RC::Data1D{0.9095001149174413}, RC::Data1D{0.047375251952105124}, RC::Data1D{0.6136003347970201}, RC::Data1D{0.509158173487327}, RC::Data1D{0.9086204401228319}, RC::Data1D{0.29173278075857345}, RC::Data1D{0.21133823208318991}, RC::Data1D{0.3486697107703963}, RC::Data1D{0.6133070180193294}, RC::Data1D{0.2543218606043621}, RC::Data1D{0.8415550255964913}, RC::Data1D{0.451266191131239}, RC::Data1D{0.630460361915359}, RC::Data1D{0.5839816070688606}, RC::Data1D{0.16380582158191392}, RC::Data1D{0.04316072879248156}, RC::Data1D{0.34306701281859264}, RC::Data1D{0.9622724508096492}, RC::Data1D{0.12807137044210948}, RC::Data1D{0.3820232576325955}, RC::Data1D{0.6478251929468746}, RC::Data1D{0.401183331740261}, RC::Data1D{0.5469825842137773}, RC::Data1D{0.1669386247634611}, RC::Data1D{0.7167247895611969}, RC::Data1D{0.3879295432639449}, RC::Data1D{0.1047137506834318}, RC::Data1D{0.6926596675828738}, RC::Data1D{0.19337301003363794}, RC::Data1D{0.12804511192747203}, RC::Data1D{0.26800038743489785}, RC::Data1D{0.7223566167168483}, RC::Data1D{0.8124347430752547}, RC::Data1D{0.7440804107572834}, RC::Data1D{0.6172737230085625}, RC::Data1D{0.41250889434112603}, RC::Data1D{0.9418810965201707}, RC::Data1D{0.8847763820848163}, RC::Data1D{0.2202471994766526}, RC::Data1D{0.9796014250199659}, RC::Data1D{0.11170585882646089}, RC::Data1D{0.2565055556433533}, RC::Data1D{0.5527926517390942}, RC::Data1D{0.26881527862510823}, RC::Data1D{0.6593139119678582}, RC::Data1D{0.08229989449000841}, RC::Data1D{0.0006511362280600119}, RC::Data1D{0.7685472925474158}, RC::Data1D{0.61882030499443}, RC::Data1D{0.48975982387510153}, RC::Data1D{0.5756774060810461}, RC::Data1D{0.7784053623904598}, RC::Data1D{0.5547502708397922}, RC::Data1D{0.830934295528252}, RC::Data1D{0.5380957354908619}, RC::Data1D{0.653227486776229}, RC::Data1D{0.720760962238966}, RC::Data1D{0.9197486416703049}, RC::Data1D{0.09014444187166692}, RC::Data1D{0.9337529998958287}}, RC::Data2D{RC::Data1D{0.739898466245184}, RC::Data1D{0.19285745557357892}, RC::Data1D{0.2937290185721847}, RC::Data1D{0.6977924597251762}, RC::Data1D{0.9914546514014542}, RC::Data1D{0.0008157169871191305}, RC::Data1D{0.2553095057001772}, RC::Data1D{0.28228880624695296}, RC::Data1D{0.4504161511613506}, RC::Data1D{0.45847304232817865}, RC::Data1D{0.34658244290719853}, RC::Data1D{0.3206795282468998}, RC::Data1D{0.6290295180610687}, RC::Data1D{0.5648724669078934}, RC::Data1D{0.9167438526651535}, RC::Data1D{0.6144219476686181}, RC::Data1D{0.33916264800564155}, RC::Data1D{0.4099737490747559}, RC::Data1D{0.485452039581696}, RC::Data1D{0.34450895855871777}, RC::Data1D{0.07631005788156409}, RC::Data1D{0.6902482701606281}, RC::Data1D{0.884489658252211}, RC::Data1D{0.072235124989108}, RC::Data1D{0.6201671352816388}, RC::Data1D{0.0877777641211972}, RC::Data1D{0.604539156018786}, RC::Data1D{0.2395629244663403}, RC::Data1D{0.9482624648926626}, RC::Data1D{0.7604755032418183}, RC::Data1D{0.784949579733662}, RC::Data1D{0.6015912236218546}, RC::Data1D{0.5371317250924088}, RC::Data1D{0.5258814098385511}, RC::Data1D{0.49880412820380493}, RC::Data1D{0.8474389131680875}, RC::Data1D{0.3762536907890328}, RC::Data1D{0.38799545382222944}, RC::Data1D{0.9996975883734309}, RC::Data1D{0.2867347173248397}, RC::Data1D{0.2758692376737596}, RC::Data1D{0.5412456307498167}, RC::Data1D{0.7790362244011189}, RC::Data1D{0.1567015340785488}, RC::Data1D{0.4270013749054159}, RC::Data1D{0.9679511992411514}, RC::Data1D{0.11319724031999923}, RC::Data1D{0.8059259758068772}, RC::Data1D{0.4143239133725847}, RC::Data1D{0.6974945531395317}, RC::Data1D{0.6317295179985348}, RC::Data1D{0.0014934880553342245}, RC::Data1D{0.9728401308788013}, RC::Data1D{0.39445254721394707}, RC::Data1D{0.6718412771368044}, RC::Data1D{0.3422664909423797}, RC::Data1D{0.8980575340511385}, RC::Data1D{0.15363699085537108}, RC::Data1D{0.24238003804271546}, RC::Data1D{0.21695156725814357}, RC::Data1D{0.891847469970085}, RC::Data1D{0.08068185147517026}, RC::Data1D{0.5022719940965982}, RC::Data1D{0.6504731980371701}, RC::Data1D{0.6885054828187992}, RC::Data1D{0.35409270734469933}, RC::Data1D{0.19683370193962457}, RC::Data1D{0.6929159534374527}, RC::Data1D{0.8007149889269909}, RC::Data1D{0.4514937229749483}, RC::Data1D{0.6776618881062564}, RC::Data1D{0.04606123055953193}, RC::Data1D{0.015066949829920406}, RC::Data1D{0.9600522461542037}, RC::Data1D{0.8873949605022468}, RC::Data1D{0.49090533446933315}, RC::Data1D{0.6458520393141285}, RC::Data1D{0.9320120190788587}, RC::Data1D{0.16369850079951465}, RC::Data1D{0.8712775603764674}, RC::Data1D{0.15555844902526195}, RC::Data1D{0.556029546860464}, RC::Data1D{0.618054090719781}, RC::Data1D{0.6435936078140331}, RC::Data1D{0.28586784291217016}, RC::Data1D{0.12132244553514271}, RC::Data1D{0.6312966246762461}, RC::Data1D{0.0671964185411491}, RC::Data1D{0.3956573115448919}, RC::Data1D{0.1805607076912792}, RC::Data1D{0.7580306140979988}, RC::Data1D{0.5004759967241568}, RC::Data1D{0.756619572738252}, RC::Data1D{0.6618433426260089}, RC::Data1D{0.1328560050613865}, RC::Data1D{0.4309487981333132}, RC::Data1D{0.749608168146719}, RC::Data1D{0.5996006472990156}, RC::Data1D{0.9267840593406391}, RC::Data1D{0.7453260821324907}, RC::Data1D{0.36412508238942054}, RC::Data1D{0.5039029864927486}, RC::Data1D{0.6565967449159434}, RC::Data1D{0.36132044230374427}, RC::Data1D{0.5989221226047916}, RC::Data1D{0.10040030081240447}, RC::Data1D{0.41285972576764796}, RC::Data1D{0.8229617058216325}, RC::Data1D{0.17804512349576107}, RC::Data1D{0.20610601596710054}, RC::Data1D{0.04511095857638736}, RC::Data1D{0.8695156298951418}, RC::Data1D{0.583954115573731}, RC::Data1D{0.3317671687897952}, RC::Data1D{0.662524642899522}, RC::Data1D{0.3391209306454852}, RC::Data1D{0.4258619802393079}, RC::Data1D{0.44338052572207287}, RC::Data1D{0.20403892239992205}, RC::Data1D{0.06797061917770753}, RC::Data1D{0.41839055837491934}, RC::Data1D{0.6311327706083004}, RC::Data1D{0.1043004984711301}, RC::Data1D{0.27456730963662657}, RC::Data1D{0.8157908983501255}, RC::Data1D{0.7965702897644872}, RC::Data1D{0.21219064204301796}, RC::Data1D{0.3907925460626219}, RC::Data1D{0.9097348066586859}, RC::Data1D{0.317782433597128}, RC::Data1D{0.02239475481386799}, RC::Data1D{0.8610220041160406}, RC::Data1D{0.5724442507883664}, RC::Data1D{0.31813548366626077}, RC::Data1D{0.07846946703923963}, RC::Data1D{0.744279592226895}, RC::Data1D{0.056597082143415745}, RC::Data1D{0.34664643651052607}, RC::Data1D{0.2264437231548515}, RC::Data1D{0.2919994774381024}, RC::Data1D{0.8741573841371384}, RC::Data1D{0.334163324120079}, RC::Data1D{0.4338246003449914}, RC::Data1D{0.47915512939338933}, RC::Data1D{0.05871298689837823}, RC::Data1D{0.7081421622070999}, RC::Data1D{0.8564193989903724}, RC::Data1D{0.5352160807169938}, RC::Data1D{0.9796080832638163}, RC::Data1D{0.8205111813926507}, RC::Data1D{0.4158263256790655}, RC::Data1D{0.04114454711442839}, RC::Data1D{0.5747631470866268}, RC::Data1D{0.9605462955788756}, RC::Data1D{0.11602200602506174}, RC::Data1D{0.9350004258562891}, RC::Data1D{0.494648570124126}, RC::Data1D{0.18500163345854814}, RC::Data1D{0.6641330837744412}, RC::Data1D{0.19328553921478664}, RC::Data1D{0.5171111755910636}, RC::Data1D{0.3842595766382416}, RC::Data1D{0.9301243888899766}, RC::Data1D{0.22467811465914056}, RC::Data1D{0.4300706524166926}, RC::Data1D{0.1296375297127359}, RC::Data1D{0.9277624372640996}, RC::Data1D{0.061299248265933914}, RC::Data1D{0.3391702869315003}, RC::Data1D{0.2842529662154175}, RC::Data1D{0.6739829658135997}, RC::Data1D{0.5333940553316262}, RC::Data1D{0.6878472757977868}, RC::Data1D{0.0716715686901851}, RC::Data1D{0.8686770002428206}, RC::Data1D{0.6512116291656749}, RC::Data1D{0.07539268791846909}}, RC::Data2D{RC::Data1D{0.7874253356318061}, RC::Data1D{0.8489693500748582}, RC::Data1D{0.8596210748480505}, RC::Data1D{0.8860249185302937}, RC::Data1D{0.6829362576396328}, RC::Data1D{0.8562294578737302}, RC::Data1D{0.3071711877778681}, RC::Data1D{0.5557870873791101}, RC::Data1D{0.25994302930622715}, RC::Data1D{0.12389799522570488}, RC::Data1D{0.8904533497850038}, RC::Data1D{0.8611202674980178}, RC::Data1D{0.8375542817072581}, RC::Data1D{0.6260302236866372}, RC::Data1D{0.5572336517373573}, RC::Data1D{0.7328291893351041}, RC::Data1D{0.9214597664771343}, RC::Data1D{0.4972881154004247}, RC::Data1D{0.3592436488023508}, RC::Data1D{0.6591527563955349}, RC::Data1D{0.3856307307464173}, RC::Data1D{0.362939235530781}, RC::Data1D{0.1640312588052243}, RC::Data1D{0.9605413449304917}, RC::Data1D{0.8412798646844156}, RC::Data1D{0.29824688924135156}, RC::Data1D{0.14570261723940625}, RC::Data1D{0.5715657325683868}, RC::Data1D{0.2957929410237894}, RC::Data1D{0.7458276790799014}, RC::Data1D{0.4236851268248204}, RC::Data1D{0.24370958474134818}, RC::Data1D{0.30684710322885833}, RC::Data1D{0.34172293209457805}, RC::Data1D{0.9709451495177187}, RC::Data1D{0.9338050330731468}, RC::Data1D{0.6045186628725121}, RC::Data1D{0.6426602221355929}, RC::Data1D{0.2094085838021117}, RC::Data1D{0.7830395275761184}, RC::Data1D{0.1740413808940633}, RC::Data1D{0.6238905601907334}, RC::Data1D{0.4018876521099769}, RC::Data1D{0.8022093110606023}, RC::Data1D{0.3350957942427669}, RC::Data1D{0.6972671227628341}, RC::Data1D{0.10793823585843898}, RC::Data1D{0.2531203972237983}, RC::Data1D{0.4072500101083095}, RC::Data1D{0.3330693572616221}, RC::Data1D{0.8028240183470815}, RC::Data1D{0.29799915860219306}, RC::Data1D{0.8259538057234146}, RC::Data1D{0.8776393754948191}, RC::Data1D{0.14110118763489554}, RC::Data1D{0.5506402003394846}, RC::Data1D{0.11677587749087726}, RC::Data1D{0.8690304806708192}, RC::Data1D{0.6739184012988161}, RC::Data1D{0.1425003393814278}, RC::Data1D{0.8648224887767214}, RC::Data1D{0.23769141859621878}, RC::Data1D{0.0768427956289297}, RC::Data1D{0.7608903986101215}, RC::Data1D{0.869732124859778}, RC::Data1D{0.2612508501123365}, RC::Data1D{0.5246664484839935}, RC::Data1D{0.49079622680201707}, RC::Data1D{0.44384646366640945}, RC::Data1D{0.5035780440528238}, RC::Data1D{0.2031116181389785}, RC::Data1D{0.49883036952546345}, RC::Data1D{0.3435117073495759}, RC::Data1D{0.5866443441651004}, RC::Data1D{0.8143752091088804}, RC::Data1D{0.3336084290238408}, RC::Data1D{0.21442559927226157}, RC::Data1D{0.8234082011692018}, RC::Data1D{0.4427943925290073}, RC::Data1D{0.7811757964616555}, RC::Data1D{0.9913064303365117}, RC::Data1D{0.980351047510172}, RC::Data1D{0.38290850980035773}, RC::Data1D{0.7639543950855873}, RC::Data1D{0.34402986643695044}, RC::Data1D{0.03879963011231391}, RC::Data1D{0.6152132642319117}, RC::Data1D{0.043657242595650625}, RC::Data1D{0.8583314819595047}, RC::Data1D{0.08356268491616148}, RC::Data1D{0.8543154952189752}, RC::Data1D{0.690924577326756}, RC::Data1D{0.5453239664430678}, RC::Data1D{0.8226685392446076}, RC::Data1D{0.4666254704718942}, RC::Data1D{0.5402008667128007}, RC::Data1D{0.2722339440321887}, RC::Data1D{0.37432039780705384}, RC::Data1D{0.7639862013739409}, RC::Data1D{0.45943644355890045}, RC::Data1D{0.6241902894736088}, RC::Data1D{0.865741207974323}, RC::Data1D{0.8609189298805907}, RC::Data1D{0.2741565284988634}, RC::Data1D{0.1709112363284161}, RC::Data1D{0.9706227278692}, RC::Data1D{0.9037045313015237}, RC::Data1D{0.765998478953909}, RC::Data1D{0.3406611911550982}, RC::Data1D{0.7581375388682815}, RC::Data1D{0.3542819597718123}, RC::Data1D{0.6321976387168091}, RC::Data1D{0.7884957752833344}, RC::Data1D{0.0827503697925045}, RC::Data1D{0.03324465870939308}, RC::Data1D{0.6303575205249324}, RC::Data1D{0.14864278843898604}, RC::Data1D{0.9248387982008127}, RC::Data1D{0.9843643121231437}, RC::Data1D{0.3849253938051451}, RC::Data1D{0.39680221072611166}, RC::Data1D{0.6372570379342882}, RC::Data1D{0.3015564474005219}, RC::Data1D{0.9876255109324805}, RC::Data1D{0.18828088289905054}, RC::Data1D{0.7783679016787817}, RC::Data1D{0.47517062740921434}, RC::Data1D{0.34133105764767036}, RC::Data1D{0.15478818186966647}, RC::Data1D{0.26086173777838206}, RC::Data1D{0.0678506191973769}, RC::Data1D{0.0015839586598594968}, RC::Data1D{0.02642733262787922}, RC::Data1D{0.7674306670571293}, RC::Data1D{0.5549635895290458}, RC::Data1D{0.36817409515824195}, RC::Data1D{0.38896647304914356}, RC::Data1D{0.46297704766660286}, RC::Data1D{0.16407784065651487}, RC::Data1D{0.08081528227308599}, RC::Data1D{0.2510488044707443}, RC::Data1D{0.713367081728286}, RC::Data1D{0.8108296222245389}, RC::Data1D{0.4366966496334429}, RC::Data1D{0.8077653177531098}, RC::Data1D{0.267311038422579}, RC::Data1D{0.28019604467922765}, RC::Data1D{0.7874765058158837}, RC::Data1D{0.5204652753146359}, RC::Data1D{0.0049013571594760386}, RC::Data1D{0.8859804820995064}, RC::Data1D{0.5372890843849606}, RC::Data1D{0.5724834728274354}, RC::Data1D{0.6359446632378151}, RC::Data1D{0.5578125137475636}, RC::Data1D{0.7329601345324163}, RC::Data1D{0.59158328983558}, RC::Data1D{0.5673403529444909}, RC::Data1D{0.8848320802904578}, RC::Data1D{0.790179967086135}, RC::Data1D{0.17775196041768548}, RC::Data1D{0.07218413643206456}, RC::Data1D{0.2430918127234255}, RC::Data1D{0.14549660825151545}, RC::Data1D{0.2937113566096643}, RC::Data1D{0.5036860302734464}, RC::Data1D{0.8824787744618554}, RC::Data1D{0.12537178716578756}, RC::Data1D{0.9143628334087757}, RC::Data1D{0.34391502697937104}, RC::Data1D{0.9685685960657168}, RC::Data1D{0.8760554431302581}, RC::Data1D{0.5647356567845112}, RC::Data1D{0.4749281646021114}, RC::Data1D{0.618054004505423}, RC::Data1D{0.9418325169660131}, RC::Data1D{0.2042000909026589}}, RC::Data2D{RC::Data1D{0.22521585866542393}, RC::Data1D{0.9387644501834054}, RC::Data1D{0.7536477957977596}, RC::Data1D{0.6449892766871322}, RC::Data1D{0.11962514524478052}, RC::Data1D{0.10291772639156649}, RC::Data1D{0.4571947667255325}, RC::Data1D{0.2628887444550221}, RC::Data1D{0.1372364066496038}, RC::Data1D{0.8146004903429956}, RC::Data1D{0.7877760264337496}, RC::Data1D{0.3150483896266145}, RC::Data1D{0.0875520916111393}, RC::Data1D{0.9071858280490649}, RC::Data1D{0.01427011183205007}, RC::Data1D{0.33070436408082615}, RC::Data1D{0.6493899990009198}, RC::Data1D{0.46650424046360406}, RC::Data1D{0.7348824745811011}, RC::Data1D{0.5742903154248838}, RC::Data1D{0.40478160228465143}, RC::Data1D{0.21015417794825864}, RC::Data1D{0.6190002526073107}, RC::Data1D{0.29996802235059583}, RC::Data1D{0.35659320919123494}, RC::Data1D{0.7763069163402087}, RC::Data1D{0.36869608394538744}, RC::Data1D{0.0536893967781884}, RC::Data1D{0.9083763000881435}, RC::Data1D{0.1751086120793066}, RC::Data1D{0.021374703533852513}, RC::Data1D{0.12215568198193272}, RC::Data1D{0.37979087909428466}, RC::Data1D{0.7321336765021241}, RC::Data1D{0.5268411671373789}, RC::Data1D{0.5203870820155063}, RC::Data1D{0.10587650718980346}, RC::Data1D{0.7666975121895354}, RC::Data1D{0.48126226174324727}, RC::Data1D{0.3064804042177035}, RC::Data1D{0.2988781936139695}, RC::Data1D{0.050139432413070506}, RC::Data1D{0.7702584557917762}, RC::Data1D{0.17399137916205576}, RC::Data1D{0.6166242648368394}, RC::Data1D{0.4333279829186255}, RC::Data1D{0.5867606590293847}, RC::Data1D{0.6435751960366374}, RC::Data1D{0.8361643369411161}, RC::Data1D{0.1295223283758441}, RC::Data1D{0.06224213095948583}, RC::Data1D{0.01901901476694523}, RC::Data1D{0.07186366904249153}, RC::Data1D{0.8709132874506277}, RC::Data1D{0.22702331996611957}, RC::Data1D{0.4840380576443011}, RC::Data1D{0.24654084589869218}, RC::Data1D{0.445303114669391}, RC::Data1D{0.8111868116668185}, RC::Data1D{0.2426396645297204}, RC::Data1D{0.6148175267115555}, RC::Data1D{0.3309471348672952}, RC::Data1D{0.5014331574241855}, RC::Data1D{0.27584567122881987}, RC::Data1D{0.7161460846085933}, RC::Data1D{0.3985023460647902}, RC::Data1D{0.3434925765173883}, RC::Data1D{0.9758377742944321}, RC::Data1D{0.16173826640880506}, RC::Data1D{0.41261582302422006}, RC::Data1D{0.7134019282708998}, RC::Data1D{0.5496082540588834}, RC::Data1D{0.28007292563166675}, RC::Data1D{0.07229663273262665}, RC::Data1D{0.5272729274782426}, RC::Data1D{0.15315551243171854}, RC::Data1D{0.6280019569568711}, RC::Data1D{0.11090318715671954}, RC::Data1D{0.1176971857569713}, RC::Data1D{0.029841452359882115}, RC::Data1D{0.4973745619357314}, RC::Data1D{0.9342686163545714}, RC::Data1D{0.7047725848420363}, RC::Data1D{0.18671408684357427}, RC::Data1D{0.301914745450946}, RC::Data1D{0.4251759322917912}, RC::Data1D{0.006523172867834659}, RC::Data1D{0.6209268931285367}, RC::Data1D{0.44080374863792815}, RC::Data1D{0.7246918911152657}, RC::Data1D{0.5269225009201335}, RC::Data1D{0.5307614565175421}, RC::Data1D{0.6033850786938888}, RC::Data1D{0.37459602340780684}, RC::Data1D{0.4111606415650033}, RC::Data1D{0.08847056190161318}, RC::Data1D{0.6704314398332197}, RC::Data1D{0.011258046057391558}, RC::Data1D{0.6773968842043354}, RC::Data1D{0.6745863242044621}, RC::Data1D{0.8724369657343357}, RC::Data1D{0.945812999252762}, RC::Data1D{0.6857980594179427}, RC::Data1D{0.8446437635039029}, RC::Data1D{0.0157594070498287}, RC::Data1D{0.06786676047161544}, RC::Data1D{0.5164400567623599}, RC::Data1D{0.6603249883089759}, RC::Data1D{0.8641921041648557}, RC::Data1D{0.5278526582949856}, RC::Data1D{0.10067800867210974}, RC::Data1D{0.2686424001937361}, RC::Data1D{0.7635423341423346}, RC::Data1D{0.8276806186602186}, RC::Data1D{0.3473559492494559}, RC::Data1D{0.3125507684002675}, RC::Data1D{0.5471629559892738}, RC::Data1D{0.3139206433410743}, RC::Data1D{0.4560942313776206}, RC::Data1D{0.13374573770031817}, RC::Data1D{0.913711424828483}, RC::Data1D{0.15021968360179128}, RC::Data1D{0.5798745560761662}, RC::Data1D{0.7995636083918316}, RC::Data1D{0.6564004892919832}, RC::Data1D{0.9350121909482205}, RC::Data1D{0.2724536033456939}, RC::Data1D{0.05283165687794289}, RC::Data1D{0.4534141845304278}, RC::Data1D{0.050346958766351646}, RC::Data1D{0.27531171824195566}, RC::Data1D{0.7273865710808962}, RC::Data1D{0.09895403240889611}, RC::Data1D{0.6978099618769695}, RC::Data1D{0.038011888005839656}, RC::Data1D{0.7087945352068238}, RC::Data1D{0.294895126979581}, RC::Data1D{0.8883131523516207}, RC::Data1D{0.06874142939530203}, RC::Data1D{0.43958635912941457}, RC::Data1D{0.730203080180103}, RC::Data1D{0.3591755291001777}, RC::Data1D{0.7394971150672501}, RC::Data1D{0.7787120319477374}, RC::Data1D{0.020132485875723805}, RC::Data1D{0.4548514728205939}, RC::Data1D{0.0085567738526805}, RC::Data1D{0.25453240961566914}, RC::Data1D{0.14982252920613104}, RC::Data1D{0.44684942556769247}, RC::Data1D{0.3601861255904838}, RC::Data1D{0.4859478210680024}, RC::Data1D{0.8469944670623321}, RC::Data1D{0.7649221896375294}, RC::Data1D{0.09394720648894683}, RC::Data1D{0.8186290275269122}, RC::Data1D{0.7993231544289572}, RC::Data1D{0.8037591997954481}, RC::Data1D{0.2236378610368901}, RC::Data1D{0.8455142670033792}, RC::Data1D{0.6863116367408625}, RC::Data1D{0.8000473112323918}, RC::Data1D{0.8126241538074243}, RC::Data1D{0.45621051433024085}, RC::Data1D{0.9904130516855836}, RC::Data1D{0.7929463028789397}, RC::Data1D{0.052538204572370906}, RC::Data1D{0.7880387146163605}, RC::Data1D{0.6355530575626741}, RC::Data1D{0.7402942290917666}, RC::Data1D{0.6983107774823942}, RC::Data1D{0.1620914841896045}, RC::Data1D{0.8021412824856828}, RC::Data1D{0.081380245445146}, RC::Data1D{0.611495496534302}, RC::Data1D{0.7443132438875021}, RC::Data1D{0.9590301630595551}}, RC::Data2D{RC::Data1D{0.9411992150603231}, RC::Data1D{0.783049707850874}, RC::Data1D{0.3085137795802366}, RC::Data1D{0.4137716676872243}, RC::Data1D{0.2898391767440788}, RC::Data1D{0.04044690073981294}, RC::Data1D{0.11455897098246637}, RC::Data1D{0.33869582328713066}, RC::Data1D{0.5569975438398204}, RC::Data1D{0.4349088284764745}, RC::Data1D{0.5354130003350618}, RC::Data1D{0.11730403646360821}, RC::Data1D{0.7617452648114579}, RC::Data1D{0.4319705316811704}, RC::Data1D{0.7900304201543064}, RC::Data1D{0.9238037072559784}, RC::Data1D{0.5327947968608856}, RC::Data1D{0.9403443672831062}, RC::Data1D{0.8129326010676848}, RC::Data1D{0.6439323027029301}, RC::Data1D{0.6727655026791957}, RC::Data1D{0.7411369336034725}, RC::Data1D{0.08043211182203658}, RC::Data1D{0.47883564776863063}, RC::Data1D{0.08427947012076697}, RC::Data1D{0.0990709489741246}, RC::Data1D{0.3287545643733033}, RC::Data1D{0.3624874183910948}, RC::Data1D{0.3796344878128738}, RC::Data1D{0.3344494478512645}, RC::Data1D{0.30570893239379837}, RC::Data1D{0.6641273861047945}, RC::Data1D{0.5143357607781489}, RC::Data1D{0.028310552796609656}, RC::Data1D{0.5115545004094917}, RC::Data1D{0.5569442693287706}, RC::Data1D{0.9852209850819859}, RC::Data1D{0.09278172389401729}, RC::Data1D{0.5143625223513395}, RC::Data1D{0.43953778443930724}, RC::Data1D{0.1537242883243879}, RC::Data1D{0.06809384556313591}, RC::Data1D{0.6321434803046948}, RC::Data1D{0.160521742624687}, RC::Data1D{0.40729838684480224}, RC::Data1D{0.9913511207425286}, RC::Data1D{0.4292219142927002}, RC::Data1D{0.05468052522665723}, RC::Data1D{0.9472006299019479}, RC::Data1D{0.052818259752632546}, RC::Data1D{0.8839539740426055}, RC::Data1D{0.5148291635607275}, RC::Data1D{0.13559394653137902}, RC::Data1D{0.9226207795274458}, RC::Data1D{0.39066449806724524}, RC::Data1D{0.7552249812296149}, RC::Data1D{0.9860574769077705}, RC::Data1D{0.38764843891869183}, RC::Data1D{0.5636211749552791}, RC::Data1D{0.2504250467442172}, RC::Data1D{0.218417683104547}, RC::Data1D{0.8462986298265305}, RC::Data1D{0.45998995340645943}, RC::Data1D{0.3244956086089742}, RC::Data1D{0.32050868982310543}, RC::Data1D{0.43182004548588326}, RC::Data1D{0.10215689650331206}, RC::Data1D{0.9456899252972958}, RC::Data1D{0.061785067536462845}, RC::Data1D{0.22287306251726635}, RC::Data1D{0.5884582774599497}, RC::Data1D{0.16170341249811782}, RC::Data1D{0.7560691740669238}, RC::Data1D{0.2654978268621526}, RC::Data1D{0.5620531077409557}, RC::Data1D{0.37876748807286154}, RC::Data1D{0.6294875391284399}, RC::Data1D{0.9934165990216809}, RC::Data1D{0.8993490601965035}, RC::Data1D{0.09237769363378157}, RC::Data1D{0.44319213794171086}, RC::Data1D{0.20084734467377363}, RC::Data1D{0.2990691461448074}, RC::Data1D{0.47138694605510556}, RC::Data1D{0.787793439833549}, RC::Data1D{0.3044162668925371}, RC::Data1D{0.04419429485989701}, RC::Data1D{0.2959992259172294}, RC::Data1D{0.8132334458530873}, RC::Data1D{0.496247732276052}, RC::Data1D{0.2035739538762975}, RC::Data1D{0.2999364318498007}, RC::Data1D{0.7629970817222863}, RC::Data1D{0.9934198046938317}, RC::Data1D{0.9831993112422093}, RC::Data1D{0.36319348929104944}, RC::Data1D{0.40187365018616117}, RC::Data1D{0.9659282381549378}, RC::Data1D{0.31609372769257327}, RC::Data1D{0.8680284297495001}, RC::Data1D{0.15253132167746275}, RC::Data1D{0.08664561860605735}, RC::Data1D{0.1674625380135275}, RC::Data1D{0.09847259894107285}, RC::Data1D{0.9093149633315349}, RC::Data1D{0.48802035375604635}, RC::Data1D{0.17096551700395046}, RC::Data1D{0.38370316221395717}, RC::Data1D{0.213760927484973}, RC::Data1D{0.752274817147197}, RC::Data1D{0.8262485001607236}, RC::Data1D{0.38025372215488207}, RC::Data1D{0.2587787367069003}, RC::Data1D{0.7652116656269925}, RC::Data1D{0.02256465147513087}, RC::Data1D{0.8562449856015214}, RC::Data1D{0.8413629528879532}, RC::Data1D{0.8227565948805489}, RC::Data1D{0.852052610972107}, RC::Data1D{0.6275328927378246}, RC::Data1D{0.9766933228143192}, RC::Data1D{0.09291299504545847}, RC::Data1D{0.014020643417707257}, RC::Data1D{0.4207406750325954}, RC::Data1D{0.7620936172594452}, RC::Data1D{0.688920536853898}, RC::Data1D{0.3220477924343109}, RC::Data1D{0.9698201567759054}, RC::Data1D{0.8513565221741565}, RC::Data1D{0.4331675522934809}, RC::Data1D{0.9829765962951899}, RC::Data1D{0.5648039990089863}, RC::Data1D{0.21259745177713707}, RC::Data1D{0.6399615330677143}, RC::Data1D{0.7000093142941437}, RC::Data1D{0.7508469179654006}, RC::Data1D{0.5932582449912496}, RC::Data1D{0.323570627035474}, RC::Data1D{0.4210366853550226}, RC::Data1D{0.9756468063841188}, RC::Data1D{0.8117086278869428}, RC::Data1D{0.23125113653164409}, RC::Data1D{0.17455046525579798}, RC::Data1D{0.543584089178809}, RC::Data1D{0.40596560134490844}, RC::Data1D{0.7132537647729575}, RC::Data1D{0.1352842100537971}, RC::Data1D{0.8751922250384607}, RC::Data1D{0.16289972299359656}, RC::Data1D{0.1851803686767306}, RC::Data1D{0.11477760844368745}, RC::Data1D{0.01061062289404402}, RC::Data1D{0.8106489931399528}, RC::Data1D{0.6456208705772322}, RC::Data1D{0.1470409220679515}, RC::Data1D{0.005611036235565359}, RC::Data1D{0.390784855956549}, RC::Data1D{0.791658747755448}, RC::Data1D{0.8599184525721285}, RC::Data1D{0.0018135630943898473}, RC::Data1D{0.20977255464706968}, RC::Data1D{0.49917849186449137}, RC::Data1D{0.667877509923791}, RC::Data1D{0.10237782813865215}, RC::Data1D{0.9780881342907076}, RC::Data1D{0.1763747263852089}, RC::Data1D{0.17148419100552548}, RC::Data1D{0.40858287074466715}, RC::Data1D{0.4733000541093124}, RC::Data1D{0.6261499121309337}, RC::Data1D{0.8113059821454283}, RC::Data1D{0.6655573536504225}, RC::Data1D{0.05445351631668571}, RC::Data1D{0.3051749232558545}, RC::Data1D{0.40921475211927183}, RC::Data1D{0.2970981944245862}, RC::Data1D{0.9408918698675943}}, RC::Data2D{RC::Data1D{0.6662588066265016}, RC::Data1D{0.2643589648720276}, RC::Data1D{0.6957333260602729}, RC::Data1D{0.6591385393059856}, RC::Data1D{0.7833981461730087}, RC::Data1D{0.039311979247742435}, RC::Data1D{0.6623986450355152}, RC::Data1D{0.6083675469956017}, RC::Data1D{0.19345165735248993}, RC::Data1D{0.40113614899864347}, RC::Data1D{0.31288440665869466}, RC::Data1D{0.6071958470068434}, RC::Data1D{0.8756612208986079}, RC::Data1D{0.46698051643479843}, RC::Data1D{0.4627731055500225}, RC::Data1D{0.5916121446608102}, RC::Data1D{0.4007410616660061}, RC::Data1D{0.9233533295553957}, RC::Data1D{0.5901043365024932}, RC::Data1D{0.39058974790428913}, RC::Data1D{0.2206912282332124}, RC::Data1D{0.11508990510054884}, RC::Data1D{0.9591215567873334}, RC::Data1D{0.018401206450882057}, RC::Data1D{0.05220425952462904}, RC::Data1D{0.8898685350403006}, RC::Data1D{0.5338572422171098}, RC::Data1D{0.7383689987111797}, RC::Data1D{0.9234473319167914}, RC::Data1D{0.3577769849042336}, RC::Data1D{0.24273636639716434}, RC::Data1D{0.5895492888627749}, RC::Data1D{0.49184290391786634}, RC::Data1D{0.5661880189684898}, RC::Data1D{0.2025598682270181}, RC::Data1D{0.7222494732449778}, RC::Data1D{0.5725745733682045}, RC::Data1D{0.6370271412035828}, RC::Data1D{0.86987733399764}, RC::Data1D{0.4251492106268784}, RC::Data1D{0.8026923936381207}, RC::Data1D{0.5124256822227112}, RC::Data1D{0.44845072213433}, RC::Data1D{0.6414445062001107}, RC::Data1D{0.6058492492689059}, RC::Data1D{0.25981386111309623}, RC::Data1D{0.18027784940562874}, RC::Data1D{0.008151048834639463}, RC::Data1D{0.8876357130904371}, RC::Data1D{0.6363321348129738}, RC::Data1D{0.6352837144045611}, RC::Data1D{0.9479481280154636}, RC::Data1D{0.9817317080447384}, RC::Data1D{0.08985029602127836}, RC::Data1D{0.19589894521503326}, RC::Data1D{0.6106448835929589}, RC::Data1D{0.025533501129097136}, RC::Data1D{0.14569990187235826}, RC::Data1D{0.32128493358283083}, RC::Data1D{0.14410118702109442}, RC::Data1D{0.7852192315273131}, RC::Data1D{0.05428215232152778}, RC::Data1D{0.0928868510143146}, RC::Data1D{0.18984805413733585}, RC::Data1D{0.8868629846205199}, RC::Data1D{0.877908472749481}, RC::Data1D{0.10563318351630602}, RC::Data1D{0.7773219428439463}, RC::Data1D{0.6771411272801244}, RC::Data1D{0.04170828923400771}, RC::Data1D{0.7272542665130718}, RC::Data1D{0.1583649008506519}, RC::Data1D{0.699758322440847}, RC::Data1D{0.3246197460825877}, RC::Data1D{0.1000562246825265}, RC::Data1D{0.13833995456007886}, RC::Data1D{0.9907467950380413}, RC::Data1D{0.2933587226368979}, RC::Data1D{0.9829748630818828}, RC::Data1D{0.42131636951254203}, RC::Data1D{0.25398668787825585}, RC::Data1D{0.30301825793253445}, RC::Data1D{0.7809082586929389}, RC::Data1D{0.06734328189040562}, RC::Data1D{0.3251710356526655}, RC::Data1D{0.5239167987145126}, RC::Data1D{0.064155004339569}, RC::Data1D{0.32549219072910585}, RC::Data1D{0.6862921770169642}, RC::Data1D{0.6049350847362417}, RC::Data1D{0.4169506661986955}, RC::Data1D{0.6668435703926752}, RC::Data1D{0.26118764143180395}, RC::Data1D{0.9415175854284862}, RC::Data1D{0.464255928908329}, RC::Data1D{0.6752283071991559}, RC::Data1D{0.5054996343268079}, RC::Data1D{0.5491611078354569}, RC::Data1D{0.9505670705720808}, RC::Data1D{0.006297960099682087}, RC::Data1D{0.6072334214163643}, RC::Data1D{0.42079977857443696}, RC::Data1D{0.4744603542082886}, RC::Data1D{0.7026928996124954}, RC::Data1D{0.4994915700774465}, RC::Data1D{0.5781522535070858}, RC::Data1D{0.29741087238148123}, RC::Data1D{0.12204101012091517}, RC::Data1D{0.5514153897505508}, RC::Data1D{0.7918614086964032}, RC::Data1D{0.21738273307741274}, RC::Data1D{0.3749149574805206}, RC::Data1D{0.02470084043788623}, RC::Data1D{0.3565683391878959}, RC::Data1D{0.18359573794899597}, RC::Data1D{0.8874424364852723}, RC::Data1D{0.9918917804733272}, RC::Data1D{0.9303873888149695}, RC::Data1D{0.5662406466640114}, RC::Data1D{0.1404220024469609}, RC::Data1D{0.5001713403786842}, RC::Data1D{0.32483811985981714}, RC::Data1D{0.9762476131615181}, RC::Data1D{0.46836876691248697}, RC::Data1D{0.6227128015946007}, RC::Data1D{0.8266224019128611}, RC::Data1D{0.7698967723978861}, RC::Data1D{0.9951421276513502}, RC::Data1D{0.7467858135749883}, RC::Data1D{0.0824859473728734}, RC::Data1D{0.9929977601538718}, RC::Data1D{0.8927706064349838}, RC::Data1D{0.43801060386925694}, RC::Data1D{0.8511616819280524}, RC::Data1D{0.08646288372725464}, RC::Data1D{0.1513308652311922}, RC::Data1D{0.3351002473507182}, RC::Data1D{0.19402728356921917}, RC::Data1D{0.6675435361191865}, RC::Data1D{0.4205790363347879}, RC::Data1D{0.6549410217537184}, RC::Data1D{0.7024054082507962}, RC::Data1D{0.1517420911814693}, RC::Data1D{0.48191979158456355}, RC::Data1D{0.2008431669672095}, RC::Data1D{0.17987632324044478}, RC::Data1D{0.9617360008213933}, RC::Data1D{0.5889498399042695}, RC::Data1D{0.5024489948945515}, RC::Data1D{0.0646137207356019}, RC::Data1D{0.8338892541761177}, RC::Data1D{0.3467485428431587}, RC::Data1D{0.8454609598261129}, RC::Data1D{0.9061720873254022}, RC::Data1D{0.6262998941358271}, RC::Data1D{0.632659144351761}, RC::Data1D{0.9002990342374483}, RC::Data1D{0.6448350646323764}, RC::Data1D{0.9389429173667345}, RC::Data1D{0.19060970747876815}, RC::Data1D{0.1902238441854458}, RC::Data1D{0.6253344925580565}, RC::Data1D{0.32426483426297004}, RC::Data1D{0.8569968560410224}, RC::Data1D{0.6721258017826628}, RC::Data1D{0.9375933002330878}, RC::Data1D{0.6876445876222692}, RC::Data1D{0.32282813576118574}, RC::Data1D{0.4531344647187566}, RC::Data1D{0.8791465845822485}, RC::Data1D{0.4140594156521089}, RC::Data1D{0.7913286567594607}, RC::Data1D{0.33509180123823834}, RC::Data1D{0.5322801844341958}, RC::Data1D{0.22839553200339402}, RC::Data1D{0.2838978626915084}, RC::Data1D{0.9627014382369375}}};


    auto in_powers = RC::MakeAPtr<EEGPowers>(sampling_rate);
    in_powers->data = in_data;
    auto in_powers_captr = in_powers.ExtractConst();
    in_powers_captr->Print(2, 3);

    // Run classifier
    double result = classifier.TestClassification(in_powers_captr);
    RC_DEBOUT(result);
  }

  void TestPyBind11() {
    auto& pythonInterface = PythonInterface::GetInstance();
    RC_DEBOUT(pythonInterface.Sqrt(2.0));
  }

  void TestAllCode() {
    //TestLog10Transform();
    //TestLog10TransformWithEpsilon();
    //TestAvgOverTime();
    //TestMirrorEnds();
    //TestRemoveMirrorEnds();
    //TestBipolarReference();
    //TestMorletTransformer();
    //TestMorletTransformerRealData();
    //TestEEGCircularData();
    // TODO: JPB: (need) test binning with negative values too
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
    //TestFindArtifactChannelsRandomData();
    //TestDifferentiate();
    //TestProcess_Handler();
    //TestProcess_HandlerRandomData();
    //TestClassification();
    TestPyBind11();
  }
}

