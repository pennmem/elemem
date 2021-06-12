#include "EEGAcq.h"
#include "RC/RTime.h"
#include "RC/Errors.h"


namespace CML {
  EEGAcq::EEGAcq() {
    AddToThread(this);
  }

  EEGAcq::~EEGAcq() {
    StopEverything();
  }


  // TODO This needs to split data by channel type.
  // First split SetChannels into SetMacroChannels/SetMicroChannels.
  void EEGAcq::GetData_Slot() {
    if (ShouldAbort()) {
      StopEverything();
      return;
    }

    try {
      RC::APtr<EEGData> data_aptr = new EEGData(sampling_rate);
      data_aptr->data.Resize(cbNUM_ANALOG_CHANS);
      auto& data = data_aptr->data;

      auto& cereb_chandata = cereb.GetData();

      size_t max_len = 0;
      for (size_t c=0; c<cereb_chandata.size(); c++) {
        max_len = std::max(max_len, cereb_chandata[c].data.size());
      }

      if (max_len == 0) {
        return;
      }

      for(size_t i=0; i<cereb_chandata.size(); i++) {
        auto cereb_chan = cereb_chandata[i].chan;
        if (cereb_chan >= data.size()) {
          continue;
        }
        auto& cereb_data = cereb_chandata[i].data;
        auto& chan = data[cereb_chan];
        chan.Resize(max_len);
        for (size_t d=0; d<cereb_data.size(); d++) {
          chan[d] = cereb_data[d];
        }
        // Fill in zeros if needed to guarantee all channels the same size.
        for (size_t d=cereb_data.size(); d<chan.size(); d++) {
          chan[d] = 0;
        }
      }


      auto data_captr = data_aptr.ExtractConst();
      for (size_t i=0; i<data_callbacks.size(); i++) {
        data_callbacks[i].callback(data_captr);
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


  void EEGAcq::SetChannels_Handler(ChannelList& channels,
      const size_t& new_sampling_rate) {
    sampling_rate = new_sampling_rate;

    size_t samprate_index = 0;

    // These are the only values the NeuroPort can handle.
    switch (sampling_rate) {
      case 500:   samprate_index = 1; break;
      case 1000:  samprate_index = 2; break;
      case 2000:  samprate_index = 3; break;
      case 10000: samprate_index = 4; break;
      case 30000: samprate_index = 5; break;
      default: Throw_RC_Type(File, "Configuration selected invalid sampling "
                 "rate.  Allowed values are 500, 1000, 2000, 10000, 30000.");
    }

    StopEverything();

    std::vector<uint16_t> channel_vect(channels.size());

    for (size_t i=0; i<channels.size(); i++) {
      channel_vect[i] = channels[i];
    }

    cereb.SetChannels(channel_vect, samprate_index);
  }


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

    if (data_callbacks.size() == 0 && acq_timer.IsSet()) {
      acq_timer->stop();
    }
  }


  void EEGAcq::CloseCerebus_Handler() {
    cereb.Close();
    StopEverything();
  }


  void EEGAcq::StopEverything() {
    if (acq_timer.IsSet()) {
      acq_timer->stop();
      acq_timer.Delete();
    }
  }


  void EEGAcq::BeAllocatedTimer() {
    if (acq_timer.IsNull()) {
      acq_timer = new QTimer();
      acq_timer->setTimerType(Qt::PreciseTimer);

      QObject::connect(acq_timer.Raw(), &QTimer::timeout, this,
                     &EEGAcq::GetData_Slot);
    }
  }
}

