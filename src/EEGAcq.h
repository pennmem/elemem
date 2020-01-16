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

//    RCqt::TaskCaller<RC::RStr> StartSaving =
//      TaskHandler(EEGAcq::StartSaving_Handler);

//    RCqt::TaskCaller<> StopSaving =
//      TaskHandler(EEGAcq::StopSaving_Handler);

//    RCqt::TaskCaller<> SaveMore =
//      TaskHandler(EEGAcq::SaveMore_Handler);

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
//    void StartSaving_Handler(RC::RStr& output_path);
//    void StopSaving_Handler();
//    void SaveMore_Handler();
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

    // Change this to timer_running, restructure.
    // Move data saving to another object.
//    bool saving_data = false;
  };
}

#endif // EEGACQ_H
