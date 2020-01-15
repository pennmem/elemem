#include "EEGAcq.h"
#include "RC/RTime.h"


namespace CML {
  EEGAcq::EEGAcq() {
  }

  EEGAcq::~EEGAcq() {
    if (acq_timer.IsSet()) {
      acq_timer->stop();
    }
  }


  void EEGAcq::GetData_Slot() {
    try {

      EEGData data(cbNUM_ANALOG_CHANS);

      auto& cereb_chandata = cereb.GetData();

      for(size_t i=0; i<cereb_chandata.size(); i++) {
        auto cereb_chan = cereb_chandata[i].chan;
        if (cereb_chan >= data.size()) {
          continue;
        }
        auto& cereb_data = cereb_chandata[i].data;
        auto& chan = data[cereb_chan];
        chan.Resize(cereb_data.size());
        for (size_t d=0; d<cereb_data.size(); d++) {
          chan[d] = cereb_data[d];
        }
      }

      // TODO delete
      static size_t count=0;
      if (count % 100 == 0) {
        std::cout << "Calling " << data_callbacks.size() << " callbacks.\n";
      }
      count++;
      for (size_t i=0; i<data_callbacks.size(); i++) {
        data_callbacks[i].callback(data);
      }
    }
    catch (...) {
      // Stop acquisition timer upon error, and one pop-up only.
      StopEverything();
      throw;
    }
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


//  void EEGAcq::StartSaving_Handler(RC::RStr& output_path) {
//    StopSaving_Handler();

//    if ( ! eeg_out.Open(output_path) ) {
//      Throw_RC_Type(File, "Could not write to EEG File");
//    }

//    saving_data = true;

//    SaveMore();
//  }


//  void EEGAcq::StopSaving_Handler() {
//    if (saving_data) {
//      saving_data = false;

//      eeg_out.Close();
//    }
//  }


  //  void EEGAcq::SaveMore_Handler() {
  //    if (!saving_data) {
  //      return;
  //    }

  //    auto& data = cereb.GetData();

  //    for(size_t c=0; c<data.size(); c++) {
  //      auto& chan = data[c];
  //      eeg_out.Put("Channel ");
  //      eeg_out.Put(RC::RStr(c+1));
  //      for (size_t d=0; d<chan.size(); d++) {
  //        eeg_out.Put(", ");
  //        eeg_out.Put(RC::RStr(chan[d]));
  //      }
  //      eeg_out.Put("\n");
  //    }

  //    RC::Time::Sleep(0.010);
  //    SaveMore();
  //  }


  void EEGAcq::RegisterCallback_Handler(const RC::RStr& tag,
                                        const EEGCallback& callback) {
    RemoveCallback_Handler(tag);

    data_callbacks += TaggedCallback{tag, callback};

    BeAllocatedTimer();

    if (!acq_timer->isActive()) {
      acq_timer->start(polling_interval_ms);
    }
  }


  void EEGAcq::RemoveCallback_Handler(const RC::RStr& tag) {
    for (size_t i=0; i<data_callbacks.size(); i++) {
      if (data_callbacks[i].tag == tag) {
        data_callbacks.Remove(i);
        i--;
      }
    }

    BeAllocatedTimer();

    if (data_callbacks.size() == 0) {
      acq_timer->stop();
    }
  }


  void EEGAcq::CloseCerebus_Handler() {
    cereb.Close();
  }


  void EEGAcq::StopEverything() {
    if (acq_timer.IsSet()) {
      acq_timer->stop();
    }
  }


  void EEGAcq::BeAllocatedTimer() {
    if (acq_timer.IsNull()) {
      acq_timer = new QTimer();
      acq_timer->setTimerType(Qt::PreciseTimer);
      std::cout << "Timer type: " << acq_timer->timerType() << std::endl;
    }

    QObject::connect(acq_timer.Raw(), &QTimer::timeout, this,
                     &EEGAcq::GetData_Slot);
  }
}

