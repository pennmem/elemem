#include "StimGUIConfig.h"
#include "RC/RCBits.h"
#include <QHBoxLayout>


using namespace RC;

namespace CML {
  StimConfigBox::StimConfigBox(
      Caller<void, const size_t&, const StimSettings&> settings_callback,
      Caller<void, const size_t&> test_stim_callback)
    : settings_callback(settings_callback),
      test_stim_callback(test_stim_callback) {

    setTitle("Not configured");

    RC::Ptr<QVBoxLayout> stim_conf = new QVBoxLayout();

    lab = new LabeledEdit(MakeCaller(this, &StimConfigBox::LabelChanged),
                          "Label");
    stim_conf->addWidget(lab);
    amp = new LabeledF64(MakeCaller(this, &StimConfigBox::AmpChanged),
                         "Amplitude (mA)");
    stim_conf->addWidget(amp);
    freq = new LabeledI64(MakeCaller(this, &StimConfigBox::FreqChanged),
                          "Frequency (Hz)");
    stim_conf->addWidget(freq);
    dur = new LabeledI64(MakeCaller(this, &StimConfigBox::DurChanged),
                         "Duration (ms)");
    stim_conf->addWidget(dur);

    RC::Ptr<QHBoxLayout> stim_approve = new QHBoxLayout();
    test_stim = new Button(
        MakeCaller(this, &StimConfigBox::TestStim), "Test Stim");
    stim_approve->addWidget(test_stim);

    approve_check = new CheckBox(
        MakeCaller(this, &StimConfigBox::ApproveChanged), "Approve Settings");
    stim_approve->addWidget(approve_check);

    stim_conf->addLayout(stim_approve);

    setLayout(stim_conf);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    SetEnabled_Handler(false);
  }

  QSize StimConfigBox::sizeHint() const {
    return QSize(100, 100);
  }


  void StimConfigBox::SetChannel_Handler(const StimChannel& minvals,
      const StimChannel& maxvals, const RC::RStr& label,
      const RC::RStr& stimtag, const size_t& index) {
    disabled = true;
    RC::RStr new_title = RStr(minvals.electrode_pos) + "_" +
          RStr(minvals.electrode_neg);
    if (!stimtag.empty()) {
      new_title += " \"" + stimtag + "\"";
    }
    setTitle(new_title.ToQString());
    settings.params = minvals;
    settings.label = label;
    settings.stimtag = stimtag;
    settings.approved = false;
    amp->SetRange(minvals.amplitude * 1e-3, maxvals.amplitude * 1e-3);
    freq->SetRange(minvals.frequency, maxvals.frequency);
    dur->SetRange(int64_t(minvals.duration * 1e-3 + 0.5), int64_t(maxvals.duration * 1e-3));
    lab->SetDefault(label);
    label_locked = ! label.empty();
    config_index = index;
    approve_check->Set(false);

    SetEnabled_Handler(true);
  }

  void StimConfigBox::SetParameters_Handler(const StimChannel& params) {
    disabled = true;
    settings.params = params;
    amp->Set(params.amplitude * 1e-3);
    freq->Set(params.frequency);
    dur->Set(int64_t(params.duration * 1e-3 + 0.5));
    disabled = false;
  }

  void StimConfigBox::Clear_Handler() {
    disabled = true;
    setTitle("Not configured");
    amp->Set(0);
    freq->Set(0);
    dur->Set(0);
    lab->SetDefault("");
    config_index = size_t(-1);
    settings = {};
  }

  void StimConfigBox::SetEnabled_Handler(const bool& enabled) {
    if (enabled && config_index == size_t(-1)) {
      return;
    }
    amp->SetReadOnly( ! enabled );
    freq->SetReadOnly( ! enabled );
    dur->SetReadOnly( ! enabled );
    lab->SetReadOnly( ( ! enabled ) || label_locked );
    test_stim->SetEnabled(enabled);
    approve_check->SetEnabled(enabled);
  }


