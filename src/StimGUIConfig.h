#ifndef STIMGUICONFIG_H
#define STIMGUICONFIG_H

#include "CereStim.h"
#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "GuiParts.h"
#include <cmath>
#include <QGroupBox>


namespace CML {
  class StimSettings {
    public:
    CSStimChannel params;
    RC::RStr label;
    bool approved;
  };

  class StimConfigBox : public QGroupBox, public RCqt::Worker {
    Q_OBJECT

    public:

    StimConfigBox(RC::Caller<void, const StimSettings&> settings_callback,
      RC::Caller<void> test_stim_callback);

    protected:
    void SetChannel_Handler(const CSStimChannel& minvals,
      const CSStimChannel& maxvals, const RC::RStr& label="");
    void SetParameters_Handler(const CSStimChannel& stim_params);
    void Clear_Handler();

    public:
    RCqt::TaskCaller<const CSStimChannel, const CSStimChannel, const RC::RStr>
      SetChannel = TaskHandler(StimConfigBox::SetChannel_Handler);
    RCqt::TaskCaller<const CSStimChannel> SetParameters =
      TaskHandler(StimConfigBox::SetParameters_Handler);
    RCqt::TaskCaller<> Clear = TaskHandler(StimConfigBox::Clear_Handler);

    // Internal callbacks
    void AmpChanged(const f64& new_amp_mA) {
      settings.params.amplitude = uint16_t(std::round(new_amp_mA*1000));
    }
    void FreqChanged(const int64_t& new_freq_Hz) {
      settings.params.frequency = uint32_t(new_freq_Hz);
    }
    void DurChanged(const int64_t& new_dur_ms) {
      settings.params.duration = uint32_t(new_dur_ms*1000);
    }
    void LabelChanged(const RC::RStr& new_label) {
      settings.label = new_label;
    }

    protected:
    RC::Caller<void, const StimSettings&> settings_callback;
    RC::Ptr<LabeledF64> amp;
    RC::Ptr<LabeledI64> freq;
    RC::Ptr<LabeledI64> dur;
    RC::Ptr<LabeledEdit> lab;
    StimSettings settings;
  };
}


#endif // STIMGUICONFIG_H

