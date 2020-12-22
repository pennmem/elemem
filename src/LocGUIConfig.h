#ifndef LOCGUICONFIG_H
#define LOCGUICONFIG_H

#include "CereStim.h"
#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "Settings.h"
#include "GuiParts.h"
#include <cmath>
#include <QGroupBox>


namespace CML {
  class LocConfigBox : public QGroupBox, public RCqt::Worker {
    Q_OBJECT

    public:

    LocConfigBox(
      RC::Caller<void, const size_t&> testing_callback,
      RC::Caller<void, const RC::Data1D<bool>&> approval_callback,
      RC::RStr title);

    protected:
    void SetOptions_Handler(const RC::Data1D<RC::RStr>& labels);
    void Clear_Handler();
    void SetApprovals_Handler(const RC::Data1D<bool>& new_approvals);
    void SetEnabled_Handler(const bool& enabled);

    public:
    RCqt::TaskCaller<const RC::Data1D<RC::RStr>> SetOptions =
      TaskHandler(LocConfigBox::SetOptions_Handler);
    RCqt::TaskCaller<> Clear = TaskHandler(LocConfigBox::Clear_Handler);
    RCqt::TaskCaller<const RC::Data1D<bool>> SetApprovals =
      TaskHandler(LocConfigBox::SetApprovals_Handler);
    RCqt::TaskCaller<const bool> SetEnabled =
      TaskHandler(LocConfigBox::SetEnabled_Handler);


    // Internal callbacks
    void ApprovalChanged(const size_t& index, const bool& state) {
      if (disabled) { return; }
      approvals[index] = state;
      approval_callback(approvals);
    }

    void TestSelChanged(const size_t& index, const bool& state) {
      if (disabled) { return; }
      if (state) {
        test_select = index;
        testing_callback(test_select);
      }
    }

    QSize sizeHint() const;

    protected:
    RC::Caller<void, const size_t&> testing_callback;
    RC::Caller<void, const RC::Data1D<bool>&> approval_callback;
    RC::Data1D<RC::Ptr<CheckBox>> checkboxes;
    RC::Data1D<RC::Ptr<RadioButton>> selected;
    RC::Data1D<bool> approvals;
    size_t test_select = size_t(-1);
    RC::Ptr<QGridLayout> options_layout;
    bool disabled = true;
  };
}


#endif // LOCGUICONFIG_H

