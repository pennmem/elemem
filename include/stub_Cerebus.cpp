// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file probides a stub emulating the Cerebus unit from Blackrock.
//
/////////////////////////////////////////////////////////////////////////////

#include "Cerebus.h"
#include "Popup.h"
#include "RC/RTime.h"
#include "RC/Errors.h"

namespace CML {
  uint64_t TimeSinceLast_ms() {
    static uint64_t last = uint64_t(RC::Time::Get()*1000);
    uint64_t cur = uint64_t(RC::Time::Get()*1000);
    uint64_t diff = cur - last;
    last = cur;
    return diff;
  }

  // Here so Cerebus.h is shared for real and stub.
  uint64_t stub_chan_count = 0;

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
    static bool first_run = true;
    if (first_run) {
      TimeSinceLast_ms();
      stub_chan_count = 0;
      PopupWin("Cerebus simulator activated", "Warning");
    }

    is_open = true;
  }


  void Cerebus::Close() {
    if (is_open) {
      ClearChannels();

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

  const std::vector<TrialData>& Cerebus::GetData() {
    if (first_chan > last_chan) {
      throw std::runtime_error("Set channels before getting data");
    }

    BeOpen();

    size_t data_len = TimeSinceLast_ms(); // 1000Hz
    switch(StubSampRateStash()) {
      case 0: data_len = 0; break;
      case 1: data_len /= 2; break;
      case 2: break;
      case 3: data_len *= 2; break;
      case 4: data_len *= 10; break;
      case 5: data_len *= 30; break;
      default: Throw_RC_Error("Unsupported stub sampling rate");
    }

    channel_data.resize(stub_chan_count);
    for (uint32_t c=0; c<channel_data.size(); c++) {
      channel_data[c].data.resize(data_len);
      for (uint64_t d=0; d<channel_data[c].data.size(); d++) {
        channel_data[c].data[d] = d;
      }
    }

    return channel_data;
  }


  void Cerebus::ClearChannels() {
    first_chan=uint16_t(-1);  // unset
    last_chan=0;

    stub_chan_count = 0;

    channel_data.resize(cbNUM_ANALOG_CHANS);
    for (uint32_t c=0; c<cbNUM_ANALOG_CHANS; c++) {
      channel_data[c].data.resize(0);
    }
  }

  void Cerebus::ConfigureChannel(uint16_t channel, uint32_t samprate_index) {
    if (stub_chan_count > channel_data.size()) {
      throw std::runtime_error("ConfigureChannel count exceeded maximum.");
    }

    StubSampRateStash(samprate_index);

    channel_data[stub_chan_count].chan = channel;
    stub_chan_count++;
  }

  void Cerebus::SetTrialConfig() {
    TimeSinceLast_ms();
  }
}

