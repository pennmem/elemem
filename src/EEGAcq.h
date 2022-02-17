#ifndef EEGACQ_H
#define EEGACQ_H

#include "RC/File.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"
#include "EEGData.h"
#include "EEGSource.h"
#include <QTimer>

namespace CML {
  using ChannelList = RC::Data1D<uint16_t>;
  using EEGCallback = RCqt::TaskCaller<RC::APtr<const EEGData>>;

  class EEGAcq : public RCqt::WorkerThread, public QObject {
    public:

    EEGAcq();
    ~EEGAcq();

    // Rule of 3.
    EEGAcq(const EEGAcq&) = delete;
    EEGAcq& operator=(const EEGAcq&) = delete;

    RCqt::TaskCaller<RC::APtr<EEGSource>> SetSource =
      TaskHandler(EEGAcq::SetSource_Handler);

    RCqt::TaskBlocker<const size_t, const size_t> InitializeChannels =
      TaskHandler(EEGAcq::InitializeChannels_Handler);

    RCqt::TaskCaller<> StartingExperiment =
      TaskHandler(EEGAcq::StartingExperiment_Handler);

    RCqt::TaskCaller<const RC::RStr, const EEGCallback>
      RegisterCallback =
      TaskHandler(EEGAcq::RegisterCallback_Handler);

    RCqt::TaskCaller<const RC::RStr> RemoveCallback =
      TaskHandler(EEGAcq::RemoveCallback_Handler);

    RCqt::TaskBlocker<> CloseSource =
      TaskHandler(EEGAcq::CloseSource_Handler);

    protected slots:

    void GetData_Slot();

    protected:

    void SetSource_Handler(RC::APtr<EEGSource>& new_source);
    void InitializeChannels_Handler(const size_t& new_sampling_rate, const size_t& new_binned_sampling_rate);
    void StartingExperiment_Handler() { eeg_source->StartingExperiment(); }

    // All channels have either 0 data or the same amount.
    void RegisterCallback_Handler(const RC::RStr& tag,
                                  const EEGCallback& callback);
    void RemoveCallback_Handler(const RC::RStr& tag);
    void CloseSource_Handler();

    void StopEverything();

    void BeAllocatedTimer();
    void BePollingIfCallbacks();

    RC::APtr<EEGSource> eeg_source;
    size_t sampling_rate = 1000;
    size_t binned_sampling_rate;

    RC::APtr<QTimer> acq_timer;
    int polling_interval_ms = 5;
    bool channels_initialized = false;

    struct TaggedCallback {
      RC::RStr tag;
      EEGCallback callback;
    };
    RC::Data1D<TaggedCallback> data_callbacks;
  };
}

#endif // EEGACQ_H
