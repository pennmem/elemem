#ifndef TESTING_H
#define TESTING_H

#include "EEGData.h"
#include "EEGPowers.h"

namespace CML {
  // Helper Functions
  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw();
  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw(size_t sampling_rate); 
  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw(size_t sampling_rate, size_t eventlen, size_t chanlen);
  RC::APtr<const EEGDataRaw> CreateTestingEEGDataRaw(size_t sampling_rate, size_t eventlen, size_t chanlen, int16_t offset);
  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble();
  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble(size_t sampling_rate); 
  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble(size_t sampling_rate, size_t eventlen, size_t chanlen);
  RC::APtr<const EEGDataDouble> CreateTestingEEGDataDouble(size_t sampling_rate, size_t eventlen, size_t chanlen, int16_t offset);
  RC::APtr<const EEGPowers> CreateTestingEEGPowers();
  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate);
  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate, size_t eventlen, size_t chanlen, size_t freqlen);
  RC::APtr<const EEGPowers> CreateTestingEEGPowers(size_t sampling_rate, size_t eventlen, size_t chanlen, size_t freqlen, int16_t offset);
  
  // Data Storage and Binning
  void TestEEGCircularData();
  void TestEEGBinning();

  // Feature Filters
  void TestBipolarReference();  
  void TestMirrorEnds();
  void TestAvgOverTime();
  void TestLog10Transform();
  void TestMorletTransformer();
  void TestRollingStats();
  void TestNormalizePowers();

  void TestAllCode();

  //class TaskClassifierManagerTester : TaskClassifierManager {
  //  public:
  //}; 
}

#endif // TESTING_H
