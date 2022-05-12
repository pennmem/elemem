#ifndef STIMGUICONFIG_H
#define STIMGUICONFIG_H

#include "ChannelConf.h"
#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "Settings.h"
#include "GuiParts.h"
#include <cmath>
#include <QGroupBox>


namespace CML {
  class StimConfigBox : public QGroupBox, public RCqt::Worker {
    Q_OBJECT

    public:

    StimConfigBox(
      RC::Caller<void, const size_t&, const StimSettings&> settings_callback,
      RC::Caller<void, const size_t&> test_stim_callback);

    protected:
    void SetChannel_Handler(const StimChannel& minvals,
      const StimChannel& maxvals, const RC::RStr& label="",
      const RC::RStr& stimtag="", const size_t& index=0);
    void SetParameters_Handler(const StimChannel& stim_params);
    void Clear_Handler();
    void SetEnabled_Handler(const bool& enabled);

    public:
    RCqt::TaskCaller<const StimChannel, const StimChannel,
      const RC::RStr, const RC::RStr, const size_t>
      SetChannel = TaskHandler(StimConfigBox::SetChannel_Handler);
    RCqt::TaskCaller<const StimChannel> SetParameters =
      TaskHandler(StimConfigBox::SetParameters_Handler);
    RCqt::TaskCaller<> Clear = TaskHandler(StimConfigBox::Clear_Handler);
    RCqt::TaskCaller<const bool> SetEnabled =
      TaskHandler(StimConfigBox::SetEnabled_Handler);

    // Internal callbacks
    void AmpChanged(const f64& new_amp_mA) {
      if (disabled) { return; }
      settings.params.amplitude = uint16_t(std::round(new_amp_mA*1000));
      settings_callback(config_index, settings);
    }
    void FreqChanged(const int64_t& new_freq_Hz) {
      if (disabled) { return; }
      settings.params.frequency = uint32_t(new_freq_Hz);
      settings_callback(config_index, settings);
    }
    void DurChanged(const int64_t& new_dur_ms) {
      if (disabled) { return; }
      settings.params.duration = uint32_t(new_dur_ms*1000);
      settings_callback(config_index, settings);
    }
    void LabelChanged(const RC::RStr& new_label) {
      if (disabled) { return; }
      settings.label = new_label;
      settings_callback(config_index, settings);
    }
    void TestStim() {
      if (disabled) { return; }
      test_stim_callback(config_index);
    }
    void ApproveChanged(const bool& state) {
      if (disabled) { return; }
      settings.approved = state;
      settings_callback(config_index, settings);
    }

    QSize sizeHint() const;

    protected:
    RC::Caller<void, const size_t&, const StimSettings&> settings_callback;
    RC::Caller<void, const size_t&> test_stim_callback;
    RC::Ptr<LabeledF64> amp;
    RC::Ptr<LabeledI64> freq;
    RC::Ptr<LabeledI64> dur;
    RC::Ptr<LabeledEdit> lab;
    RC::Ptr<Button> test_stim;
    RC::Ptr<CheckBox> approve_check;
    bool disabled = true;
    bool label_locked = true;
    size_t config_index = size_t(-1);
    StimSettings settings = {};
  };

  class MinMaxStimConfigBox : public QGroupBox, public RCqt::Worker {
    Q_OBJECT

    public:

    MinMaxStimConfigBox(
      RC::Caller<void, const size_t&, const StimSettings&> settings_callback,
      RC::Caller<void, const size_t&> test_stim_callback);

    protected:
    void SetChannel_Handler(const StimChannel& minvals,
      const StimChannel& maxvals, const RC::RStr& label="",
      const RC::RStr& stimtag="", const size_t& index=0);
    void SetParameters_Handler(const StimChannel& stim_params);
    void Clear_Handler();
    void SetEnabled_Handler(const bool& enabled);

    public:
    RCqt::TaskCaller<const StimChannel, const StimChannel,
      const RC::RStr, const RC::RStr, const size_t>
      SetChannel = TaskHandler(MinMaxStimConfigBox::SetChannel_Handler);
    RCqt::TaskCaller<const StimChannel> SetParameters =
      TaskHandler(MinMaxStimConfigBox::SetParameters_Handler);
    RCqt::TaskCaller<> Clear = TaskHandler(MinMaxStimConfigBox::Clear_Handler);
    RCqt::TaskCaller<const bool> SetEnabled =
      TaskHandler(MinMaxStimConfigBox::SetEnabled_Handler);

    // Internal callbacks
    void AmpChanged(const f64& new_amp_mA) {
      if (disabled) { return; }
      settings.params.amplitude = uint16_t(std::round(new_amp_mA*1000));
      settings_callback(config_index, settings);
    }
    void FreqChanged(const int64_t& new_freq_Hz) {
      if (disabled) { return; }
      settings.params.frequency = uint32_t(new_freq_Hz);
      settings_callback(config_index, settings);
    }
    void DurChanged(const int64_t& new_dur_ms) {
      if (disabled) { return; }
      settings.params.duration = uint32_t(new_dur_ms*1000);
      settings_callback(config_index, settings);
    }
    void LabelChanged(const RC::RStr& new_label) {
      if (disabled) { return; }
      settings.label = new_label;
      settings_callback(config_index, settings);
    }
    void TestStim() {
      if (disabled) { return; }
      test_stim_callback(config_index);
    }
    void ApproveChanged(const bool& state) {
      if (disabled) { return; }
      settings.approved = state;
      settings_callback(config_index, settings);
    }

    QSize sizeHint() const;

    protected:
    RC::Caller<void, const size_t&, const StimSettings&> settings_callback;
    RC::Caller<void, const size_t&> test_stim_callback;
    RC::Ptr<LabeledF64> amp;
    RC::Ptr<LabeledI64> freq;
    RC::Ptr<LabeledI64> dur;
    RC::Ptr<LabeledEdit> lab;
    RC::Ptr<Button> test_stim;
    RC::Ptr<CheckBox> approve_check;
    bool disabled = true;
    bool label_locked = true;
    size_t config_index = size_t(-1);
    StimSettings settings = {};
  };
}


#endif // STIMGUICONFIG_H

