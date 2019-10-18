// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the Cerebus cbsdk API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#include "Cerebus.h"

namespace CML {
  CBException::~CBException() { }

  std::string CBException::CodeToString(cbSdkResult err) {
    int err_i = int(err);
    switch(err_i) {
      case 3:
        return "If file conversion is needed";
      case 2:
        return "Library is already closed";
      case 1:
        return "Library is already opened";
      case 0:
        return "Successful operation";
      case -1:
        return "Not implemented";
      case -2:
        return "Unknown error";
      case -3:
        return "Invalid parameter";
      case -4:
        return "Interface is closed cannot do this operation";
      case -5:
        return "Interface is open cannot do this operation";
      case -6:
        return "Null pointer";
      case -7:
        return "Unable to open Central interface";
      case -8:
        return "Unable to open UDP interface (might happen if default)";
      case -9:
        return "Unable to open UDP port";
      case -10:
        return "Unable to allocate RAM for trial cache data";
      case -11:
        return "Unable to open UDP timer thread";
      case -12:
        return "Unable to open Central communication thread";
      case -13:
        return "Invalid channel number";
      case -14:
        return "Comment too long or invalid";
      case -15:
        return "Filename too long or invalid";
      case -16:
        return "Invalid callback type";
      case -17:
        return "Callback register/unregister failed";
      case -18:
        return "Trying to run an unconfigured method";
      case -19:
        return "Invalid trackable id, or trackable not present";
      case -20:
        return "Invalid video source id, or video source not present";
      case -21:
        return "Cannot open file";
      case -22:
        return "Wrong file format";
      case -23:
        return "Socket option error (possibly permission issue)";
      case -24:
        return "Socket memory assignment error";
      case -25:
        return "Invalid range or instrument address";
      case -26:
        return "library memory allocation error";
      case -27:
        return "Library initialization error";
      case -28:
        return "Conection timeout error";
      case -29:
        return "Resource is busy";
      case -30:
        return "Instrument is offline";
      case -31:
        return "The instrument runs an outdated protocol version";
      case -32:
        return "The library is outdated";
      default:
        return "Unrecognized error code";

    }
  }


  Cerebus::Cerebus(uint32_t instance_)
      : instance(instance_) {
  }


  Cerebus::~Cerebus() {
    Close();
  }


  void Cerebus::Open() {
    if (is_open) {
      Close();
    }

    cbSdkResult res = cbSdkOpen(instance, CBSDKCONNECTION_DEFAULT);

    if (res != CBSDKRESULT_SUCCESS) {
      throw CBException(res, "cbSdkOpen", instance);
    }

    for (size_t i=0; i<cbNUM_ANALOG_CHANS; i++) {
      trial.samples[i] = reinterpret_cast<void*>(channel_data[i].data());
    }

    is_open = true;
  }


  void Cerebus::Close() {
    if (is_open) {
      ClearChannels();

      cbSdkClose(instance);

      is_open = false;
    }
  }


  void Cerebus::BeOpen() {
    if (!is_open) {
      Open();
    }
  }


  void Cerebus::SetInstance(uint32_t instance_) {
    Close();
    instance = instance_;
  }


  void Cerebus::SetChannel(uint16_t channel) {
    BeOpen();

    ClearChannels();

    ConfigureChannel(channel);
    first_chan = channel;
    last_chan = channel;

    SetTrialConfig();
  }


  void Cerebus::SetChannelRange(uint16_t first_channel, uint16_t last_channel) {
    if (first_channel > last_channel) {
      throw std::runtime_error("Invalid channel range");
    }

    BeOpen();

    first_chan = first_channel;
    last_chan = last_channel;

    ClearChannels();

    for (uint16_t c=first_channel; c<last_channel; c++) {
      ConfigureChannel(c);
    }

    SetTrialConfig();
  }


  void Cerebus::SetChannels(std::vector<uint16_t> channel_list) {
    BeOpen();

    ClearChannels();

    for (size_t i=0; i<channel_list.size(); i++) {
      first_chan = std::min(first_chan, channel_list[i]);
      last_chan = std::max(last_chan, channel_list[i]);
      ConfigureChannel(channel_list[i]);
    }

    SetTrialConfig();
  }


  const std::vector<std::vector<int16_t>>& Cerebus::GetData() {
    if (first_chan > last_chan) {
      throw std::runtime_error("Set channels before getting data");
    }

    BeOpen();

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
      throw CBException(res, "cbSdkGetTrialData", instance);
    }

    // Set vector sizes to actually acquired data.
    for (uint32_t c=first_chan; c<=last_chan; c++) {
      channel_data[c].resize(trial.num_samples[c]);
    }

    return channel_data;
  }


  void Cerebus::ClearChannels() {
    first_chan=uint16_t(-1);  // unset
    last_chan=0;

    return; // TODO - How should this be done?  Is the rest needed?
/*
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
    }*/
  }

  void Cerebus::ConfigureChannel(uint16_t channel) {
    cbPKT_CHANINFO channel_info;

    cbSdkResult res;
    res = cbSdkGetChannelConfig(instance, channel+1, &channel_info);

    if (res == CBSDKRESULT_SUCCESS) {
      channel_info.smpgroup = 5; // Continuous sampling, 30kHz rate.

      res = cbSdkSetChannelConfig(instance, channel+1, &channel_info);
    }

    if (res != CBSDKRESULT_SUCCESS) {
      throw CBException(res, "channel config", instance);
    }
  }

  void Cerebus::SetTrialConfig() {
    cbSdkResult res;
    res = cbSdkSetTrialConfig(instance, 1, first_chan+1, 0, 0, last_chan+1,
        0, 0, false, 0, cbSdk_CONTINUOUS_DATA_SAMPLES, 0, 0, 0, true);

    if (res != CBSDKRESULT_SUCCESS) {
      throw CBException(res, "cbSdkSetTrialConfig", instance);
    }

    // Set size of data to 0 ahead of time for inactive channels.
    for (uint32_t c=0; c<channel_data.size(); c++) {
      if (c < first_chan || c > last_chan) {
        channel_data[c].resize(0);
      }
    }
  }
}

