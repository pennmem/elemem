#ifndef TESTING_H
#define TESTING_H

#include "FeatureFilters.h"
#include "ChannelConf.h"
#include "TaskClassifierManager.h"
#include "EEGCircularData.h"

namespace CML {

  RC::APtr<const EEGData> CreateTestingEEGData();
  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate); 
  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate, size_t eventlen, size_t chanlen);
  RC::APtr<const EEGPowers> CreateTestingEEGPowers();
  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate);
  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate, size_t eventlen, size_t chanlen, size_t freqlen);


  RC::APtr<const EEGData> CreateTestingEEGData() {
    return CreateTestingEEGData(42);
  }

  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate) {
    return CreateTestingEEGData(sampling_rate, 4, 3);
  }

  RC::APtr<const EEGData> CreateTestingEEGData(size_t sampling_rate, size_t eventlen, size_t chanlen) {
      RC::APtr<EEGData> data = new EEGData(sampling_rate);
      auto& datar = data->data;
  
      datar.Resize(chanlen);
      RC_ForIndex(i, datar) {
        datar[i].Resize(eventlen);
        RC_ForIndex(j, datar[i]) {
          datar[i][j] = i*eventlen + j;
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
      RC::APtr<EEGPowers> powers = new EEGPowers(sampling_rate, eventlen, chanlen, freqlen);
      auto& datar = powers->data;
  
      RC_ForRange(i, 0, freqlen) { // Iterate over freqlen
        RC_ForRange(j, 0, chanlen) { // Iterate over chanlen
          RC_ForRange(k, 0, eventlen) { // Iterate over eventlen
            datar[i][j][k] = i*chanlen*eventlen + j*eventlen + k;
          }
        }
      }
  
      return powers.ExtractConst();
  }
  
  void TestBipolarReference() {
    RC::APtr<const EEGData> in_data = CreateTestingEEGData();
  
    RC::Data1D<BipolarPair> bipolar_reference_channels = {BipolarPair{0,1}, BipolarPair{1,0}, BipolarPair{0,2}, BipolarPair{2,1}}; 
    RStr deb_msg = "";
    RC_ForEach(pair, bipolar_reference_channels) {
      deb_msg += ("(" + RStr(pair.pos) + ", " + RStr(pair.neg) + "), ");
    }
    deb_msg += "\n";
    RC_DEBOUT(deb_msg);

    RC::APtr<const EEGData> out_data = FeatureFilters::BipolarReference(in_data, bipolar_reference_channels);
  
    PrintEEGData(*in_data);
    PrintEEGData(*out_data);
  }
  
  void TestMirrorEnds() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGData> in_data = CreateTestingEEGData(sampling_rate);
    
    RC::APtr<const EEGData> out_data = FeatureFilters::MirrorEnds(in_data, 2);

    PrintEEGData(*in_data);
    PrintEEGData(*out_data);
  }
  
  void TestAvgOverTime() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();
  
    RC::APtr<const EEGPowers> out_powers = FeatureFilters::AvgOverTime(in_powers);
  
    PrintEEGPowers(*in_powers);
    PrintEEGPowers(*out_powers);
  }

  void TestLog10Transform() {
    RC::APtr<const EEGPowers> in_powers = CreateTestingEEGPowers();

    RC::APtr<const EEGPowers> out_powers = FeatureFilters::Log10Transform(in_powers);
  
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
    RC::APtr<const EEGPowers> out_powers = morlet_transformer.Filter(in_data);

    PrintEEGData(*in_data);
    PrintEEGPowers(*out_powers);
  }

  class TaskClassifierManagerTester : TaskClassifierManager {
    public:
  }; 

  void TestEEGCircularData() {
    size_t sampling_rate = 1000;
    RC::APtr<const EEGData> in_data = CreateTestingEEGData(sampling_rate, 4, 3);
    // TODO: JPB: Test different sized arrays
    PrintEEGData(*in_data);

    EEGCircularData circular_data(sampling_rate);
    circular_data.Append(in_data);
    circular_data.PrintData();
    circular_data.PrintRawData();
    circular_data.Append(in_data);
    circular_data.PrintData();
    circular_data.PrintRawData();
    circular_data.Append(in_data);
    circular_data.PrintData();
    circular_data.PrintRawData();
  }

  void TestFeatureFilters() {
    //TestLog10Transform();
    //TestAvgOverTime();
    //TestMirrorEnds();
    //TestBipolarReference();
    //TestMorletTransformer();
    TestEEGCircularData();
  }
}

#endif // TESTING_H
