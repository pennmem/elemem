// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the Cerebus cbsdk API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CEREBUS_H
#define CEREBUS_H

#include "cbsdk.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef WIN32
#include <unistd.h>
#ifndef Sleep
#define Sleep(x) usleep((x) * 1000)
#endif
#endif

namespace CML {
  // All channel numbers are zero-based.  For user-interfacing use one-based.
  class Cerebus {
    public:
    Cerebus(uint32_t instance_=0)
      : instance(instance_) {

      cbSdkResult res = cbSdkOpen(instance, CBSDKCONNECTION_DEFAULT);
      
      if (res != CBSDKRESULT_SUCCESS) {
        throw std::runtime_error(
            std::string("cbSdkOpen failed, instance ") +
            std::to_string(instance)
        );
      }

      for (int i=0; i<cbNUM_ANALOG_CHANS; i++) {
        trial.samples[i] = channel_data[i].data();
      }
    }

    ~Cerebus() {
      cbSdkClose(instance);
    }

    // Rule of 3.
    Cerebus(const Cerebus& other) = delete;
    Cerebus& operator=(const Cerebus& other) = delete;

    void SetChannel(uint16_t channel) {
      ClearChannels();

      ConfigureChannel(channel);
      first_chan = channel;
      last_chan = channel;

      SetTrialConfig();
    }

    void SetChannelRange(uint16_t first_channel, uint16_t last_channel) {
      if (first_channel > last_channel) {
        throw std::runtime_error("Invalid channel range");
      }

      first_chan = first_channel;
      last_chan = last_channel;

      ClearChannels();

      for (uint32_t c=first_channel; c<last_channel; c++) {
        ConfigureChannel(c);
      }

      SetTrialConfig();
    }

    void SetChannels(std::vector<uint16_t> channel_list) {
      ClearChannels();

      for (size_t i=0; i<channel_list.size(); i++) {
        first_chan = std::min(first_chan, channel_list[i]);
        last_chan = std::max(last_chan, channel_list[i]);
        ConfigureChannel(channel_list[i]);
      }

      SetTrialConfig();
    }

    const std::vector<std::vector<int16_t>>& GetData() {
      if (first_chan > last_chan) {
        throw std::runtime_error("Set channels before getting data");
      }

      // Set vector sizes to maximum allowed.
      for (uint32_t c=first_chan; c<=last_chan; c++) {
        channel_data[c].resize(cbSdk_CONTINUOUS_DATA_SAMPLES);
      }

      cbSdkResult res = cbSdkInitTrialData(instance, 1, nullptr, &trial,
        nullptr, nullptr);

      if (res == CBSDKRESULT_SUCCESS) {
        res = cbSdkGetTrialData(instance, 1, nullptr, &trial, nullptr,
            nullptr);
      }

      if (res != CBSDKRESULT_SUCCESS) {
        throw std::runtime_error(
            std::string("cbSdkGetTrialData failed, instance ") +
            std::to_string(instance)
        );
      }

      // Set vector sizes to actually acquired data.
      for (uint32_t c=first_chan; c<=last_chan; c++) {
        channel_data[c].resize(trial.num_samples[c]);
      }

      return channel_data;
    }

    protected:

    void ClearChannels() {
      return; // TODO - How should this be done?
      cbPKT_CHANINFO channel_info;

      cbSdkResult res;
      res = cbSdkGetChannelConfig(instance, 0, &channel_info);  // 0 == all

      if (res == CBSDKRESULT_SUCCESS) {
        channel_info.smpgroup = 0;

        res = cbSdkSetChannelConfig(instance, 0, &channel_info);
      }

      if (res != CBSDKRESULT_SUCCESS) {
        throw std::runtime_error(
            std::string("cbSdk clear channel config failed, instance ") +
            std::to_string(instance)
        );
      }
    }

    void ConfigureChannel(uint16_t channel) {
      cbPKT_CHANINFO channel_info;

      cbSdkResult res;
      res = cbSdkGetChannelConfig(instance, channel+1, &channel_info);

      if (res == CBSDKRESULT_SUCCESS) {
        channel_info.smpgroup = 5; // Continuous sampling, 30kHz rate.

        res = cbSdkSetChannelConfig(instance, channel+1, &channel_info);
      }

      if (res != CBSDKRESULT_SUCCESS) {
        throw std::runtime_error(
            std::string("cbSdk channel config failed, instance ") +
            std::to_string(instance) + ", channel " + std::to_string(channel+1)
        );
      }
    }

    void SetTrialConfig() {
      cbSdkResult res;
      res = cbSdkSetTrialConfig(instance, 1, first_chan+1, 0, 0, last_chan+1,
          0, 0, false, 0, cbSdk_CONTINUOUS_DATA_SAMPLES, 0, 0, 0, true);

      if (res != CBSDKRESULT_SUCCESS) {
        throw std::runtime_error("cbSdkSetTrialConfig failed");
      }

      // Set size of data to 0 ahead of time for inactive channels.
      for (uint32_t c=0; c<channel_data.size(); c++) {
        if (c < first_chan || c > last_chan) {
          channel_data[c].resize(0);
        }
      }
    }

    uint32_t instance;
    uint16_t first_chan=-1;  // unset
    uint16_t last_chan=0;

    std::vector<std::vector<int16_t>> channel_data{cbNUM_ANALOG_CHANS,
        std::vector<int16_t>(cbSdk_CONTINUOUS_DATA_SAMPLES)};

    cbSdkTrialCont trial{};
  };
}

#endif

