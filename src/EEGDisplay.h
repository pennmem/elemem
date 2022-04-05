#ifndef EEGDISPLAY_H
#define EEGDISPLAY_H

#include "CImage.h"
#include "EEGData.h"
#include "RC/Data1D.h"
#include <vector>
#include <variant>


namespace CML {
  enum class ChanType {None, Mono, Bipolar};

  // TODO: JPB: (need) Move to ChannelConf.h
  class EEGChan {
    public:
    // TODO: JPB: (need) Change everything to u8?
    class BipolarPair {
      public:
      uint32_t pos;
      uint32_t neg;
    };

    EEGChan() {}
    EEGChan(uint32_t channel, uint32_t data_index, RC::RStr label="") {
      SetMono(channel, data_index, label);
    }
    EEGChan(uint32_t pos, uint32_t neg, uint32_t data_index, RC::RStr label="") {
      SetBipolar(pos, neg, data_index, label);
    }

    void SetMono(uint32_t channel, uint32_t new_data_index, RC::RStr label="") {
      chan_type = ChanType::Mono;
      channels[0] = channel;
      data_index = new_data_index;
      if (label.empty()) {
        this->label = RC::RStr(channel+1);
      }
    }

    void SetBipolar(uint32_t pos, uint32_t neg, uint32_t new_data_index, RC::RStr label="") { 
      chan_type = ChanType::Bipolar;
      channels[0] = pos;
      channels[1] = neg;
      data_index = new_data_index;
      if (label.empty()) {
        this->label = RC::RStr(pos+1) + "-" + RC::RStr(neg+1);
      }
    }

    ChanType GetChanType() const {
      return chan_type;
    }

    RC::RStr GetName() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get channel name.");
        case ChanType::Mono:
          return RC::RStr(channels[0]+1);
        case ChanType::Bipolar:
          return RC::RStr(channels[0]+1) + "_" + RC::RStr(channels[1]+1);
      }
    }

    uint32_t GetDataIndex() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get channel data index.");
        case ChanType::Mono:
        case ChanType::Bipolar:
          return data_index;
      }
    }

    RC::RStr GetLabel() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get channel label.");
        case ChanType::Mono:
        case ChanType::Bipolar:
          return label;
      }
    }

    uint32_t GetMonoChannel() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get mono channel.");
        case ChanType::Mono:
          return channels[0];
        case ChanType::Bipolar:
          Throw_RC_Error("Trying to get bipolar channel data from mono channel.");
      }
    }

    BipolarPair GetBipolarChannels() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get mono channel.");
        case ChanType::Mono:
          Throw_RC_Error("Trying to get mono channel data from bipolar channel.");
        case ChanType::Bipolar:
          return {.pos=channels[0], .neg=channels[1]};
      }
    }

    protected:
    ChanType chan_type = ChanType::None;
    // TODO: JPB: (need) Change Data1D to std::variant?
    RC::Data1D<uint32_t> channels{0, 0};
    uint32_t data_index = 0;
    RC::RStr label = "";
  };

  class EEGDisplay : public CImage {
    public:

    EEGDisplay(int width, int height);
    virtual ~EEGDisplay();

    RCqt::TaskCaller<RC::APtr<const EEGData>> UpdateData =
      TaskHandler(EEGDisplay::UpdateData_Handler);

    RCqt::TaskCaller<EEGChan> SetChannel =
      TaskHandler(EEGDisplay::SetChannel_Handler);

    RCqt::TaskCaller<EEGChan> UnsetChannel =
      TaskHandler(EEGDisplay::UnsetChannel_Handler);

    RCqt::TaskCaller<> Clear =
      TaskHandler(EEGDisplay::Clear_Handler);

    protected:

    void UpdateData_Handler(RC::APtr<const EEGData>& new_data);
    void SetChannel_Handler(EEGChan& chan);
    void UnsetChannel_Handler(EEGChan& chan);
    void Clear_Handler();

    void SetSamplingRate(size_t sampling_rate);

    virtual void DrawBackground();
    virtual void DrawOnTop();

    // From cbhwlib.h cbNUM_ANALOG_CHANS
    const size_t num_data_chans = 272;

    uint64_t window_seconds = 4;

    EEGData data{1000, 0}; // TODO: JPB: (refactor) Maybe this should be changed to pointer (redo EnableChan)
    size_t data_offset = 0;
    RC::Data1D<EEGChan> eeg_channels;

    RC::Time timer;
    size_t update_cnt = 0;
  };
}

#endif // EEGDISPLAY_H

