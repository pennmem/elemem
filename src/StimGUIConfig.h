#ifndef STIMGUICONFIG_H
#define STIMGUICONFIG_H

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
  class StimConfigBox : public QWidget, public RCqt::Worker {
    Q_OBJECT

    public:

    StimConfigBox(RC::Caller<void, const CSStimChannel&> callback);

    protected:
    void SetChannel_Handler(const uint8_t& anode, const uint8_t& cathode,
      const RC::RStr& label="");
    void SetParameters_Handler(const CSStimChannel& stim_params);

    public:
    RCqt::TaskCaller<const uint8_t. const uint8_t, const RC::RStr>
      SetChannel = TaskHandler(StimConfigBox::SetChannel_Handler);
    RCqt::TaskCaller<const CSStimChannel> SetParameters =
      TaskHandler(StimConfigBox::SetParameters_Handler);

    protected slots:
    void ChangedSlot() { Changed(); }
    void Initialize();

    protected:
    virtual void Changed() {
      // TODO correct this
      RC::RStr val = edit.text().toStdString();
      callback(val);
    }

    RC::Caller<void, const CSStimChannel&> callback;
  };
}


#endif // STIMGUICONFIG_H

