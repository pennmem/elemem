#ifndef CHANNELSELECTOR_H
#define CHANNELSELECTOR_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "EEGDisplay.h"
#include <QScrollArea>

namespace CML {
  class MainWindow;

  class ChannelSelector : public QScrollArea, public RCqt::Worker {
    Q_OBJECT

    public:
    ChannelSelector(RC::Ptr<MainWindow> main_window);

    RCqt::TaskCaller<const RC::Data1D<EEGChan>> SetChannels =
      TaskHandler(ChannelSelector::SetChannels_Handler);

    QSize sizeHint() const { return QSize(160, 120); }

    protected:
    void SetChannels_Handler(const RC::Data1D<EEGChan>& channels);

    void ChannelChecked(size_t i, bool b);

    RC::Data1D<EEGChan> chans;
    RC::Data1D<RC::Caller<void, const bool&>> chan_callbacks;
    RC::Ptr<MainWindow> main_window;
  };
}

#endif // CHANNELSELECTOR_H

