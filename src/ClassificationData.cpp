#include "ClassificationData.h"
#include "RC/Macros.h"
#include "Handler.h"
#include "EEGAcq.h"
#include "Classifier.h"

namespace CML {
  ClassificationData::ClassificationData(RC::Ptr<Handler> hndl, size_t sampling_rate)
      : hndl(hndl), buffer(sampling_rate),
        sampling_rate(sampling_rate) {
      callback_ID = RC::RStr("Classifier_") + RC::RStr(sampling_rate);
      buffer.sampling_rate = sampling_rate;

      hndl->eeg_acq.RegisterCallback(callback_ID, ClassifyData);
    }

  void ClassificationData::ClassifyData_Handler(RC::APtr<const EEGData>& data) {
    auto& datar = data->data;
    if (buffer.data.size() < datar.size()) {
      buffer.data.Resize(datar.size());
    }

    //------------
    // TODO: Steps to get the Classification handler working
    // Store data in circ buffer one by one
    // Bin data as you go
    // if (numData > dataSinceLastClassification)
    //   dataSinceLastClassification = 0;
    //   if (!(eventLoop has too many waiting data points)) // skip classification to catch up
    //     <Create new ordered buff from circ buf>
    //     Classifier.classify(orderedData)
    // publish the result 
    //------------
    
    RC::RStr deb_msg = RC::RStr("Data\n");
    for (size_t c=0; c<datar.size(); c++) {
      deb_msg += "Channel " + RC::RStr(c) + ": " + RC::RStr::Join(datar[c], ", ") + "\n";
    } 
    deb_msg += "\n\n";
    RC_DEBOUT(deb_msg);

    callback(data);
    #include <stdlib.h>
    exit(0);
  }
  
  void ClassificationData::SetCallback_Handler(EEGCallbackTask& new_callback) {
    callback = new_callback;
  }
}
