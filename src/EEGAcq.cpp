#include "EEGAcq.h"
#include "RC/RTime.h"
#include "RC/Errors.h"
#include "Cerebus.h" // TODO Remove after moving injection to Handler.
#include "CerebusSim.h" // TODO Remove after moving injection to Handler.

#include "FeatureFilters.h"
#include "Utils.h"

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

    if (eeg_source.IsNull() || !channels_initialized) {
      return;
    }

    try {
      auto& cereb_chandata = eeg_source->GetData();

      size_t max_len = 0;
      for (size_t c=0; c<cereb_chandata.size(); c++) {
        max_len = std::max(max_len, cereb_chandata[c].data.size());
      }

      if (max_len == 0) {
        return;
      }

      RC::APtr<EEGData> data_aptr = new EEGData(sampling_rate, max_len);
      auto& data = data_aptr->data;
      data.Resize(cbNUM_ANALOG_CHANS);

      for(size_t i=0; i<cereb_chandata.size(); i++) {
        uint16_t cereb_chan = cereb_chandata[i].chan;
        // Unsigned -1 used for deactivated channel.
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

      // Report Original Data
      for (size_t i=0; i<mono_data_callbacks.size(); i++) {
        mono_data_callbacks[i].callback(data_captr);
      }

      // Bin data
      RC::APtr<BinnedData> binned_data = [&] {
        if (rollover_data.IsSet()) {
          return FeatureFilters::BinData(rollover_data, data_captr, binned_sampling_rate);
        } else {
          return FeatureFilters::BinData(data_captr, binned_sampling_rate);
        }
      }();
      rollover_data = binned_data->leftover_data.ExtractConst();
      auto binned_data_captr = binned_data->out_data.ExtractConst();

      // Report binned data only if there's a non-zero amount.
      auto& binned_data_captr_dr = binned_data_captr->data;
      size_t bin_max_len = 0;
      for (size_t c=0; c<binned_data_captr_dr.size(); c++) {
        bin_max_len = std::max(bin_max_len, binned_data_captr_dr[c].size());
      }

      if (bin_max_len > 0) {
        // Bipolar reference data
        auto out_data_captr = [&] {
          if (bipolar_channels.IsEmpty()) { // Mono
            return FeatureFilters::MonoSelector(binned_data_captr).ExtractConst();
          }
          else { // Bipolar
            return FeatureFilters::BipolarReference(binned_data_captr, bipolar_channels).ExtractConst();
          }
        }();

        // Report bipolar binned data
        for (size_t i=0; i<data_callbacks.size(); i++) {
          data_callbacks[i].callback(out_data_captr);
        }
      }
    }
    catch (...) {
      // Stop acquisition timer upon error, and one pop-up only.
      StopEverything();
      throw;
    }
  }


  void EEGAcq::SetSource_Handler(RC::APtr<EEGSource>& new_source) {
    eeg_source = new_source;
  }


  void EEGAcq::SetBipolarChannels_Handler(RC::Data1D<EEGChan>& new_bipolar_channels) {
    bipolar_channels = new_bipolar_channels;
  }


  void EEGAcq::InitializeChannels_Handler(const size_t& new_sampling_rate,
                                          const size_t& new_binned_sampling_rate) {
    if (eeg_source.IsNull()) {
      return;
    }

    StopEverything();

    sampling_rate = new_sampling_rate;
    binned_sampling_rate = new_binned_sampling_rate;

    rollover_data.Delete();
    eeg_source->InitializeChannels(sampling_rate);

    channels_initialized = true;
    BePollingIfCallbacks();
  }


  void EEGAcq::RegisterEEGCallback_Handler(const RC::RStr& tag,
                                           const EEGCallback& callback) {
    RemoveEEGCallback_Handler(tag);

    data_callbacks += TaggedCallback<EEGCallback>{tag, callback};

    BePollingIfCallbacks();
  }


  void EEGAcq::RemoveEEGCallback_Handler(const RC::RStr& tag) {
    for (size_t i=0; i<data_callbacks.size(); i++) {
      if (data_callbacks[i].tag == tag) {
        data_callbacks.Remove(i);
        i--;
      }
    }

    if (data_callbacks.size() == 0 && mono_data_callbacks.size() == 0 && acq_timer.IsSet()) {
      acq_timer->stop();
    }
  }


  void EEGAcq::RegisterEEGMonoCallback_Handler(const RC::RStr& tag,
                                               const EEGMonoCallback& callback) {
    RemoveEEGMonoCallback_Handler(tag);

    mono_data_callbacks += TaggedCallback<EEGMonoCallback>{tag, callback};

    BePollingIfCallbacks();
  }


  void EEGAcq::RemoveEEGMonoCallback_Handler(const RC::RStr& tag) {
    for (size_t i=0; i<mono_data_callbacks.size(); i++) {
      if (mono_data_callbacks[i].tag == tag) {
        mono_data_callbacks.Remove(i);
        i--;
      }
    }

    if (data_callbacks.size() == 0 && mono_data_callbacks.size() == 0 && acq_timer.IsSet()) {
      acq_timer->stop();
    }
  }



  void EEGAcq::CloseSource_Handler() {
    if (eeg_source.IsSet()) {
      eeg_source->Close();
    }
    channels_initialized = false;
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
      AddToThread(acq_timer);  // For maintenance robustness.
      acq_timer->setTimerType(Qt::PreciseTimer);

      QObject::connect(acq_timer.Raw(), &QTimer::timeout, this,
                     &EEGAcq::GetData_Slot);
    }
  }


  void EEGAcq::BePollingIfCallbacks() {
    if (DirectCallingMode()) {
      return;
    }
    BeAllocatedTimer();
    if (!acq_timer->isActive() && data_callbacks.size() > 0) {
      acq_timer->start(polling_interval_ms);
    }
  }
}

