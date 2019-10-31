#ifndef EEGDISPLAY_H
#define EEGDISPLAY_H

#include "CImage.h"
#include "RC/Data1D.h"
#include <vector>


namespace CML {
  typedef RC::Data1D<RC::Data1D<float>> EEGData;

  class EEGChan {
    public:
    EEGChan(uint32_t channel=0, RC::RStr label="")
      : channel(channel), label(label) {
    }
    uint32_t channel;
    RC::RStr label;
  };

  class EEGDisplay : public CImage {
    public:

    EEGDisplay(int width, int height);
    virtual ~EEGDisplay();

    RCqt::TaskCaller<EEGData> UpdateData =
      TaskHandler(EEGDisplay::UpdateData_Handler);

    RCqt::TaskCaller<EEGChan> SetChannel =
      TaskHandler(EEGDisplay::SetChannel_Handler);

    RCqt::TaskCaller<EEGChan> UnsetChannel =
      TaskHandler(EEGDisplay::UnsetChannel_Handler);

    protected:

    void UpdateData_Handler(EEGData& new_data);
    void SetChannel_Handler(EEGChan& chan);
    void UnsetChannel_Handler(EEGChan& chan);

    virtual void DrawBackground();
    virtual void DrawOnTop();

    // From cbhwlib.h cbNUM_ANALOG_CHANS
    const size_t num_data_chans = 272;
    // 6 seconds at 1000Hz
    size_t data_samples = 6000;

    EEGData data;
    size_t data_offset = 0;
    RC::Data1D<EEGChan> eeg_channels;

  };
}

#endif // EEGDISPLAY_H

