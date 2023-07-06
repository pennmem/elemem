#ifndef EEGDISPLAY_H
#define EEGDISPLAY_H

#include "CImage.h"
#include "EEGData.h"
#include "RC/Data1D.h"
#include <vector>
#include <variant>
#include "ChannelConf.h"


namespace CML {
  class EEGDisplay : public CImage {
    public:

    EEGDisplay(int width, int height);
    virtual ~EEGDisplay();

    RCqt::TaskCaller<RC::APtr<const EEGDataDouble>> UpdateData =
      TaskHandler(EEGDisplay::UpdateData_Handler);

    RCqt::TaskCaller<EEGChan> SetChannel =
      TaskHandler(EEGDisplay::SetChannel_Handler);

    RCqt::TaskCaller<EEGChan> UnsetChannel =
      TaskHandler(EEGDisplay::UnsetChannel_Handler);

    RCqt::TaskCaller<const bool> SetAutoScale =
      TaskHandler(EEGDisplay::SetAutoScale_Handler);
    RCqt::TaskCaller<const i64> SetScale =
      TaskHandler(EEGDisplay::SetScale_Handler);

    RCqt::TaskCaller<> Clear =
      TaskHandler(EEGDisplay::Clear_Handler);

    protected:

    void UpdateData_Handler(RC::APtr<const EEGDataDouble>& new_data);
    void SetChannel_Handler(EEGChan& chan);
    void UnsetChannel_Handler(EEGChan& chan);
    void SetAutoScale_Handler(const bool& on);
    void SetScale_Handler(const i64& val);
    void Clear_Handler();

    void SetSamplingRate(size_t sampling_rate);

    virtual void DrawBackground();
    virtual void DrawOnTop();

    // From cbhwlib.h cbNUM_ANALOG_CHANS
    const size_t num_data_chans = 272;

    uint64_t window_seconds = 4;

    EEGDataDouble data{1000, 0}; // TODO: JPB: (refactor) Maybe this should be changed to pointer (redo EnableChan)
    size_t data_offset = 0;
    RC::Data1D<EEGChan> eeg_channels;

    RC::Time timer;
    size_t update_cnt = 0;

    bool autoscale = false;
    u64 scale_val = 32768;
  };
}

#endif // EEGDISPLAY_H

