// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the Cerebus cbsdk API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#include "Cerebus.h"

#include <algorithm>
#include <cstdint>
#include <ctime>
#include <limits>
#include <utility>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#endif

namespace CML {
  /// Sleeps for a floating point number of seconds, with sub-second
  /// precision to the limit of the system.
  static inline void CSleep(double seconds) {
    if (seconds <= 0) {
      return;
    }
  #ifdef WIN32
    ::Sleep(static_cast<unsigned long>(seconds*1000+0.5));
  #else // POSIX
    struct timespec req;
    seconds += 0.5e-9;
    req.tv_sec = time_t(seconds);
    req.tv_nsec = long(1e9*(seconds-(uint64_t)(seconds)));
    nanosleep(&req, NULL);
  #endif
  }

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


  Cerebus::Cerebus(uint32_t chan_count,
      const std::vector<uint32_t>& extra_chans, bool hardware_lnc,
      uint32_t instance_)
      : instance(instance_), chan_count(chan_count), extra_chans(extra_chans),
        hardware_lnc(hardware_lnc) {
    for (auto& chan : this->extra_chans) {
      chan -= 1;
    }
  }


  Cerebus::~Cerebus() {
    Close();
  }


  void Cerebus::Open() {
    if (is_open) {
      Close();
    }

    cbSdkResult res = cbSdkOpen(instance, CBSDKCONNECTION_UDP);

    if (res == CBSDKRESULT_TIMEOUT) {
      cbSdkClose(instance);
      CSleep(1);
      res = cbSdkOpen(instance, CBSDKCONNECTION_DEFAULT);
    }

    if (res != CBSDKRESULT_SUCCESS) {
      throw CBException(res, "Open of Neuroport failed.", instance);
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


  void Cerebus::InitializeChannels(size_t sampling_rate_Hz) {
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
      default: throw std::runtime_error("Configuration selected invalid sampling "
                 "rate.  Allowed values are 500, 1000, 2000, 10000, 30000.");
    }

    first_chan = 0;
    last_chan = chan_count-1;
    if (last_chan > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error("Last channel out of 16-bit bounds.");
    }
    for (uint16_t c=uint16_t(first_chan); c<=last_chan; c++) {
      ConfigureChannel(c, samprate_index);
    }

    for (uint32_t c : extra_chans) {
      ConfigureChannel(static_cast<uint16_t>(c), samprate_index);
    }

    CSleep(0.5);

    SetTrialConfig();

    CSleep(0.75);

    SyncChannels();
  }


  void Cerebus::SetChannel(uint16_t channel, uint32_t samprate_index) {
    BeOpen();

    ClearChannels();

    ConfigureChannel(channel, samprate_index);
    first_chan = channel;
    last_chan = channel;

    SetTrialConfig();
  }

  void Cerebus::SetChannelRange(uint16_t first_channel, uint16_t last_channel,
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


  void Cerebus::SetChannels(std::vector<uint16_t> channel_list,
      uint32_t samprate_index) {
    BeOpen();

    ClearChannels();

    for (size_t i=0; i<channel_list.size(); i++) {
      first_chan = std::min(first_chan, uint32_t(channel_list[i]));
      last_chan = std::max(last_chan, uint32_t(channel_list[i]));
      ConfigureChannel(channel_list[i], samprate_index);
    }

    SetTrialConfig();
  }


  const std::vector<TrialData>& Cerebus::GetData() {
    if (first_chan > last_chan) {
      throw std::runtime_error("Set channels before getting data");
    }

    BeOpen();

    // Set vector sizes to maximum allowed.
    channel_data.resize(cbNUM_ANALOG_CHANS);
    for (uint32_t c=0; c<channel_data.size(); c++) {
      channel_data[c].data.resize(cbSdk_CONTINUOUS_DATA_SAMPLES);
      trial.samples[c] = reinterpret_cast<void*>(channel_data[c].data.data());
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

    // Pre-mark all channels as -1.
    for (uint32_t c=trial.count; c<channel_data.size(); c++) {
      channel_data[c].chan = uint16_t(-1);
    }

    for (uint32_t c=0; c<trial.count; c++) {
      size_t cnum = trial.chan[c]-1;
      // Set vector sizes to fit actually acquired data.
      if (cnum >= channel_data.size()) {
        throw std::runtime_error("Neuroport provided channel number out "
                                 "of cbsdk supported range.");
      }
      channel_data[c].chan = uint16_t(cnum);
      channel_data[c].data.resize(trial.num_samples[c]);
    }

    // Clear all vectors for channels still marked as -1.
    for (uint32_t c=0; c<channel_data.size(); c++) {
      if (channel_data[c].chan == uint16_t(-1)) {
        channel_data[c].data.resize(0);
      }
    }

    return channel_data;
  }


  void Cerebus::ClearChannels() {
    first_chan=uint16_t(-1);  // unset
    last_chan=0;

    channel_data.resize(cbNUM_ANALOG_CHANS);
    for (uint32_t c=0; c<cbNUM_ANALOG_CHANS; c++) {
      channel_data[c].data.resize(0);

      cbPKT_CHANINFO channel_info;

      cbSdkResult res;
      res = cbSdkGetChannelConfig(instance, uint16_t(c+1), &channel_info);

      // Enable hardware line noise correction.
      // Enable DC offset correction.
      channel_info.ainpopts = cbAINP_OFFSET_CORRECT;
      if (hardware_lnc) {
        channel_info.ainpopts |= cbAINP_LNC_RUN_HARD;
      }

      if (res == CBSDKRESULT_SUCCESS) {
        // 0=None, 1=500Hz, 2=1kHz, 3=2kHz, 4=10kHz, 5=30kHz
        channel_info.smpgroup = 0; // Disable.

        res = cbSdkSetChannelConfig(instance, uint16_t(c+1), &channel_info);
      }
      // Ignore errors, not all channels are valid.
    }
  }

  // samprate_index: 0=None, 1=500Hz, 2=1kHz, 3=2kHz, 4=10kHz, 5=30kHz
  // default is 2=1kHz
  void Cerebus::ConfigureChannel(uint16_t channel, uint32_t samprate_index) {
    cbPKT_CHANINFO channel_info;

    cbSdkResult res;
    res = cbSdkGetChannelConfig(instance, channel+1, &channel_info);

    if (res == CBSDKRESULT_SUCCESS) {
      channel_info.smpgroup = samprate_index; // Continuous sampling.

      res = cbSdkSetChannelConfig(instance, channel+1, &channel_info);
    }

    if (res != CBSDKRESULT_SUCCESS) {
      throw CBException(res, "channel config", instance);
    }
  }

  void Cerebus::SetTrialConfig() {
    cbSdkResult res;
    res = cbSdkSetTrialConfig(instance, 1, 0, 0, 0, 0,
        0, 0, false, 0, cbSdk_CONTINUOUS_DATA_SAMPLES, 0, 0, 0, true);

    if (res != CBSDKRESULT_SUCCESS) {
      throw CBException(res, "cbSdkSetTrialConfig", instance);
    }
  }

  template<class T> inline void CerebusUnusedVar(const T&) { }

  void Cerebus::SyncChannels() {
    bool synched = false;
    float sample_time = 0.01f;
    size_t runs = size_t(3/sample_time + 0.5);  // Max 3 seconds.
    uint32_t first_bad = uint32_t(-1);
    size_t data_len = 0;

    // Purge data.
    for (size_t r=0; r < 25; r++) {
      CSleep(0.01);
      const std::vector<TrialData>& data = GetData();
      CerebusUnusedVar(data);
    }

    // Enforce synchronization.
    for (size_t r=0; r < runs && (!synched); r++) {
      CSleep(0.01);
      const std::vector<TrialData>& data = GetData();

      data_len = 0;
      synched = true;

      auto check_if_bad = [&](auto c) {
        if (data[c].chan == uint16_t(-1)) {
          return false;
        }
        if (data_len == 0) {
          data_len = data[c].data.size();
        }
        else if (data[c].data.size() != data_len) {
          synched = false;
          first_bad = c;
          return true;
        }
        return false;
      };

      for (uint16_t c=uint16_t(first_chan); c<=last_chan; c++) {
        if (check_if_bad(c)) {
          break;
        }
      }

      if (synched) {
        for (uint32_t c : extra_chans) {
          if (check_if_bad(c)) {
            break;
          }
        }
      }
    }

    if ( ! synched ) {
      if (data_len == 0) {
        throw std::runtime_error("No data on any channels after initialize!");
      }
      else {
        throw std::runtime_error(
            std::string("Could not obtain synchronized data across channels.")
            + std::string("  First channel with mismatched data count: ")
            + std::to_string(first_bad));
      }
    }
  }
}

