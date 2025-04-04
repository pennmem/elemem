#ifndef CHANNELCONF_H
#define CHANNELCONF_H

#include <iostream>
#include "RC/RStr.h"

namespace CML {
  class StimChannel {
    public:
    uint16_t electrode_pos = 0;  // Goes positive first / anodic
    uint16_t electrode_neg = 0;  // Goes negative first / cathodic
    uint16_t amplitude = 0; // Unit 1uA, granularity 100uA for macro.
    uint32_t frequency = 0; // Unit Hz.
    uint32_t duration = 0;  // Unit us.
    float area = 0; // Unit mm^2.
    float burst_frac = 1; // Fraction of 1/burst_slow_freq to stimulate for.
    float burst_slow_freq = 0; // Unit Hz.  Slower envelope freq of bursts.
  };

  class BipolarPair {
    public:
    uint16_t pos = 0;
    uint16_t neg = 0;
  };

  inline std::ostream& operator<< (std::ostream& out, const BipolarPair& bp) {
    out << "(" << (unsigned int)(bp.pos) << ", "
               << (unsigned int)(bp.neg) << ")";
    return out;
  }

  enum class ChanType {None, Mono, Bipolar};

  class EEGChan {
    public:
    EEGChan() {}
    EEGChan(uint16_t channel, uint32_t data_index, RC::RStr label="") {
      SetMono(channel, data_index, label);
    }
    EEGChan(uint16_t pos, uint16_t neg, uint32_t data_index, RC::RStr label="") {
      SetBipolar(pos, neg, data_index, label);
    }

    void SetMono(uint16_t channel, uint32_t new_data_index, RC::RStr label="") {
      chan_type = ChanType::Mono;

      channels[0] = channel;
      data_index = new_data_index;
      if (label.empty()) {
        this->label = RC::RStr(channel+1);
      }
      else {
        this->label = label;
      }
    }

    void SetBipolar(uint16_t pos, uint16_t neg, uint32_t new_data_index, RC::RStr label="") { 
      chan_type = ChanType::Bipolar;
      channels[0] = pos;
      channels[1] = neg;
      data_index = new_data_index;
      if (label.empty()) {
        this->label = RC::RStr(pos+1) + "-" + RC::RStr(neg+1);
      }
      else {
        this->label = label;
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
          return RC::RStr(channels[0] + 1);
        case ChanType::Bipolar:
          return RC::RStr(channels[0] + 1) + "_" + RC::RStr(channels[1] + 1);
      }
      Throw_RC_Error("Unrecognized ChanType.");
    }

    uint32_t GetDataIndex() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get channel data index.");
        case ChanType::Mono:
        case ChanType::Bipolar:
          return data_index;
      }
      Throw_RC_Error("Unrecognized ChanType.");
    }

    RC::RStr GetLabel() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get channel label.");
        case ChanType::Mono:
        case ChanType::Bipolar:
          return label;
      }
      Throw_RC_Error("Unrecognized ChanType.");
    }

    uint16_t GetMonoChannel() const {
      switch (chan_type) {
        case ChanType::None:
          Throw_RC_Error("EEGChan has not been set before trying to get mono channel.");
        case ChanType::Mono:
          return channels[0];
        case ChanType::Bipolar:
          Throw_RC_Error("Trying to get bipolar channel data from mono channel.");
      }
      Throw_RC_Error("Unrecognized ChanType.");
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
      Throw_RC_Error("Unrecognized ChanType.");
    }

    protected:
    ChanType chan_type = ChanType::None;
    RC::Data1D<uint16_t> channels{0, 0};
    uint32_t data_index = 0;
    RC::RStr label = "";
  };

  inline std::ostream& operator<< (std::ostream& out, const EEGChan& chan) {
    out << "(" << static_cast<int>(chan.GetChanType()) << "," << chan.GetName()
        << "," << chan.GetDataIndex() << "," << chan.GetLabel() << ")";
    return out;
  }
}

#endif // CHANNELCONF_H

