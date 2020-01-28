#ifndef EEGACQ_H
#define EEGACQ_H

#include "RC/File.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"
#include "EEGData.h"
#include "Cerebus.h"
#include <QTimer>

namespace CML {
  using ChannelList = RC::Data1D<uint16_t>;
  using EEGCallback = RC::Caller<void, RC::APtr<const EEGData>>;

  class EEGAcq : public RCqt::WorkerThread, public QObject {
    public:
    
    EEGAcq();
    ~EEGAcq();

    // Rule of 3.
    EEGAcq(const EEGAcq&) = delete;
    EEGAcq& operator=(const EEGAcq&) = delete;

    RCqt::TaskCaller<uint32_t> SetInstance =
      TaskHandler(EEGAcq::SetInstance_Handler);

    RCqt::TaskCaller<ChannelList> SetChannels =
      TaskHandler(EEGAcq::SetChannels_Handler);

    RCqt::TaskCaller<const RC::RStr, const EEGCallback>
      RegisterCallback =
      TaskHandler(EEGAcq::RegisterCallback_Handler);

    RCqt::TaskCaller<const RC::RStr> RemoveCallback =
      TaskHandler(EEGAcq::RemoveCallback_Handler);

    RCqt::TaskBlocker<> CloseCerebus =
      TaskHandler(EEGAcq::CloseCerebus_Handler);

    protected slots:

    void GetData_Slot();

    protected:

    void SetInstance_Handler(uint32_t& instance);
    void SetChannels_Handler(ChannelList& channels);

    // All channels have either 0 data or the same amount.
    void RegisterCallback_Handler(const RC::RStr& tag,
                                  const EEGCallback& callback);
    void RemoveCallback_Handler(const RC::RStr& tag);
    void CloseCerebus_Handler();

    void StopEverything();

    void BeAllocatedTimer();

    Cerebus cereb;

    RC::FileWrite eeg_out;
    RC::APtr<QTimer> acq_timer;
    int polling_interval_ms = 20;

    struct TaggedCallback {
      RC::RStr tag;
      EEGCallback callback;
    };
    RC::Data1D<TaggedCallback> data_callbacks;
  };
}

#endif // EEGACQ_H
