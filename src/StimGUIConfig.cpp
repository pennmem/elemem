#include "StimGuiConfig.h"
#include "RC/RCBits.h"

using namespace RC;

namespace CML {
  StimConfigBox::StimConfigBox(
      Caller<void, const StimSettings&> settings_callback,
      Caller<void> test_stim_callback)
    : settings_callback(settings_callback) {

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

    RC::Ptr<Button> test_stim = new Button(test_stim_callback, "Test Stim");
    stim_conf->addWidget(test_stim);

    setLayout(stim_conf);
  }


  void StimConfigBox::SetChannel_Handler(const CSStimChannel& minvals,
      const CSStimChannel& maxvals, const RC::RStr& label) {
    setTitle((RStr(minvals.electrode_pos+1) + "_" +
          RStr(minvals.electrode_neg+1)).ToQString());
    settings.params = minvals;
    settings.label = label;
    amp->SetRange(minvals.amplitude * 1e-3, maxvals.amplitude * 1e-3);
    freq->SetRange(minvals.frequency, maxvals.frequency);
    dur->SetRange(int64_t(minvals.duration * 1e-3 + 0.5), int64_t(maxvals.duration * 1e-3));
    lab->SetDefault(label);
    lab->SetReadOnly( ! label.empty() );
  }

  void StimConfigBox::SetParameters_Handler(const CSStimChannel& params) {
    amp->Set(params.amplitude * 1e-3);
    freq->Set(params.frequency);
    dur->Set(int64_t(params.duration * 1e-3 + 0.5));
  }

  void StimConfigBox::Clear_Handler() {
    setTitle("Not configured");
    amp->Set(0);
    freq->Set(0);
    dur->Set(0);
    lab->SetDefault("");
  }
}



