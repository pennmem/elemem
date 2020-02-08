#include "GuiParts.h"
#include "RC/RCBits.h"
#include "RC/RStr.h"

using namespace RC;

namespace CML {
  LabeledEdit::LabeledEdit(Caller<void, const RStr&> callback,
                           const RStr &label_text)
    : label(label_text.c_str()), callback(callback) {
    Initialize();
  }

  LabeledEdit::LabeledEdit(const RStr &label_text)
    : label(label_text.c_str()) {
    Initialize();
  }

  void LabeledEdit::Initialize() {
    edit.setAlignment(Qt::AlignRight);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(2,4,2,4);
    layout->addWidget(&label);
    layout->addWidget(&edit);
    setLayout(layout);

    connect(&edit, SIGNAL(editingFinished()),
            this, SLOT(ChangedSlot()));
  }

  void LabeledEdit::AlignLeft() {
    edit.setAlignment(Qt::AlignLeft);
  }

  void LabeledEdit::Set_Handler(const RC::RStr& str) {
    RStr valid = str;
    Validate(valid);
    edit.setText(valid.c_str());
  }

  void LabeledEdit::ToDefault_Handler() {
    Set_Handler(default_val);
  }

  void LabeledEdit::SetDefault(const RC::RStr& new_default) {
    default_val = new_default;
    edit.setText(default_val.c_str());
  }

////////////////////////////

  LabeledF64::LabeledF64(Caller<void, const f64&> callback,
                         const RStr &label_text)
    : LabeledEdit(label_text), callbackf64(callback) {

    SetRange(-RC::MAX_VAL<f64>(), RC::MAX_VAL<f64>());
    Set(0);
  }

  void LabeledF64::SetRange(f64 new_min, f64 new_max) {
    min = new_min;
    max = new_max;
    last_val = min;
    edit.setText(RC::RStr(min).c_str());
  }

  void LabeledF64::Validate(RC::RStr &str) {
    f64 val = str.Get_f64();
    if (val < min || val > max) {
      val = default_val.Get_f64();
      str = default_val;
    }
    else {
      str = RC::RStr(val);
    }
  }

////////////////////////////

  LabeledI64::LabeledI64(Caller<void, const i64&> callback,
                         const RStr &label_text)
    : label(label_text.c_str()), callback(callback) {

    spin.setAlignment(Qt::AlignRight);
    spin.setSizePolicy(QSizePolicy::Expanding,
                       spin.sizePolicy().verticalPolicy());

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(2,4,2,4);
    layout->addWidget(&label);
    layout->addWidget(&spin);
    setLayout(layout);

    connect(&spin, SIGNAL(valueChanged(int)),
            this, SLOT(ChangedSlot(int)));
  }

  void LabeledI64::Set_Handler(const i64& val) {
    spin.setValue(RC::CappedCast<int>(val));
  }

  void LabeledI64::SetRange(i64 new_min, i64 new_max) {
    spin.setMinimum(RC::CappedCast<int>(new_min));
    spin.setMaximum(RC::CappedCast<int>(new_max));
  }

////////////////////////////

  Button::Button(Caller<> callback, const RC::RStr &text)
    : QPushButton(text.c_str()), callback(callback) {

    connect(this, SIGNAL(clicked(bool)), SLOT(ClickedSlot(bool)));
  }

  void Button::SetColor(Color c) {
    Data1D<u8> cd(3);
    cd[0] = u8(255.9999f*c.r);
    cd[1] = u8(255.9999f*c.g);
    cd[2] = u8(255.9999f*c.b);
    RStr colorstr = RStr::Join(cd, ", ");
    setStyleSheet(("background-color: rgb("+colorstr+");"
                   "font-weight: bold").ToQString());
    update();
  }

////////////////////////////

  CheckBox::CheckBox(Caller<void, const bool&> callback, const RC::RStr &text)
    : QCheckBox(text.c_str()), callback(callback) {

    connect(this, SIGNAL(stateChanged(int)), SLOT(ToggledSlot(int)));
  }

  void CheckBox::Set_Handler(const bool& state) {
    setChecked(state);
  }

  void CheckBox::SetEnabled_Handler(const bool& enabled) {
    setEnabled(enabled);
  }

///////////////////////////

  ComboBox::ComboBox() {
    connect(this, SIGNAL(currentIndexChanged(int)),
            SLOT(IndexChangedSlot(int)));
  }

  void ComboBox::Set_Handler(const u64& index) {
    setCurrentIndex(RC::CappedCast<int>(index));
  }
}



