#include "ChannelSelector.h"
#include "GuiParts.h"
#include "MainWindow.h"
#include <QObjectCleanupHandler>
#include <QVBoxLayout>

namespace CML {
  ChannelSelector::ChannelSelector(RC::Ptr<MainWindow> main_window)
    : main_window(main_window) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  }

  void ChannelSelector::SetChannels_Handler(
      const RC::Data1D<EEGChan>& channels) {

    main_window->GetEEGDisplay()->Clear();
    chans = channels;

    RC::Ptr<QVBoxLayout> lyt = new QVBoxLayout();
    chan_callbacks.Resize(chans.size());
    for (size_t i=0; i<chans.size(); i++) {
      chan_callbacks[i] = [=](const bool& b) {
        ChannelChecked(i, b);
      };
      RC::Ptr<CheckBox> chk = new CheckBox(chan_callbacks[i],
        RC::RStr(channels[i].channel+1) + "(" + channels[i].label + ")");
      lyt->addWidget(chk);
      if (i < 16) {
        chk->Set(true);
      }
    }
    lyt->setContentsMargins(2, 1, 2, 1);
    RC::Ptr<QWidget> widg = new QWidget();
    widg->setLayout(lyt);
    setWidget(widg);
  }

  void ChannelSelector::ChannelChecked(size_t i, bool b) {
    if (b) {
      main_window->GetEEGDisplay()->SetChannel(chans[i]);
    }
    else {
      main_window->GetEEGDisplay()->UnsetChannel(chans[i]);
    }
  }
}

