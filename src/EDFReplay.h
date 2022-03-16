// 2021, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// Reloads edf files and provides them as if they were an EEG source.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef EDFREPLAY_H
#define EDFREPLAY_H

#include "edflib/edflib.h"
#include "EEGSource.h"
#include "RC/Data1D.h"
#include "RC/RStr.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace CML {
  class EDFReplay : public EEGSource {
    public:
    EDFReplay(RC::RStr edf_filename);
    ~EDFReplay();

    // Rule of 5.
    EDFReplay(const EDFReplay& other) = delete;
    EDFReplay& operator=(const EDFReplay& other) = delete;
    EDFReplay(EDFReplay&& other) = delete;
    EDFReplay& operator=(EDFReplay&& other);


    void Close();

    void InitializeChannels(size_t sampling_rate_Hz);
    void StartingExperiment();
    void ExperimentReady();

    const std::vector<TrialData>& GetData();


    protected:

    void Open(bool prebuffer=true);  // Automatic at first use.
    void Prebuffer();
    int16_t ClampInt(int val);
    uint64_t TimeSinceLast_samples();

    size_t sampling_rate = 0;
    int edf_hdl = -1;
    edf_hdr_struct edf_hdr;
    RC::RStr filename;
    std::vector<TrialData> channel_data;  // TODO Do we need this?
    RC::Data1D<RC::Data1D<int>> file_bufs;
    size_t amnt_buffered = 0;
    size_t max_requested = 1024;
  };
}

#endif

