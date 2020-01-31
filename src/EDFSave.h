#ifndef EDFSAVE_H
#define EDFSAVE_H

#include "EEGData.h"
#include "RC/File.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  class EDFSave : public RCqt::WorkerThread {
    public:
    EDFSave(RC::Ptr<Handler> hndl);
    ~EDFSave();

    // Rule of 3.
    EDFSave(const EDFSave&) = delete;
    EDFSave& operator=(const EDFSave&) = delete;

    RCqt::TaskCaller<const RC::RStr> StartFile =
      TaskHandler(EDFSave::StartFile_Handler);
    RCqt::TaskCaller<> StopSaving =
      TaskHandler(EDFSave::StopSaving_Handler);

    RCqt::TaskCaller<RC::APtr<const EEGData>> SaveData =
      TaskHandler(EDFSave::SaveData_Handler);

    protected:
    void StartFile_Handler(const RC::RStr& filename);
    void StopSaving_Handler();
    void SaveData_Handler(RC::APtr<const EEGData>& data);

    template<class F, class P>
    void SetChanParam(F func, P p, RC::RStr error_msg);

    int edf_hdl = -1;
    RC::Ptr<Handler> hndl;
    RC::Data1D<uint8_t> channels;
    EEGData buffer;
    size_t amount_buffered = 0;
    size_t amount_written = 0;
    size_t sampling_rate = 1000;
  };
}

#endif // EDFSAVE_H

