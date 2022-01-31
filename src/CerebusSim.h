// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the Cerebus cbsdk API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CEREBUSSIM_H
#define CEREBUSSIM_H

#include "EEGSource.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace CML {
  // All channel numbers are zero-based.  For user-interfacing use one-based.
  class CerebusSim : public EEGSource {
    public:
    CerebusSim(uint32_t instance_=0);
    ~CerebusSim();

    // Rule of 5.
    CerebusSim(const CerebusSim& other) = delete;
    CerebusSim& operator=(const CerebusSim& other) = delete;
    CerebusSim(CerebusSim&& other) = delete;
    CerebusSim& operator=(CerebusSim&& other);


    void Open();  // Automatic at first use.
    void Close();

    void SetInstance(uint32_t instance);

    void InitializeChannels(size_t sampling_rate_Hz);

    // samprate_index: 0=None, 1=500Hz, 2=1kHz, 3=2kHz, 4=10kHz, 5=30kHz
    void SetChannel(uint16_t channel, uint32_t samprate_index=2);
    void SetChannelRange(uint16_t first_channel, uint16_t last_channel,
        uint32_t samprate_index=2);
    void SetChannels(std::vector<uint16_t> channel_list,
        uint32_t samprate_index=2);

    const std::vector<TrialData>& GetData();


    protected:

    void ClearChannels();
    void ConfigureChannel(uint16_t channel, uint32_t samprate_index);
    void SetTrialConfig();

    void BeOpen();

    uint32_t instance;
    uint16_t first_chan=uint16_t(-1);  // unset
    uint16_t last_chan=0;

    std::vector<TrialData> channel_data;

    uint64_t stub_chan_count = 0;
    const size_t num_analog_chans = 256+16;

    bool is_open = false;
  };
}

#endif

