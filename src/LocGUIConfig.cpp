#include "LocGUIConfig.h"
#include "RC/RCBits.h"
#include <QGridLayout>


using namespace RC;

namespace CML {
  LocConfigBox::LocConfigBox(
      RC::Caller<void, const size_t&> testing_callback,
      RC::Caller<void, const RC::Data1D<bool>&> approval_callback,
      RC::RStr title)
    : testing_callback(testing_callback),
      approval_callback(approval_callback) {

    setTitle(title.ToQString());

    options_layout = new QGridLayout();
    setLayout(options_layout);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    SetEnabled_Handler(false);
  }

  QSize LocConfigBox::sizeHint() const {
    return QSize(600, 200);
  }


  void LocConfigBox::SetOptions_Handler(const RC::Data1D<RC::RStr>& labels) {
    Clear_Handler();

    approvals.Resize(labels.size());
    approvals.Zero();

    checkboxes.Resize(labels.size());
    selected.Resize(labels.size());

    for (size_t i=0; i<labels.size(); i++) {
      selected[i] = new RadioButton(
          RC::MakeCaller(this, &LocConfigBox::TestSelChanged), labels[i], i);
      checkboxes[i] = new CheckBox(
          RC::MakeCaller(this, &LocConfigBox::ApprovalChanged), "Approve", i);
      options_layout->addWidget(selected[i], i, 0);
      options_layout->addWidget(checkboxes[i], i, 1);
    }

    SetEnabled_Handler(true);
    disabled = false;
  }

  void LocConfigBox::Clear_Handler() {
    disabled = true;
    approvals.Resize(0);

    // Clear out layout contents by reassign to temporary.
    if (layout()) {
      QWidget().setLayout(layout());
    }
    checkboxes.Clear();
    selected.Clear();

    SetEnabled_Handler(false);

    options_layout = new QGridLayout();
    setLayout(options_layout);
  }

  void LocConfigBox::SetApprovals_Handler(
      const RC::Data1D<bool>& new_approvals) {
    disabled = true;
    for (size_t i=0; i<checkboxes.size(); i++) {
      checkboxes[i]->Set(new_approvals[i]);
    }
    disabled = false;
    approvals = new_approvals;
  }

  void LocConfigBox::SetEnabled_Handler(const bool& enabled) {
    for (size_t i=0; i<checkboxes.size(); i++) {
      checkboxes[i]->SetEnabled(enabled);
    }
    for (size_t i=0; i<selected.size(); i++) {
      selected[i]->SetEnabled(enabled);
    }
  }
}



