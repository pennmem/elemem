#ifndef SIGQUALITY_H
#define SIGQUALITY_H

#include "EEGData.h"
#include "RC/APtr.h"
#include "RCqt/Worker.h"
#include "ChannelConf.h"


namespace CML {
  class SigQualityResults {
    public:
    RC::Data1D<double> linenoise_frac;
    RC::Data1D<double> worst_channel;
    size_t worst_channum;
    RC::Data1D<RC::RStr> report_messages;
    RC::RStr measured_freq;
    bool success;
  };

  using TaskSigQualityIncoming = RCqt::TaskCaller<RC::APtr<const EEGDataRaw>>;
  using SigResultCallback = RCqt::TaskCaller<const SigQualityResults>;

  class EEGAcq;
  class EventLog;


  class SigQuality : public RCqt::WorkerThread {
    public:
    SigQuality(RC::Ptr<EEGAcq> eeg_acq);
    ~SigQuality();

    RCqt::TaskCaller<> Start =
      TaskHandler(SigQuality::Start_Handler);

    RCqt::TaskCaller<> Stop =
      TaskHandler(SigQuality::Stop_Handler);

    RCqt::TaskCaller<const RC::Data1D<bool>> SetChannelMask =
      TaskHandler(SigQuality::SetChannelMask_Handler);

    TaskSigQualityIncoming Process =
      TaskHandler(SigQuality::Process_Handler);

    // Callbacks cleared after each finalize step.
    RCqt::TaskCaller<const RC::RStr, const SigResultCallback>
      RegisterCallback =
      TaskHandler(SigQuality::RegisterCallback_Handler);

    RCqt::TaskBlocker<const RC::RStr> RemoveCallbacks =
      TaskHandler(SigQuality::RemoveCallback_Handler);


    protected:
    void Start_Handler();
    void Stop_Handler();
    void SetChannelMask_Handler(const RC::Data1D<bool>& mask);
    void Process_Handler(RC::APtr<const EEGDataRaw>&);
    void RegisterCallback_Handler(const RC::RStr& tag,
        const SigResultCallback& callback);
    void RemoveCallback_Handler(const RC::RStr& tag);

    void Finalize();
    void Evaluate();
    void ClearCallbacks();

    struct TaggedCallback {
      RC::RStr tag;
      SigResultCallback callback;
    };
    RC::Data1D<TaggedCallback> callbacks;

    RC::Ptr<EEGAcq> eeg_acq;

    RC::Data1D<bool> channels;
    RC::Data1D<RC::Data1D<double>> unwrapped;
    RC::Data1D<RC::Data1D<double>> wrapped20; // 60Hz
    RC::Data1D<RC::Data1D<double>> wrapped25; // 50Hz

    RC::Data1D<double> linenoise_frac;
    RC::RStr measured_freq;

    size_t amnt_processed = 0;
    size_t target_amnt = 0;
    size_t target_20 = 0;
    size_t target_25 = 0;
    size_t sampling_rate = 0;

    size_t meas_seconds = 30;
    double bad_line_noise = 0.3;
    size_t bank_size = 32;
    size_t bad_per_bank_thresh = 16;
    double bad_chan_frac = 0.33;
  };
}

#endif // SIGQUALITY_H

