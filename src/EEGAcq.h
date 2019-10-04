#ifndef EEGACQ_H
#define EEGACQ_H

#include "RC/File.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"
#include "Cerebus.h"

namespace CML {
  using ChannelList = RC::Data1D<uint16_t>;

  class EEGAcq : public RCqt::WorkerThread {
    public:
    
    EEGAcq();

    RCqt::TaskCaller<ChannelList> SetChannels =
      TaskHandler(EEGAcq::SetChannels_Handler);

    RCqt::TaskCaller<RC::RStr> StartSaving =
      TaskHandler(EEGAcq::StartSaving_Handler);

    RCqt::TaskCaller<> StopSaving =
      TaskHandler(EEGAcq::StopSaving_Handler);

    RCqt::TaskCaller<> SaveMore =
      TaskHandler(EEGAcq::SaveMore_Handler);

    protected:

    void SetChannels_Handler(ChannelList& channels);
    void StartSaving_Handler(RC::RStr& output_path);
    void StopSaving_Handler();
    void SaveMore_Handler();

    Cerebus cereb;

    RC::FileWrite eeg_out;

    bool saving_data = false;
  };
}

#endif // EEGACQ_H
