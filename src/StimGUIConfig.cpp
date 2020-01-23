#include "StimGuiConfig.h"
#include "RC/RCBits.h"

using namespace RC;

namespace CML {
  StimConfigBox::StimConfigBox(Caller<void, const CSStimChannel&> callback)
    : callback(callback) {
    Initialize();
  }


  void StimConfigBox::Initialize() {
    // TODO From BuildStimConfig
    edit.setAlignment(Qt::AlignRight);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(2,4,2,4);
    layout->addWidget(&label);
    layout->addWidget(&edit);
    setLayout(layout);

    connect(&edit, SIGNAL(editingFinished()),
            this, SLOT(ChangedSlot()));
  }

  // TODO update:

  void StimConfigBox::Set_Handler(const RC::RStr& str) {
    RStr valid = str;
    Validate(valid);
    edit.setText(valid.c_str());
  }

  void StimConfigBox::ToDefault_Handler() {
    Set_Handler(default_val);
  }

  void StimConfigBox::SetDefault(const RC::RStr& new_default) {
    default_val = new_default;
    edit.setText(default_val.c_str());
  }
}



