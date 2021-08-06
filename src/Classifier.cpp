#include "Classifier.h"
#include "RC/Macros.h"
#include "Handler.h"
#include "EEGAcq.h"

namespace CML {
  Classifier::Classifier(RC::Ptr<Handler> hndl, size_t sampling_rate)
      : hndl(hndl), buffer(sampling_rate),
        sampling_rate(sampling_rate) {
      callback_ID = RC::RStr("Classifier_") + RC::RStr(sampling_rate);
      buffer.sampling_rate = sampling_rate;

      hndl->eeg_acq.RegisterCallback(callback_ID, ClassifyData);
    }

  void Classifier::ClassifyData_Handler(RC::APtr<const EEGData>& data) {
    auto& datar = data->data;
    if (buffer.data.size() < datar.size()) {
      buffer.data.Resize(datar.size());
    }
    
    RC::RStr deb_msg("Data\n");
    //DebugLog(deb_msg);
    RC_DEBOUT(deb_msg);
  }
}
