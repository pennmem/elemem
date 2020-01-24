#include "StimGuiConfig.h"
#include "RC/RCBits.h"

using namespace RC;

namespace CML {
  StimConfigBox::StimConfigBox(
      Caller<void, const StimSettings&> settings_callback,
      Caller<void> test_stim_callback)
    : settings_callback(settings_callback) {

    // Store this label so it can be set at config load.
    setTitle("Not configured");

    RC::Ptr<QVBoxLayout> stim_conf = new QVBoxLayout();

    label = new LabeledEdit(MakeCaller(this, &StimConfigBox::LabelChanged),
                            "Label");
    stim_conf->addWidget(label);
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
    // TODO
  }

  void StimConfigBox::SetParameters_Handler(const CSStimChannel& params) {
    // TODO
  }

  void StimConfigBox::Clear_Handler() {
    // TODO
  }
}



