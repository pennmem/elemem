// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file probides a stub emulating the Cerebus unit from Blackrock.
//
/////////////////////////////////////////////////////////////////////////////

#include "CerebusSim.h"
#include "Popup.h"
#include "RC/RTime.h"
#include "RC/Errors.h"
#include <cmath>

namespace CML {
  uint64_t TimeSinceLast_ms() {
    static uint64_t last = uint64_t(RC::Time::Get()*1000);
    uint64_t cur = uint64_t(RC::Time::Get()*1000);
    uint64_t diff = cur - last;
    last = cur;
    return diff;
  }


  CerebusSim::CerebusSim(uint32_t instance_)
      : instance(instance_) {
  }


  CerebusSim::~CerebusSim() {
    Close();
  }


  void CerebusSim::Open() {
    static bool first_run = true;
    if (first_run) {
      TimeSinceLast_ms();
      stub_chan_count = 0;
      PopupWin("CerebusSim simulator activated", "Warning");
    }

    is_open = true;
  }


  void CerebusSim::Close() {
    if (is_open) {
      ClearChannels();

      is_open = false;
    }
  }


  void CerebusSim::BeOpen() {
    if (!is_open) {
      Open();
    }
  }


  void CerebusSim::SetInstance(uint32_t instance_) {
    Close();
    instance = instance_;
  }


  void CerebusSim::InitializeChannels(size_t sampling_rate_Hz) {
    BeOpen();

    ClearChannels();

    uint32_t samprate_index = 0;

    // These are the only values the NeuroPort can handle.
    switch (sampling_rate_Hz) {
      case 500:   samprate_index = 1; break;
      case 1000:  samprate_index = 2; break;
      case 2000:  samprate_index = 3; break;
      case 10000: samprate_index = 4; break;
      case 30000: samprate_index = 5; break;
      default: Throw_RC_Type(File, "Configuration selected invalid sampling "
                 "rate.  Allowed values are 500, 1000, 2000, 10000, 30000.");
    }

    first_chan = 0;
    last_chan = 255;
    for (uint16_t c=first_chan; c<=last_chan; c++) {
      ConfigureChannel(c, samprate_index);
    }

    SetTrialConfig();
  }


  void CerebusSim::SetChannel(uint16_t channel, uint32_t samprate_index) {
    BeOpen();

    ClearChannels();

    ConfigureChannel(channel, samprate_index);
    first_chan = channel;
    last_chan = channel;

    SetTrialConfig();
  }


  void CerebusSim::SetChannelRange(uint16_t first_channel, uint16_t last_channel,
      uint32_t samprate_index) {
    if (first_channel > last_channel) {
      throw std::runtime_error("Invalid channel range");
    }

    BeOpen();

    first_chan = first_channel;
    last_chan = last_channel;

    ClearChannels();

    for (uint16_t c=first_channel; c<last_channel; c++) {
      ConfigureChannel(c, samprate_index);
    }

    SetTrialConfig();
  }


  void CerebusSim::SetChannels(std::vector<uint16_t> channel_list,
      uint32_t samprate_index) {
    BeOpen();

    ClearChannels();

    for (size_t i=0; i<channel_list.size(); i++) {
      first_chan = std::min(first_chan, channel_list[i]);
      last_chan = std::max(last_chan, channel_list[i]);
      ConfigureChannel(channel_list[i], samprate_index);
    }

    SetTrialConfig();
  }

  uint32_t StubSampRateStash(uint32_t set=uint32_t(-1)) {
    static uint32_t samprate_index = 0;
    if (set != uint32_t(-1)) {
      samprate_index = set;
    }
    return samprate_index;
  }

  const std::vector<TrialData>& CerebusSim::GetData() {
    if (first_chan > last_chan) {
      throw std::runtime_error("Set channels before getting data");
    }

    BeOpen();

    size_t data_len = TimeSinceLast_ms(); // 1000Hz
    size_t sim_base_samp = 1000;
    switch(StubSampRateStash()) {
      case 0: data_len = 0;   sim_base_samp = 0;   break;
      case 1: data_len /= 2;  sim_base_samp /= 2;  break;
      case 2: break;
      case 3: data_len *= 2;  sim_base_samp *= 2;  break;
      case 4: data_len *= 10; sim_base_samp *= 10; break;
      case 5: data_len *= 30; sim_base_samp *= 30; break;
      default: Throw_RC_Error("Unsupported stub sampling rate");
    }

    channel_data.resize(stub_chan_count);
    for (uint32_t c=0; c<channel_data.size(); c++) {
      channel_data[c].data.resize(data_len);
      for (uint64_t d=0; d<channel_data[c].data.size(); d++) {
        double wt = 3*3.14159265358979*(sim_offset+d) / double(sim_base_samp);
        wt += c;
        wt *= 0.79*(c % 5);
        channel_data[c].data[d] = int16_t((1000-100*(c%7))*std::sin(wt));
      }
    }
    sim_offset += data_len;

    return channel_data;
  }


  void CerebusSim::ClearChannels() {
    first_chan=uint16_t(-1);  // unset
    last_chan=0;

    stub_chan_count = 0;

    channel_data.resize(num_analog_chans);
    for (uint32_t c=0; c<num_analog_chans; c++) {
      channel_data[c].data.resize(0);
    }
  }

  void CerebusSim::ConfigureChannel(uint16_t channel, uint32_t samprate_index) {
    if (stub_chan_count > channel_data.size()) {
      throw std::runtime_error("ConfigureChannel count exceeded maximum.");
    }

    StubSampRateStash(samprate_index);

    channel_data[stub_chan_count].chan = channel;
    stub_chan_count++;
  }

  void CerebusSim::SetTrialConfig() {
    TimeSinceLast_ms();
  }
}

