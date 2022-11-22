// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides the interface for EEG sources.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef EEGSOURCE_H
#define EEGSOURCE_H

#include <cstdint>
#include <vector>

namespace CML {
  struct TrialData {
    uint16_t chan;
    std::vector<int16_t> data;
  };

  // All channel numbers are zero-based.  For user-interfacing use one-based.
  class EEGSource {
    public:
    virtual ~EEGSource() = default;

    virtual void Close() {}

    virtual void InitializeChannels(size_t sampling_rate_Hz) = 0;
    virtual void StartingExperiment() {}
    virtual void ExperimentReady() {}

    virtual const std::vector<TrialData>& GetData() = 0;
  };
}

#endif

