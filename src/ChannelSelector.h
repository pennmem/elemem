#ifndef CHANNELSELECTOR_H
#define CHANNELSELECTOR_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "EEGDisplay.h"
#include <QScrollArea>

namespace CML {
  class Handler;

  class ChannelSelector : public QScrollArea : public Worker {
    Q_OBJECT

    public:
    ChannelSelector(RC::Ptr<MainWindow> main_window);

    RCqt::TaskCaller<const RC::Data1D<EEGChan>> SetChannels =
      TaskHandler(ChannelSelector::SetChannels_Handler);

    protected:
    void SetChannels_Handler(const RC::Data1D<EEGChan>& channels);

    void ChannelChecked(size_t i, bool b);

    RC::Data1D<EEGChan> chans;
    RC::Data1D<RC::Caller<void, bool>> chan_callbacks;
    RC::Ptr<MainWindow> main_window;
  };
}

#endif // CHANNELSELECTOR_H

