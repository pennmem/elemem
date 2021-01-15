#include "EEGFileSave.h"
#include "Handler.h"

namespace CML {
  EEGFileSave::EEGFileSave(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
  }

  EEGFileSave::~EEGFileSave() {
    StopSaving_Handler();
  }

  void EEGFileSave::StartFile_Handler(const RC::RStr& /*filename*/) {
    // Do not save data.
  }

  void EEGFileSave::StopSaving_Handler() {
    // Do not save data.
  }

  void EEGFileSave::SaveData_Handler(RC::APtr<const EEGData>& /*data*/) {
    // Do not save data.
  }
}