  // TODO: extend for stim parameters beyond amplitude
  MinMaxStimConfigBox::MinMaxStimConfigBox(
      Caller<void, const size_t&, const StimSettings&> settings_callback,
      Caller<void, const size_t&> test_stim_callback)
    : settings_callback(settings_callback),
      test_stim_callback(test_stim_callback) {

    setTitle("Not configured");

    RC::Ptr<QVBoxLayout> stim_conf = new QVBoxLayout();

    lab = new LabeledEdit(MakeCaller(this, &MinMaxStimConfigBox::LabelChanged),
                          "Label");
    stim_conf->addWidget(lab);
    amp = new LabeledF64(MakeCaller(this, &MinMaxStimConfigBox::AmpChanged),
                         "Max Amplitude (mA)");
    stim_conf->addWidget(amp);
    freq = new LabeledI64(MakeCaller(this, &MinMaxStimConfigBox::FreqChanged),
                          "Frequency (Hz)");
    stim_conf->addWidget(freq);
    dur = new LabeledI64(MakeCaller(this, &MinMaxStimConfigBox::DurChanged),
                         "Duration (ms)");
    stim_conf->addWidget(dur);

    RC::Ptr<QHBoxLayout> stim_approve = new QHBoxLayout();
    test_stim = new Button(
        MakeCaller(this, &MinMaxStimConfigBox::TestStim), "Test Stim");
    stim_approve->addWidget(test_stim);

    approve_check = new CheckBox(
        MakeCaller(this, &MinMaxStimConfigBox::ApproveChanged), "Approve Settings");
    stim_approve->addWidget(approve_check);

    stim_conf->addLayout(stim_approve);

    setLayout(stim_conf);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    SetEnabled_Handler(false);
  }

  QSize MinMaxStimConfigBox::sizeHint() const {
    return QSize(100, 100);
  }

  void MinMaxStimConfigBox::SetChannel_Handler(const StimChannel& minvals,
      const StimChannel& maxvals, const RC::RStr& label,
      const RC::RStr& stimtag, const size_t& index) {
    RC_DEBOUT(RC::RStr("StimGUIConfig.cpp::SetChannel \n"));
    disabled = true;
    RC::RStr new_title = RStr(minvals.electrode_pos) + "_" +
          RStr(minvals.electrode_neg);
    if (!stimtag.empty()) {
      new_title += " \"" + stimtag + "\"";
    }
    setTitle(new_title.ToQString());
    settings.params = minvals;
    settings.label = label;
    settings.stimtag = stimtag;
    settings.approved = false;
    amp->SetRange(minvals.amplitude * 1e-3, maxvals.amplitude * 1e-3);
    freq->SetRange(minvals.frequency, maxvals.frequency);
    dur->SetRange(int64_t(minvals.duration * 1e-3 + 0.5), int64_t(maxvals.duration * 1e-3));
    lab->SetDefault(label);
    label_locked = ! label.empty();
    config_index = index;
    approve_check->Set(false);

    SetEnabled_Handler(true);
  }

  void MinMaxStimConfigBox::SetParameters_Handler(const StimChannel& params) {
    disabled = true;
    RC_DEBOUT(RC::RStr("StimGUIConfig.cpp::SetParameters \n"));
    settings.params = params;
    amp->Set(params.amplitude * 1e-3);
    freq->Set(params.frequency);
    dur->Set(int64_t(params.duration * 1e-3 + 0.5));
    disabled = false;
  }

  void MinMaxStimConfigBox::Clear_Handler() {
    disabled = true;
    setTitle("Not configured");
    amp->Set(0);
    freq->Set(0);
    dur->Set(0);
    lab->SetDefault("");
    config_index = size_t(-1);
    settings = {};
  }

  void MinMaxStimConfigBox::SetEnabled_Handler(const bool& enabled) {
    if (enabled && config_index == size_t(-1)) {
      return;
    }
    amp->SetReadOnly( ! enabled );
    freq->SetReadOnly( ! enabled );
    dur->SetReadOnly( ! enabled );
    lab->SetReadOnly( ( ! enabled ) || label_locked );
    test_stim->SetEnabled(enabled);
    approve_check->SetEnabled(enabled);
  }
}

