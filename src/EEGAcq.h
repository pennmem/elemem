#ifndef EEGACQ_H
#define EEGACQ_H

#include "RC/File.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"
#include "EEGData.h"
#include "EEGSource.h"
#include "ChannelConf.h"
#include <QTimer>

namespace CML {
  //using ChannelList = RC::Data1D<uint16_t>;
  using EEGCallback = RCqt::TaskCaller<RC::APtr<const EEGDataDouble>>;
  using EEGMonoCallback = RCqt::TaskCaller<RC::APtr<const EEGDataRaw>>;

  class EEGAcq : public RCqt::WorkerThread, public QObject {
    public:

    EEGAcq();
    ~EEGAcq();

    // Rule of 3.
    EEGAcq(const EEGAcq&) = delete;
    EEGAcq& operator=(const EEGAcq&) = delete;

    RCqt::TaskCaller<RC::APtr<EEGSource>> SetSource =
      TaskHandler(EEGAcq::SetSource_Handler);

    RCqt::TaskCaller<RC::Data1D<EEGChan>> SetBipolarChannels =
      TaskHandler(EEGAcq::SetBipolarChannels_Handler);

    RCqt::TaskBlocker<const size_t, const size_t> InitializeChannels =
      TaskHandler(EEGAcq::InitializeChannels_Handler);

    RCqt::TaskCaller<> StartingExperiment =
      TaskHandler(EEGAcq::StartingExperiment_Handler);

    RCqt::TaskCaller<> ExperimentReady =
      TaskHandler(EEGAcq::ExperimentReady_Handler);

    RCqt::TaskCaller<const RC::RStr, const EEGCallback>
      RegisterEEGCallback =
      TaskHandler(EEGAcq::RegisterEEGCallback_Handler);

    RCqt::TaskBlocker<const RC::RStr> RemoveEEGCallback =
      TaskHandler(EEGAcq::RemoveEEGCallback_Handler);

    RCqt::TaskCaller<const RC::RStr, const EEGMonoCallback>
      RegisterEEGMonoCallback =
      TaskHandler(EEGAcq::RegisterEEGMonoCallback_Handler);

    RCqt::TaskBlocker<const RC::RStr> RemoveEEGMonoCallback =
      TaskHandler(EEGAcq::RemoveEEGMonoCallback_Handler);

    RCqt::TaskBlocker<> CloseSource =
      TaskHandler(EEGAcq::CloseSource_Handler);

    protected slots:

    void GetData_Slot();

    protected:

    void SetSource_Handler(RC::APtr<EEGSource>& new_source);
    void SetBipolarChannels_Handler(RC::Data1D<EEGChan>& new_bipolar_channels);
    void InitializeChannels_Handler(const size_t& new_sampling_rate, const size_t& new_binned_sampling_rate);
    void StartingExperiment_Handler() { eeg_source->StartingExperiment(); }
    void ExperimentReady_Handler() { eeg_source->ExperimentReady(); }

    // All channels have either 0 data or the same amount.
    void RegisterEEGCallback_Handler(const RC::RStr& tag,
                                     const EEGCallback& callback);
    void RemoveEEGCallback_Handler(const RC::RStr& tag);
    void RegisterEEGMonoCallback_Handler(const RC::RStr& tag,
                                         const EEGMonoCallback& callback);
    void RemoveEEGMonoCallback_Handler(const RC::RStr& tag);
    void CloseSource_Handler();

    void StopEverything();

    void BeAllocatedTimer();
    void BePollingIfCallbacks();

    RC::APtr<EEGSource> eeg_source;
    size_t sampling_rate = 1000;
    size_t binned_sampling_rate;

    RC::APtr<const EEGData> rollover_data;

    RC::APtr<QTimer> acq_timer;
    int polling_interval_ms = 5;
    bool channels_initialized = false;

    RC::Data1D<EEGChan> bipolar_channels;

    template <typename T>
    struct TaggedCallback {
      RC::RStr tag;
      T callback;
    };
    RC::Data1D<TaggedCallback<EEGCallback>> data_callbacks;
    RC::Data1D<TaggedCallback<EEGMonoCallback>> mono_data_callbacks;
  };
}

#endif // EEGACQ_H
