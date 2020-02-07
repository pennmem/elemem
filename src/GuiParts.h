#ifndef GUIPARTS_H
#define GUIPARTS_H

#include "Palette.h"
#include "RC/RC.h"
#include "RCqt/Worker.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>


namespace CML {
  class LabeledEdit : public QWidget, public RCqt::Worker {
    Q_OBJECT

    public:

    LabeledEdit(RC::Caller<void, const RC::RStr&> callback, const RC::RStr &label_text="");

    protected:
    void Set_Handler(const RC::RStr& str);
    void ToDefault_Handler();

    public:
    RCqt::TaskCaller<const RC::RStr> Set = TaskHandler(LabeledEdit::Set_Handler);
    RCqt::TaskCaller<> ToDefault = TaskHandler(LabeledEdit::ToDefault_Handler);

    void SetDefault(const RC::RStr& new_default);

    void SetToolTip(const RC::RStr &tip) {
      setToolTip(tip.c_str());
    }

    void SetReadOnly(bool b) {
      edit.setReadOnly(b);
    }

    void AlignLeft();

    protected slots:
    void ChangedSlot() { Changed(); }
    void Initialize();

    protected:
    // For subclasses overriding the callback.
    LabeledEdit(const RC::RStr &label_text="");

    virtual void Changed() {
      RC::RStr val = edit.text().toStdString();
      callback(val);
    }
    virtual void Validate(RC::RStr &/*str*/) { }

    QLabel label;
    QLineEdit edit;
    RC::RStr default_val;
    RC::Caller<void, const RC::RStr&> callback;
  };


  // F64

  class LabeledF64 : public LabeledEdit {
    public:

    LabeledF64(RC::Caller<void, const f64&> callback,
               const RC::RStr &label_text="");

    void SetRange(f64 new_min, f64 new_max);

    protected:
    virtual void Changed() {
      f64 val = RC::RStr(edit.text().toStdString()).Get_f64();
      if (val < min || val > max) {
        if (default_val != "") {
          val = default_val.Get_f64();
          edit.setText(default_val.c_str());
        }
        else {
          val = last_val;
          edit.setText(RC::RStr(val).c_str());
        }
      }
      else {
        edit.setText(RC::RStr(val).c_str());
      }
      last_val = val;
      callbackf64(val);
    }

    void Validate(RC::RStr &str);

    f64 min, max;
    f64 last_val;

    RC::Caller<void, const f64&> callbackf64;
  };


  // I64

  class LabeledI64 : public QWidget, public RCqt::Worker {
    Q_OBJECT

    public:

    LabeledI64(RC::Caller<void, const i64&> callback, const RC::RStr &label_text);

    protected:
    void Set_Handler(const i64& val);

    public:
    RCqt::TaskCaller<const i64> Set = TaskHandler(LabeledI64::Set_Handler);
    RCqt::TaskCaller<const bool> SetReadOnly =
      TaskHandler(LabeledI64::SetReadOnly_Handler);

    void SetRange(i64 new_min, i64 new_max);

    void SetToolTip(const RC::RStr &tip) {
      setToolTip(tip.c_str());
    }

    protected slots:
    void ChangedSlot(int) { Changed(); }

    protected:
    virtual void Changed() {
      i64 val = spin.value();
      callback(val);
    }

    void SetReadOnly_Handler(const bool& state) {
      spin.setReadOnly(state);
    }

    QLabel label;
    QSpinBox spin;

    RC::Caller<void, const i64&> callback;
  };


  // Button

  class Button : public QPushButton, public RCqt::Worker {
    Q_OBJECT

    public:

    Button(RC::Caller<> callback, const RC::RStr& text="");

    void SetToolTip(const RC::RStr &tip) {
      setToolTip(tip.c_str());
    }

    void SetColor(Color c);

    RCqt::TaskCaller<const bool> SetEnabled =
      TaskHandler(Button::SetEnabled_Handler);

    protected slots:
    void ClickedSlot(bool /*checked*/) {
      Clicked();
    }
    protected:
    virtual void Clicked() { callback(); }
    void SetEnabled_Handler(const bool& enabled) {
      setEnabled(enabled);
    }

    RC::Caller<> callback;
  };


  // CheckBox

  class CheckBox : public QCheckBox, public RCqt::Worker {
    Q_OBJECT

    public:

    CheckBox(RC::Caller<void, const bool&> callback, const RC::RStr &text);

    protected:
    void Set_Handler(const bool& state);
    void SetEnabled_Handler(const bool& enabled);

    public:
    RCqt::TaskCaller<const bool> Set = TaskHandler(CheckBox::Set_Handler);
    RCqt::TaskCaller<const bool> SetEnabled =
      TaskHandler(CheckBox::SetEnabled_Handler);

    void SetToolTip(const RC::RStr &tip) {
      setToolTip(tip.c_str());
    }

    protected slots:
    void ToggledSlot(int state) {
      Toggled(state != Qt::Unchecked);
    }
    protected:
    virtual void Toggled(bool state) { callback(state); }

    RC::Caller<void, const bool&> callback;
  };

  // ComboBox

  class ComboBox : public QComboBox, public RCqt::Worker {
    Q_OBJECT

    public:

    ComboBox();

    protected:
    void Set_Handler(const u64& index);

    public:
    RCqt::TaskCaller<const u64> Set = TaskHandler(ComboBox::Set_Handler);

    void SetToolTip(const RC::RStr &tip) {
      setToolTip(tip.c_str());
    }

    protected slots:
    void IndexChangedSlot(int index) {
      IndexChanged(u64(index));
    }
    protected:
    virtual void IndexChanged(u64 index) = 0;
  };
  template <class Enum>
  class ComboBoxCB : public ComboBox {
    public:
    ComboBoxCB(RC::Caller<void, const Enum&> callback,
               RC::Data1D< std::pair<Enum, RC::RStr> > entries)
      : callback(callback) {

      for (size_t i=0; i<entries.size(); i++) {
        insertItem(entries[i].first, entries[i].second.c_str());
      }
    }
    protected:
    void IndexChanged(u64 index) { callback(Enum(index)); }
    RC::Caller<void, const Enum&> callback;
  };


  // Generators

  template <class Enum> RC::Ptr< ComboBoxCB<Enum> >
  MakeComboBox(RC::Caller<void, const Enum&> callback,
               RC::Data1D< std::pair<Enum, RC::RStr> > entries) {
    return new ComboBoxCB<Enum>(callback, entries);
  }
}


#endif // GUIPARTS_H

