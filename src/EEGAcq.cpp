#include "EEGAcq.h"
#include "RC/RTime.h"


namespace CML {
  EEGAcq::EEGAcq() {
  }

  void EEGAcq::SetInstance_Handler(uint32_t& instance) {
    StopEverything();

    cereb.SetInstance(instance);
  }


  void EEGAcq::SetChannels_Handler(ChannelList& channels) {
    StopEverything();

    std::vector<uint16_t> channel_vect(channels.size());

    for (size_t i=0; i<channels.size(); i++) {
      channel_vect[i] = channels[i];
    }

    cereb.SetChannels(channel_vect);
  }


  void EEGAcq::StartSaving_Handler(RC::RStr& output_path) {
    StopSaving_Handler();

    if ( ! eeg_out.Open(output_path) ) {
      Throw_RC_Type(File, "Could not write to EEG File");
    }

    saving_data = true;

    SaveMore();
  }


  void EEGAcq::StopSaving_Handler() {
    if (saving_data) {
      saving_data = false;

      eeg_out.Close();
    }
  }


  void EEGAcq::SaveMore_Handler() {
    if (!saving_data) {
      return;
    }

    auto& data = cereb.GetData();

    for(size_t c=0; c<data.size(); c++) {
      auto& chan = data[c];
      eeg_out.Put("Channel ");
      eeg_out.Put(RC::RStr(c+1));
      for (size_t d=0; d<chan.size(); d++) {
        eeg_out.Put(", ");
        eeg_out.Put(RC::RStr(chan[d]));
      }
      eeg_out.Put("\n");
    }

    RC::Time::Sleep(0.010);
    SaveMore();
  }

  void EEGAcq::StopEverything() {
    if (saving_data) {
      StopSaving_Handler();
    }
  }

  void EEGAcq::CloseCerebus_Handler() {
    cereb.Close();
  }
}

