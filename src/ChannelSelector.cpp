#include "ChannelSelector.h"
#include "GuiParts.h"
#include <QVoxLayout>

namespace CML {
  ChannelSelector::ChannelSelector(RC::Ptr<MainWindow> main_window)
    : main_window(main_window) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }

  void ChannelSelector::SetChannels_Handler(
      const RC::Data1D<DispChan>& channels) {

    chans = channels;

    RC::Ptr<QVBoxLayout> lyt = new QVBoxLayout();
    for (size_t i=0; i<chans.size(); i++) {
      chan_callbacks[i] = [=](bool b) {
        ChannelChecked(i, b);
      }
      RC::Ptr<CheckBox> chk = new CheckBox(chan_callbacks[i]);
      lyt->addWidget(chk);
      if (i < 16) {
        chk.Set(true);
      }
    }
    lyt.setContentsMargins(2, 1, 2, 1);
    setLayout(lyt);
  }

  void ChannelSelector::ChannelChecked(size_t i, bool b) {
    if (b) {
      main_window->eeg_disp->SetChannel(chans[i]);
    }
    else {
      main_window->eeg_disp->UnsetChannel(chans[i]);
    }
  }
}

