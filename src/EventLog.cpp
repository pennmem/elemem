#include "EventLog.h"

namespace CML {
  EventLog::StartFile_Handler(const RC::RStr& filename) {
    fw = FileWrite(filename);
  }

  EventLog::Log_Handler(const RC::RStr& event) {
    if (fw.IsOpen()) {
      fw.Put(event);
    }
  }

  EventLog::CloseFile_Handler() {
    fw.Close();
  }
}

