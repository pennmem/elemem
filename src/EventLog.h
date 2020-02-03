#ifndef EVENTLOG_H
#define EVENTLOG_H

#include "RC/RStr.h"
#include "RC/File.h"
#include "RCqt/Worker.h"

namespace CML {
  class EventLog : public RCqt::WorkerThread {
    public:
    EventLog() { }

    RCqt::TaskCaller<const RC::RStr> StartFile =
      TaskHandler(EventLog::StartFile_Handler);
    RCqt::TaskCaller<const RC::RStr> Log =
      TaskHandler(EventLog::Log_Handler);
    RCqt::TaskCaller<> CloseFile =
      TaskHandler(EventLog::CloseFile_Handler);
    
    protected:
    void StartFile_Handler(const RC::RStr& filename);
    void Log_Handler(const RC::RStr& event);
    void CloseFile_Handler();

    RC::FileWrite fw;
  };
}

#endif // EVENTLOG_H

