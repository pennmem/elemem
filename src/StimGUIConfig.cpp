#include "StimGuiConfig.h"
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
                         "Amplitude (ms)");
    stim_conf->addWidget(amp);
    freq = new LabeledI64(MakeCaller(this, &StimConfigBox::FreqChanged),
                          "Frequency (Hz)");
    stim_conf->addWidget(freq);
    dur = new LabeledI64(MakeCaller(this, &StimConfigBox::DurChanged),
                         "Duration (ms)");
    stim_conf->addWidget(dur);

    RC::Ptr<QHBoxLayout> stim_approve = new QHBoxLayout();
    RC::Ptr<Button> test_stim = new Button(
        MakeCaller(this, &StimConfigBox::TestStim), "Test Stim");
    stim_approve->addWidget(test_stim);

    RC::Ptr<CheckBox> approve_check = new CheckBox(
        MakeCaller(this, &StimConfigBox::ApproveChanged), "Approve Settings");
    stim_approve->addWidget(approve_check);

    stim_conf->addLayout(stim_approve);

    setLayout(stim_conf);
  }


  void StimConfigBox::SetChannel_Handler(const CSStimChannel& minvals,
      const CSStimChannel& maxvals, const RC::RStr& label,
      const size_t& index) {
    disabled = true;
    setTitle((RStr(minvals.electrode_pos) + "_" +
          RStr(minvals.electrode_neg)).ToQString());
    settings.params = minvals;
    settings.label = label;
    amp->SetRange(minvals.amplitude * 1e-3, maxvals.amplitude * 1e-3);
    freq->SetRange(minvals.frequency, maxvals.frequency);
    dur->SetRange(int64_t(minvals.duration * 1e-3 + 0.5), int64_t(maxvals.duration * 1e-3));
    lab->SetDefault(label);
    lab->SetReadOnly( ! label.empty() );
    config_index = index;
  }

  void StimConfigBox::SetParameters_Handler(const CSStimChannel& params) {
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
}



