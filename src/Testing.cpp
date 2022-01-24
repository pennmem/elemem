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
    RC::APtr<EEGData> data = new EEGData(sampling_rate);
    auto& datar = data->data;
  
    datar.Resize(chanlen);
    RC_ForIndex(i, datar) {
      datar[i].Resize(eventlen);
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

    PrintEEGData(*in_data);
    PrintEEGData(*out_data);
  }
  
  void TestAvgOverTime() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();
  
    RC::APtr<EEGPowers> out_powers = FeatureFilters::AvgOverTime(in_powers);
  
    PrintEEGPowers(*in_powers);
    PrintEEGPowers(*out_powers);
  }

  void TestLog10Transform() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();

    RC::APtr<EEGPowers> out_powers = FeatureFilters::Log10Transform(in_powers);
  
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
    mor_set.num_events = num_events;
    mor_set.channels = channels;
    mor_set.frequencies = freqs;
    mor_set.sampling_rate = sampling_rate;

    MorletTransformer morlet_transformer;
    morlet_transformer.Setup(mor_set);
    RC::APtr<EEGPowers> out_powers = morlet_transformer.Filter(in_data);

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

    auto out_data = rolling_stats.ZScore(in_powers->data[0][4]);
    RC_DEBOUT(RC::RStr::Join(out_data, ", ") + "\n");

    // means should be 10 through 19
    // std_devs should be 10
    // sample_std_devs should be 14.1421...
    // zscores should be 2.1213...
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

    NormalizePowers normalize_powers(eventlen, chanlen, freqlen);
    normalize_powers.Update(in_powers1);
    normalize_powers.PrintStats();
    normalize_powers.Update(in_powers2);
    normalize_powers.PrintStats();

    RC::APtr<EEGPowers> out_powers = normalize_powers.ZScore(in_powers3);
    PrintEEGPowers(*out_powers);
  }

  void TestAllCode() {
    //TestLog10Transform();
    //TestAvgOverTime();
    //TestMirrorEnds();
    //TestBipolarReference();
    //TestMorletTransformer();
    //TestEEGCircularData();
    //TestEEGBinning();
    //TestRollingStats();
    TestNormalizePowers();
  }
}

