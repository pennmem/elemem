#ifndef EEGFILESAVE_H
#define EEGFILESAVE_H

#include "EEGData.h"
#include "Settings.h"
#include "RC/Ptr.h"
#include "RC/RStr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  class EEGFileSave : public RCqt::WorkerThread {
    public:
    EEGFileSave(RC::Ptr<Handler> hndl);
    virtual ~EEGFileSave();

    // Rule of 3.
    EEGFileSave(const EEGFileSave&) = delete;
    EEGFileSave& operator=(const EEGFileSave&) = delete;

    RCqt::TaskCaller<const RC::RStr, const FullConf> StartFile =
      TaskHandler(EEGFileSave::StartFile_Handler);
    RCqt::TaskCaller<> StopSaving =
      TaskHandler(EEGFileSave::StopSaving_Handler);

    RCqt::TaskCaller<RC::APtr<const EEGData>> SaveData =
      TaskHandler(EEGFileSave::SaveData_Handler);

    virtual RC::RStr GetExt() const { return ""; }

    protected:
    virtual void StartFile_Handler(const RC::RStr& filename,
                                   const FullConf& conf);
    virtual void StopSaving_Handler();
    virtual void SaveData_Handler(RC::APtr<const EEGData>& data);

    RC::Ptr<Handler> hndl;
  };
}

#endif // EEGFILESAVE_H

