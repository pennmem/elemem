#ifndef EEGDISPLAY_H
#define EEGDISPLAY_H

#include "CImage.h"
#include "EEGData.h"
#include "RC/Data1D.h"
#include <vector>
#include <variant>


namespace CML {
  class EEGChan {
    public:
    EEGChan() : EEGChan(0) {}
    EEGChan(uint32_t channel) : EEGChan(channel, "") {}
    EEGChan(uint32_t channel, RC::RStr label) : EEGChan(channel, label, channel+1) {}
    EEGChan(uint32_t channel, RC::RStr label, uint32_t index) : channels({channel}), label(label), index(index) {
      if (label.empty()) {
        this->label = RC::RStr(channel+1);
      }
    }

    EEGChan(uint32_t pos, uint32_t neg) : EEGChan(pos, neg, "") {}
    EEGChan(uint32_t pos, uint32_t neg, RC::RStr label) :  EEGChan(pos, neg, label, pos) {}
    EEGChan(uint32_t pos, uint32_t neg, RC::RStr label, uint32_t index) : channels({pos, neg}), label(label), index(index) {
      if (label.empty()) {
        this->label = RC::RStr(pos+1) + "-" + RC::RStr(neg+1);
      }
    }

    RC::Data1D<uint32_t> channels;
    RC::RStr label;
    uint32_t index;
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

